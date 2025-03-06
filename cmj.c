#include "cmj.h"

//#define debug printf
#define debug(...)

//forward declaration
opt_vartype cmj_parse_json(char *str, size_t str_len, int *offset);

static opt_vartype cmj_parse_scalar(char *str, size_t str_len, int *offset) {
  debug("cmj_parse_scalar: %s\n", str);
  opt_vartype result;
  OPT_INIT_ERROR();
  result.val.typ = CMSG_NONE;

  int i = 0;
  bool has_dot = false;
  bool has_sign = false;
  
  for (;i < str_len; i++) {
    if ((str[i] >= '0') && (str[i] <= '9')) {
    } else if (str[i] == '.') {
      if (has_dot) {
        OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
        return result;
      }
      has_dot = true;
    } else if ((str[i] == '-') && (i == 0)) {
      if (has_sign) {
        OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
        return result;
      }
      has_sign = true;
    } else {
      break;
    }
  }

  if (i == 0) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  } else if ((i == 1) && has_dot) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  } else if ((i == 1) && has_sign) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  }

  char *t = malloc(i + 1);
  memcpy(t, str, i);
  t[i] = 0;

  double dv;
  int64_t iv;

  if (has_dot) {
    sscanf(t, "%lf", &dv);
    opt_vartype dv_ret = cmsg_new_var_d(dv);
    if (dv_ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(dv_ret);
      free(t);
      return result;
    }
    debug("scalar real: %lf\n", dv);
    memcpy(&result.val, &dv_ret.val, sizeof(vartype));
  } else {
    iv = strtol(t, NULL, 10);
    opt_vartype iv_ret = cmsg_new_var_int64(iv);
    if (iv_ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(iv_ret);
      free(t);
      return result;
    }
    debug("scalar int: %li\n", iv);
    memcpy(&result.val, &iv_ret.val, sizeof(vartype));
  }
  free(t);

  *offset = i;
  return result;
}

static opt_vartype cmj_parse_str(char *str, size_t str_len, int *offset) {
  debug("cmj_parse_str: %s\n", str);
  opt_vartype result;
  OPT_INIT_ERROR();
  result.val.typ = CMSG_NONE;

  int i = 1;
  bool has_backslash = false;
  
  for (;i < str_len; i++) {
    if ((str[i] == '"') && (has_backslash == false)) {
      //stop here
      if (i == 1) {
        //empty string
        opt_vartype ret = cmsg_new_str("");
        if (ret.err != CMSG_ERR_OK) {
          OPT_PROPAGATE_ERROR(ret);
          return result;
        }
        *offset = i + 1;
        debug("empty str\n");
        return ret;
      } else {
        int l = i - 1;
        char *t = malloc(l + 1);
        memcpy(t, str + 1, l);
        t[l] = 0;
        opt_vartype ret = cmsg_new_str(t);
        debug("str: %s (%i)\n", t, l);
        free(t);
        if (ret.err != CMSG_ERR_OK) {
          OPT_PROPAGATE_ERROR(ret);
          return result;
        }
        *offset = i + 1;
        return ret;
      }
    } else if (str[i] == '\\') {
      if (has_backslash) {
        has_backslash = false;
      } else {
        has_backslash = true;
      }
    } else {
      has_backslash = false;      
    }
  }

  OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
  return result;
}

static opt_vartype cmj_parse_hash(char *str, size_t str_len, int *offset) {
  debug("cmj_parse_hash: %s\n", str);
  opt_vartype result = cmsg_new_hash();
  if (result.err != CMSG_ERR_OK) {
    return result;
  }

  int t_offt = 0;
  size_t offt = 1;
  size_t i;
  
  while (offt < str_len) {
    // key or closing brace
    if (str[offt] == '}') {
      offt = offt + 1;
      *offset = offt;
      return result;
    } else if (str[offt] == ' ') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\t') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\n') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\r') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '"') {
      t_offt = 0;
      opt_vartype k_ret = cmj_parse_str(str + offt, str_len - offt, &t_offt);
      if (k_ret.err != CMSG_ERR_OK) {
        OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
        vartype_dtor(&result.val);
        result.val.typ = CMSG_NONE;
        return result;
      }
      offt = offt + t_offt;
      debug("end of key @%i\n", offt);
      // now parse value
      // find colon
      bool found_colon = false;
      for (i = offt; i < str_len; i ++) {
        if (str[i] == ':') {
          found_colon = true;
          break;
        }
      }
      if (found_colon == false) {
        OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
        vartype_dtor(&result.val);
        vartype_dtor(&k_ret.val);
        result.val.typ = CMSG_NONE;
        return result;
      }
      offt = i;
      debug("colon @%i\n", offt);
      //find non-space char
      bool found_nonspace = false;
      for (i = (offt + 1); i < str_len; i ++) {
        if ((str[i] == ' ') || (str[i] == '\t')) {
          continue;
        }
        found_nonspace = true;
        break;
      }
      if (found_nonspace == false) {
        OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
        vartype_dtor(&result.val);
        vartype_dtor(&k_ret.val);
        result.val.typ = CMSG_NONE;
        return result;
      }
      offt = i;
      debug("nonspace @%i\n", offt);
      t_offt = 0;
      debug("parse val @%i\n", offt);
      opt_vartype val_ret = cmj_parse_json(str + offt, str_len - offt, &t_offt);
      if (val_ret.err != CMSG_ERR_OK) {
        OPT_PROPAGATE_ERROR(val_ret);
        vartype_dtor(&result.val);
        vartype_dtor(&k_ret.val);
        result.val.typ = CMSG_NONE;
        return result;
      }
      offt = offt + t_offt;
      cmsg_hash_add(&result.val, cmsg_vartype_to_str(&k_ret.val), &val_ret.val);
      vartype_dtor(&k_ret.val);
      vartype_dtor(&val_ret.val);
      //find separator or closing brace
      bool found_separator = false;
      bool found_closing_brace = false;
      // cmj_parse_json returns next symbol, so no need to add 1 here
      for (i = offt; i < str_len; i ++) {
        if (str[i] == ',') {
          found_separator = true;
          break;
        } else if (str[i] == '}') {
          found_closing_brace = true;
          break;
        }
      }
      if ((found_separator == false) && (found_closing_brace == false)) {
        debug("notsep, notcb\n");
        OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
        vartype_dtor(&result.val);
        result.val.typ = CMSG_NONE;
        return result;
      } else if (found_closing_brace) {
        debug("cb\n");
        offt = i + 1;
        *offset = offt;
        return result;
      }
      debug("sep\n");
      offt = i + 1;
    } else {
      debug("bad character: \"%c\" @ %i\n", str[offt], offt);
      OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
      vartype_dtor(&result.val);
      result.val.typ = CMSG_NONE;
      return result;
    }
  }

  vartype_dtor(&result.val);
  OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
  return result;
}

static opt_vartype cmj_parse_array(char *str, size_t str_len, int *offset) {
  debug("cmj_parse_array: %s\n", str);
  opt_vartype result = cmsg_new_array();
  if (result.err != CMSG_ERR_OK) {
    return result;
  }

  int t_offt = 0;
  size_t offt = 1;
  size_t i;
  
  while (offt < str_len) {
    // key or closing brace
    if (str[offt] == ']') {
      offt = offt + 1;
      *offset = offt;
      return result;
    } else if (str[offt] == ' ') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\t') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\n') {
      //skip
      offt ++;
      continue;
    } else if (str[offt] == '\r') {
      //skip
      offt ++;
      continue;
    } else {
      opt_vartype val_ret = cmj_parse_json(str + offt, str_len - offt, &t_offt);
      if (val_ret.err != CMSG_ERR_OK) {
        OPT_PROPAGATE_ERROR(val_ret);
        vartype_dtor(&result.val);
        result.val.typ = CMSG_NONE;
        return result;
      }
      offt = offt + t_offt;
      cmsg_array_push(&result.val, &val_ret.val);
      vartype_dtor(&val_ret.val);
      //find separator or closing brace
      bool found_separator = false;
      bool found_closing_brace = false;
      // cmj_parse_json returns next symbol, so no need to add 1 here
      for (i = offt; i < str_len; i ++) {
        if (str[i] == ',') {
          found_separator = true;
          break;
        } else if (str[i] == ']') {
          found_closing_brace = true;
          break;
        }
      }
      if (found_separator == false && found_closing_brace == false) {
        debug("notsep, notcb\n");
        OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
        vartype_dtor(&result.val);
        result.val.typ = CMSG_NONE;
        return result;
      } else if (found_closing_brace) {
        debug("cb\n");
        offt = i + 1;
        *offset = offt;
        return result;
      }
      debug("sep\n");
      offt = i + 1;
    }
  }

  vartype_dtor(&result.val);
  OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
  return result;
}

opt_vartype cmj_parse_json(char *str, size_t str_len, int *offset) {
  if (str[0] == '{') {
    // hash
    return cmj_parse_hash(str, str_len, offset);
  } else if (str[0] == '[') {
    // array
    return cmj_parse_array(str, str_len, offset);
  } else if (str[0] == '"') {
    // string
    return cmj_parse_str(str, str_len, offset);
  }
  // scalar value
  return cmj_parse_scalar(str, str_len, offset);
}
