#include "cmsg.h"

//#define debug printf
#define debug(...)

static char cmsg_err_ok[] = "OK";
static char cmsg_err_no_str_len[] = "no strlen";
static char cmsg_err_bad_str_len[] = "bad strlen";
static char cmsg_err_value_is_signed[] = "value is signed";
static char cmsg_err_invalid_type[] = "invalid type";
static char cmsg_err_out_of_bounds[] = "out of bounds";
static char cmsg_err_hash_parser_failed[] = "hash parser failed";
static char cmsg_err_array_parser_failed[] = "array parser failed";
static char cmsg_err_unknown_prefix[] = "unknown prefix";
static char cmsg_err_can_be_too_large[] = "can be too large";
static char cmsg_err_not_initialized[] = "not initialized";
static char cmsg_err_open_failed[] = "open failed";
static char cmsg_err_read_failed[] = "read failed";
static char cmsg_err_parser_failed[] = "parser failed";

static char *cmsg_err_strs[] = {
  cmsg_err_ok,
  cmsg_err_no_str_len,
  cmsg_err_bad_str_len,
  cmsg_err_value_is_signed,
  cmsg_err_invalid_type,
  cmsg_err_out_of_bounds,
  cmsg_err_hash_parser_failed,
  cmsg_err_array_parser_failed,
  cmsg_err_unknown_prefix,
  cmsg_err_can_be_too_large,
  cmsg_err_not_initialized,
  cmsg_err_open_failed,
  cmsg_err_read_failed,
  cmsg_err_parser_failed,
};

char *cmsg_err_to_str(enum CMSG_ERR err) {
  if (err < CMSG_ERR_N) {
    return cmsg_err_strs[err];
  }
  return NULL;
}

// Copies basic vartypes i.e. integers, floats and str.
bool vartype_basic_copy(vartype *dst, vartype *src) {
  switch (dst->typ) {
  case CMSG_U8:
  case CMSG_I8:
  case CMSG_U16:
  case CMSG_I16:
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
    dst->typ = src->typ;
    dst->val.ival = src->val.ival;
    break;
  case CMSG_F:
    dst->typ = src->typ;
    dst->val.fval = src->val.fval;
    break;
  case CMSG_D:
    dst->typ = src->typ;
    dst->val.dval = src->val.dval;
    break;
  case CMSG_STR:
    dst->typ = src->typ;
    utstring_new(dst->val.str);
    utstring_concat(dst->val.str, src->val.str);
    break;
  default:
    return false;
  }
  return true;
}

void vartype_copy(void *_dst, const void *_src);
void vartype_dtor(void *_elt);
void vartype_dtor_free(vartype *v);
UT_icd vartype_icd = {sizeof(vartype), NULL, vartype_copy, vartype_dtor};

void vartype_copy(void *_dst, const void *_src) {
  vartype *dst = (vartype *)_dst;
  vartype *src = (vartype *)_src;
  dst->typ = src->typ;
  switch (dst->typ) {
  case CMSG_NONE:
  case CMSG_U8:
  case CMSG_I8:
  case CMSG_U16:
  case CMSG_I16:
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
  case CMSG_STR:
  case CMSG_F:
  case CMSG_D:
    if (vartype_basic_copy(dst, src) == false) {
      debug("ERROR: basic vartype copy failed");
    }
    break;
  case CMSG_ARRAY:
  {
    vartype *p = NULL;
    utarray_new(dst->val.array, &vartype_icd);
    while((p = (vartype *)utarray_next(src->val.array, p))) {
      vartype *d = malloc(sizeof(vartype));
      memset(d, 0, sizeof(vartype));
      vartype_copy(d, p);
      utarray_push_back(dst->val.array, d);
    }
  }
    break;
  case CMSG_HASH:
  {
    dst->val.hash = NULL;
    hashdata *p = NULL;
    hashdata *tmp = NULL;
    HASH_ITER(hh, src->val.hash, p, tmp) {
      //HASH_DEL(v->val.val.hash, p);
      hashdata *n = malloc(sizeof(hashdata));
      memset(n, 0, sizeof(hashdata));
      int l = strlen(p->k);
      n->k = malloc(l + 1);
      n->k[l] = 0;
      memcpy(n->k, p->k, l);
      vartype_copy(&n->v, &p->v);
      HASH_ADD_KEYPTR(hh, dst->val.hash, n->k, l, n);
    }
  }
    break;
  }
}

void vartype_dtor(void *_elt) {
  vartype *elt = (vartype*)_elt;
  switch (elt->typ) {
  case CMSG_U8:
  case CMSG_I8:
  case CMSG_U16:
  case CMSG_I16:
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
  case CMSG_F:
  case CMSG_D:
    break;
  case CMSG_STR:
    utstring_free(elt->val.str);
    break;
  case CMSG_ARRAY:
    utarray_free(elt->val.array);
    break;
  case CMSG_HASH:
  {
    hashdata *p, *tmp = NULL;
    HASH_ITER(hh, elt->val.hash, p, tmp) {
      HASH_DEL(elt->val.hash, p);
      vartype_dtor(&p->v);
      free(p);
    }
  }
    break;
  }
  elt->typ = CMSG_NONE;
}

void vartype_dtor_free(vartype *v) {
  vartype_dtor(v);
  free(v);
}

#define CMSG_PREFIX 0x1601

// CMSG procedures forward declarations
opt_u64 cmsg_var_parse(uint8_t *buffer, size_t size, vartype *v);
opt_cmsgbuf cmsg_var_serialize(vartype *v);
char *cmsg_int_var_print(vartype *v);
char *cmsg_real_var_print(vartype *v);
char *cmsg_str_var_print(vartype *v);
char *cmsg_array_var_print(vartype *v);
char *cmsg_hash_var_print(vartype *v);
opt_u64 cmsg_vartype_to_u64(vartype *v);
opt_i64 cmsg_vartype_to_i64(vartype *v);
opt_u32 cmsg_vartype_to_u32(vartype *v);
opt_i32 cmsg_vartype_to_i32(vartype *v);
opt_u16 cmsg_vartype_to_u16(vartype *v);
opt_i16 cmsg_vartype_to_i16(vartype *v);
opt_u8 cmsg_vartype_to_u8(vartype *v);
opt_i8 cmsg_vartype_to_i8(vartype *v);
opt_float cmsg_vartype_to_f(vartype *v);
opt_double cmsg_vartype_to_d(vartype *v);

vartype *cmsg_vartype_new(void) {
  vartype *v = malloc(sizeof(vartype));
  memset(v, 0, sizeof(vartype));
  return v;
}

void cmsg_vartype_free(vartype *v) {
  vartype_dtor(v);
  free(v);
}

//returns offset
opt_u64 cmsg_int_var_parse(uint8_t *buffer, size_t size, vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  result.val = 0;

  CHECK_BUFFER_LEN(size, sizeof(uint8_t), CMSG_ERR_OUT_OF_BOUNDS);

  int offt = 1;

  switch (buffer[0] & CMSG_INT_MASK) {
  case CMSG_U8:
    CHECK_BUFFER_LEN(size - offt, sizeof(uint8_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(uint8_t));
    result.val = offt + sizeof(uint8_t);
    v->typ = CMSG_U8;
    break;
  case CMSG_I8:
    CHECK_BUFFER_LEN(size - offt, sizeof(int8_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(int8_t));
    result.val = offt + sizeof(int8_t);
    v->typ = CMSG_I8;
    break;
  case CMSG_U16:
    CHECK_BUFFER_LEN(size - offt, sizeof(uint16_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(uint16_t));
    result.val = offt + sizeof(uint16_t);
    v->typ = CMSG_U16;
    break;
  case CMSG_I16:
    CHECK_BUFFER_LEN(size - offt, sizeof(int16_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(int16_t));
    result.val = offt + sizeof(int16_t);
    v->typ = CMSG_I16;
    break;
  case CMSG_U32:
    CHECK_BUFFER_LEN(size - offt, sizeof(uint32_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(uint32_t));
    result.val = offt + sizeof(uint32_t);
    v->typ = CMSG_U32;
    break;
  case CMSG_I32:
    CHECK_BUFFER_LEN(size - offt, sizeof(int32_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(int32_t));
    result.val = offt + sizeof(int32_t);
    v->typ = CMSG_I32;
    break;
  case CMSG_U64:
    CHECK_BUFFER_LEN(size - offt, sizeof(uint64_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(uint64_t));
    result.val = offt + sizeof(uint64_t);
    v->typ = CMSG_U64;
    break;
  case CMSG_I64:
    CHECK_BUFFER_LEN(size - offt, sizeof(int64_t), CMSG_ERR_OUT_OF_BOUNDS);
    v->val.ival = 0;
    memcpy(&v->val.ival, buffer + offt, sizeof(int64_t));
    result.val = offt + sizeof(int64_t);
    v->typ = CMSG_I64;
    break;
  default:
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    v->typ = CMSG_NONE;
    break;
  }
  return result;
}

//returns offset
opt_u64 cmsg_real_var_parse(uint8_t *buffer, size_t size, vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  result.val = 0;

  CHECK_BUFFER_LEN(size, sizeof(uint8_t), CMSG_ERR_OUT_OF_BOUNDS);

  int offt = 1;

  switch (buffer[0] & CMSG_REAL_MASK) {
  case CMSG_F:
    CHECK_BUFFER_LEN(size - offt, sizeof(float), CMSG_ERR_OUT_OF_BOUNDS);
    memcpy(&v->val.fval, buffer + offt, sizeof(float));
    result.val = offt + sizeof(float);
    v->typ = CMSG_F;
    break;
  case CMSG_D:
    CHECK_BUFFER_LEN(size - offt, sizeof(double), CMSG_ERR_OUT_OF_BOUNDS);
    memcpy(&v->val.dval, buffer + offt, sizeof(double));
    result.val = offt + sizeof(double);
    v->typ = CMSG_D;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    v->typ = CMSG_NONE;
    break;
  }
  return result;
}

//returns result
opt_u64 cmsg_vartype_to_u64(vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U32:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint32_t));
    break;
  case CMSG_I32:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U64:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint64_t));
    break;
  case CMSG_I64:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_u32 cmsg_vartype_to_u32(vartype *v) {
  opt_u32 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U32:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint32_t));
    break;
  case CMSG_I32:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_u16 cmsg_vartype_to_u16(vartype *v) {
  opt_u16 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  case CMSG_U16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_u8 cmsg_vartype_to_u8(vartype *v) {
  opt_u8 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.err = CMSG_ERR_VALUE_IS_SIGNED;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_i64 cmsg_vartype_to_i64(vartype *v) {
  opt_i64 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
  {
    result.val = 0;
    opt_i8 ret = cmsg_vartype_to_i8(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
  {
    result.val = 0;
    opt_i16 ret = cmsg_vartype_to_i16(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U32:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint32_t));
    break;
  case CMSG_I32:
  {
    result.val = 0;
    opt_i32 ret = cmsg_vartype_to_i32(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U64:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  case CMSG_I64:
    result.val = v->val.ival;
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_i32 cmsg_vartype_to_i32(vartype *v) {
  opt_i32 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
  {
    result.val = 0;
    opt_i8 ret = cmsg_vartype_to_i8(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
  {
    result.val = 0;
    opt_i16 ret = cmsg_vartype_to_i16(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U32:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  case CMSG_I32:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(int32_t));
    break;
  case CMSG_U64:
  case CMSG_I64:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_i16 cmsg_vartype_to_i16(vartype *v) {
  opt_i16 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
  {
    result.val = 0;
    opt_i8 ret = cmsg_vartype_to_i8(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      break;
    }
    result.val = ret.val;
  }
    break;
  case CMSG_U16:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  case CMSG_I16:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(int16_t));
    break;
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_i8 cmsg_vartype_to_i8(vartype *v) {
  opt_i8 result;
  OPT_INIT_ERROR();
  switch (v->typ) {
  case CMSG_U8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.val = 0;
    memcpy(&result.val, &v->val.ival, sizeof(int8_t));
    break;
  case CMSG_U16:
  case CMSG_I16:
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
    OPT_SET_ERROR(CMSG_ERR_CAN_BE_TOO_LARGE);
    break;
  default:
    result.err = CMSG_ERR_INVALID_TYPE;
    break;
  }
  return result;
}

opt_float cmsg_vartype_to_f(vartype *v) {
  opt_float result;
  OPT_INIT_ERROR();
  if (v->typ != CMSG_F) {
    result.err = CMSG_ERR_INVALID_TYPE;
  } else {
    result.val = v->val.fval;
  }
  return result;
}

opt_double cmsg_vartype_to_d(vartype *v) {
  opt_double result;
  OPT_INIT_ERROR();
  if (v->typ != CMSG_D) {
    result.err = CMSG_ERR_INVALID_TYPE;
  } else {
    result.val = v->val.dval;
  }
  return result;
}

opt_u64 cmsg_str_var_parse(uint8_t *buffer, size_t size, vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  result.val = 0;

  debug("str buffer0: %02x\n", buffer[0]);
  if ((buffer[0] & CMSG_STR_MASK) != CMSG_STR) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  }

  uint64_t offt = 0;

  vartype len_var;
  opt_u64 ret = cmsg_int_var_parse(buffer, size, &len_var);
  if (ret.err != CMSG_ERR_OK) {
    result.err = CMSG_ERR_NO_STR_LEN;
    return result;
  }

  offt = ret.val;
  
  ret = cmsg_vartype_to_u64(&len_var);
  if (ret.err != CMSG_ERR_OK) {
    result.err = CMSG_ERR_BAD_STR_LEN;
    return result;
  }

  uint64_t len = ret.val;
  
  CHECK_BUFFER_LEN(size - offt, len, CMSG_ERR_OUT_OF_BOUNDS);

  v->typ = CMSG_STR;
  utstring_new(v->val.str);
  utstring_bincpy(v->val.str, buffer + offt, len);

  offt += len;
  result.val = offt;
  return result;
}

opt_u64 cmsg_container_var_parse(uint8_t *buffer, size_t size, vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  result.val = 0;

  uint64_t offt = 0;

  bool is_array = false;
  v->typ = CMSG_HASH;
  if ((buffer[0] & CMSG_CONTAINER_MASK) == CMSG_ARRAY) {
    v->typ = CMSG_ARRAY;
    is_array = true;
  }

  vartype len_var;
  opt_u64 ret = cmsg_int_var_parse(buffer, size, &len_var);
  if (ret.err != CMSG_ERR_OK) {
    result.err = CMSG_ERR_NO_STR_LEN;
    return result;
  }

  offt = ret.val;
  
  ret = cmsg_vartype_to_u64(&len_var);
  if (ret.err != CMSG_ERR_OK) {
    result.err = CMSG_ERR_BAD_STR_LEN;
    return result;
  }

  uint64_t len = ret.val;

  if (is_array) {
    utarray_new(v->val.array, &vartype_icd);
  } else {
    v->val.hash = NULL;
  }
  
  //CHECK_BUFFER_LEN(size - offt, len, CMSG_ERR_OUT_OF_BOUNDS);
  uint64_t i;
  for (i = 0; i < len; i++) {
    if (is_array) {
      vartype *t = malloc(sizeof(vartype));
      ret = cmsg_var_parse(buffer + offt, size - offt, t);
      if (ret.err != CMSG_ERR_OK) {
        OPT_SET_ERROR(CMSG_ERR_ARRAY_PARSER_FAILED);
        vartype_dtor(t);
        free(t);
        return result;
      }
      offt += ret.val;
      utarray_push_back(v->val.array, t);
    } else {
      vartype t;
      hashdata *n = malloc(sizeof(hashdata));
      memset(n, 0, sizeof(hashdata));

      // first parse key
      ret = cmsg_str_var_parse(buffer + offt, size - offt, &t);
      if (ret.err != CMSG_ERR_OK) {
        OPT_PROPAGATE_ERROR(ret);
        vartype_dtor(&n->v);
        free(n);
        return result;
      }
      offt += ret.val;
      unsigned int l = utstring_len(t.val.str);
      n->k = malloc(l + 1);
      memcpy(n->k, utstring_body(t.val.str), l);
      n->k[l] = 0;
      // now parse value
      ret = cmsg_var_parse(buffer + offt, size - offt, &n->v);
      if (ret.err != CMSG_ERR_OK) {
        OPT_SET_ERROR(CMSG_ERR_HASH_PARSER_FAILED);
        vartype_dtor(&n->v);
        free(n->k);
        free(n);
        return result;
      }
      offt += ret.val;
      HASH_ADD_KEYPTR(hh, v->val.hash, n->k, l, n);
    }
  }

  result.val = offt;
  return result;
}

opt_u64 cmsg_var_parse(uint8_t *buffer, size_t size, vartype *v) {
  opt_u64 result;
  OPT_INIT_ERROR();
  result.val = 0;

  uint64_t offt = 0;

  if ((buffer[0] & CMSG_CONTAINER_MASK) && (buffer[0] & CMSG_ARRAY)) {
    debug("cmsg_var_parse container\n");
    opt_u64 ret = cmsg_container_var_parse(buffer, size, v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    offt = ret.val;    
  } else if (buffer[0] & CMSG_STR_MASK) {
    debug("cmsg_var_parse str\n");
    opt_u64 ret = cmsg_str_var_parse(buffer, size, v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    offt = ret.val;
  } else if ((buffer[0] == CMSG_F) || (buffer[0] == CMSG_D)) {
    debug("cmsg_var_parse real\n");
    opt_u64 ret = cmsg_real_var_parse(buffer, size, v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    offt = ret.val;
  } else if (buffer[0] & CMSG_INT_MASK) {
    debug("cmsg_var_parse int\n");
    opt_u64 ret = cmsg_int_var_parse(buffer, size, v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    offt = ret.val;
  }

  result.val = offt;
  return result;
}

opt_vartype cmsg_parse(uint8_t *buffer, size_t size) {
  opt_vartype result;
  result.val.typ = CMSG_NONE;
  OPT_INIT_ERROR();

  uint16_t prefix = CMSG_PREFIX;
  uint16_t t = 0;
  unsigned int offt = 0;
  
  memcpy(&t, buffer, sizeof(uint16_t));
  offt += sizeof(uint16_t);
  if (t != prefix) {
    OPT_SET_ERROR(CMSG_ERR_UNKNOWN_PREFIX);
    return result;
  }

  //while (offt < size) {
  CHECK_BUFFER_LEN(size - offt, sizeof(uint8_t), CMSG_ERR_OUT_OF_BOUNDS);
  opt_u64 ret = cmsg_var_parse(buffer + offt, size - offt, &result.val);
  if (ret.err != CMSG_ERR_OK) {
    OPT_PROPAGATE_ERROR(ret);
    return result;
  }
  offt += ret.val;
//}
  return result;
}

opt_cmsgbuf cmsg_int_var_serialize(vartype *v) {
  opt_cmsgbuf result;
  result.val.len = 0;
  result.val.data = 0;
  OPT_INIT_ERROR();

  switch (v->typ) {
  case CMSG_U8:
    result.val.len = 2;
    result.val.data = malloc(2);
    result.val.data[0] = CMSG_U8;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(uint8_t));
    break;
  case CMSG_I8:
    result.val.len = 2;
    result.val.data = malloc(2);
    result.val.data[0] = CMSG_I8;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(int8_t));
    break;
  case CMSG_U16:
    result.val.len = 3;
    result.val.data = malloc(3);
    result.val.data[0] = CMSG_U16;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(uint16_t));
    break;
  case CMSG_I16:
    result.val.len = 3;
    result.val.data = malloc(3);
    result.val.data[0] = CMSG_I16;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(int16_t));
    break;
  case CMSG_U32:
    result.val.len = 5;
    result.val.data = malloc(5);
    result.val.data[0] = CMSG_U32;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(uint32_t));
    break;
  case CMSG_I32:
    result.val.len = 5;
    result.val.data = malloc(5);
    result.val.data[0] = CMSG_I32;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(int32_t));
    break;
  case CMSG_U64:
    result.val.len = 9;
    result.val.data = malloc(9);
    result.val.data[0] = CMSG_U64;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(uint64_t));
    break;
  case CMSG_I64:
    result.val.len = 9;
    result.val.data = malloc(9);
    result.val.data[0] = CMSG_I64;
    memcpy(&result.val.data[1], &v->val.ival, sizeof(int64_t));
    break;
  default:
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    break;
  }
  return result;
}

opt_cmsgbuf cmsg_real_var_serialize(vartype *v) {
  opt_cmsgbuf result;
  result.val.len = 0;
  result.val.data = 0;
  OPT_INIT_ERROR();

  switch (v->typ) {
  case CMSG_F:
    result.val.len = 1 + sizeof(float);
    result.val.data = malloc(result.val.len);
    result.val.data[0] = CMSG_F;
    memcpy(&result.val.data[1], &v->val.fval, sizeof(float));
    break;
  case CMSG_D:
    result.val.len = 1 + sizeof(double);
    result.val.data = malloc(result.val.len);
    result.val.data[0] = CMSG_D;
    memcpy(&result.val.data[1], &v->val.fval, sizeof(double));
    break;
  default:
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    break;
  }
  return result;
}

static bool calc_size_type(uint64_t len, uint8_t *size_typ, unsigned int *size_size) {
  if (len <= 0xff) {
    *size_typ = CMSG_U8;
    *size_size = 1;
  } else if (len <= 0xffff) {
    *size_typ = CMSG_U16;
    *size_size = 2;
  } else if (len <= 0xffffffff) {
    *size_typ = CMSG_U32;
    *size_size = 4;
  } else if (len <= 0xffffffffffffffff) {
    *size_typ = CMSG_U64;
    *size_size = 8;
  } else {
    return false;
  }
  return true;
}

opt_cmsgbuf cmsg_hash_var_serialize(vartype *v) {
  debug("cmsg_hash_var_serialize\n");
  opt_cmsgbuf result;
  result.val.len = 0;
  result.val.data = 0;
  OPT_INIT_ERROR();

  if (v->typ != CMSG_HASH) {
    debug("Error: not a hash\n");
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  }

  uint64_t len = HASH_COUNT(v->val.hash);
  debug("Hash length: %i\n", len);

  uint8_t size_typ = 0;
  unsigned int size_size = 0;
  if (calc_size_type(len, &size_typ, &size_size) == false) {
    OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
    return result;
  }

  uint64_t size = 0;
  opt_cmsgbuf *bufs = malloc(sizeof(opt_cmsgbuf) * len * 2);

  uint64_t i = 0;
  hashdata *p = NULL;
  hashdata *tmp = NULL;
  HASH_ITER(hh, v->val.hash, p, tmp) {
    // serialize key
    uint8_t key_len_typ = 0;
    unsigned int key_len_size = 0;
    uint64_t key_len = 0;
    uint64_t l = strlen(p->k);
    debug("k: %s\n", p->k);
    if (calc_size_type(key_len, &key_len_typ, &key_len_size) == false) {
      free(bufs);
      OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
      return result;
    }
    key_len = l + 1 + key_len_size;
    bufs[i].val.len = key_len;
    bufs[i].val.data = malloc(key_len);
    bufs[i].val.data[0] = CMSG_STR | key_len_typ;
    memcpy(bufs[i].val.data + 1, &l, key_len_size);
    memcpy(bufs[i].val.data + 1 + key_len_size, p->k, l);
    size += key_len;
    i++;

    //serialize value
    debug("value type: %i\n", p->v.typ);
    bufs[i] = cmsg_var_serialize(&p->v);
    if (bufs[i].err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(bufs[i]);
      free(bufs);
      return result;
    }
    size += bufs[i].val.len;
    i++;
  }

  result.val.len = size + 1 + size_size;
  result.val.data = malloc(result.val.len);
  result.val.data[0] = CMSG_HASH | size_typ;
  memcpy(result.val.data + 1, &len, size_size);

  uint64_t offt = 0;
  for (uint64_t i = 0; i < (len * 2); i++) {
    memcpy(result.val.data + 1 + size_size + offt, bufs[i].val.data, bufs[i].val.len);
    offt += bufs[i].val.len;
  }

  return result;
}

opt_cmsgbuf cmsg_array_var_serialize(vartype *v) {
  opt_cmsgbuf result;
  result.val.len = 0;
  result.val.data = 0;
  OPT_INIT_ERROR();

  if (v->typ != CMSG_ARRAY) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  }

  uint64_t len = utarray_len(v->val.array);

  uint8_t size_typ = 0;
  unsigned int size_size = 0;
  if (calc_size_type(len, &size_typ, &size_size) == false) {
    OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
    return result;
  }

  uint64_t size = 0;
  opt_cmsgbuf *bufs = malloc(sizeof(opt_cmsgbuf) * len);

  for (uint64_t i = 0; i < len; i++) {
    vartype *t = utarray_eltptr(v->val.array, i);
    bufs[i] = cmsg_var_serialize(t);
    if (bufs[i].err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(bufs[i]);
      free(bufs);
      return result;
    }
    size += bufs[i].val.len;
  }

  result.val.len = size + 1 + size_size;
  result.val.data = malloc(result.val.len);
  result.val.data[0] = CMSG_ARRAY | size_typ;
  memcpy(result.val.data + 1, &len, size_size);

  uint64_t offt = 0;
  for (uint64_t i = 0; i < len; i++) {
    memcpy(result.val.data + 1 + size_size + offt, bufs[i].val.data, bufs[i].val.len);
    offt += bufs[i].val.len;
  }

  return result;
}

opt_cmsgbuf cmsg_str_var_serialize(vartype *v) {
  opt_cmsgbuf result;
  result.val.len = 0;
  result.val.data = 0;
  OPT_INIT_ERROR();
  
  if (v->typ != CMSG_STR) {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
    return result;
  }

  uint64_t len = utstring_len(v->val.str);

  uint8_t size_typ = 0;
  unsigned int size_size = 0;
  if (calc_size_type(len, &size_typ, &size_size) == false) {
    OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
    return result;
  }

  result.val.len = size_size + len + 1;
  result.val.data = malloc(result.val.len);
  memcpy(result.val.data + 1, &len, size_size);
  memcpy(result.val.data + 1 + size_size, utstring_body(v->val.str), len);
  result.val.data[0] = CMSG_STR | size_typ;

  return result;
}

opt_cmsgbuf cmsg_var_serialize(vartype *v) {
  opt_cmsgbuf result;
  OPT_INIT_ERROR();
  result.val.len = 0;

  if (v->typ == CMSG_ARRAY) {
    opt_cmsgbuf ret = cmsg_array_var_serialize(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    result.val.len = ret.val.len;
    memcpy(&result.val, &ret.val, sizeof(cmsgbuf));
  } else if (v->typ == CMSG_HASH) {
    opt_cmsgbuf ret = cmsg_hash_var_serialize(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    result.val.len = ret.val.len;
    memcpy(&result.val, &ret.val, sizeof(cmsgbuf));
  } else if (v->typ == CMSG_STR) {
    opt_cmsgbuf ret = cmsg_str_var_serialize(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    result.val.len = ret.val.len;
    memcpy(&result.val, &ret.val, sizeof(cmsgbuf));
  } else if ((v->typ & CMSG_INT_MASK) != 0) {
    opt_cmsgbuf ret = cmsg_int_var_serialize(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    result.val.len = ret.val.len;
    memcpy(&result.val, &ret.val, sizeof(cmsgbuf));
  } else if ((v->typ & CMSG_REAL_MASK) != 0) {
    opt_cmsgbuf ret = cmsg_real_var_serialize(v);
    if (ret.err != CMSG_ERR_OK) {
      OPT_PROPAGATE_ERROR(ret);
      return result;
    }
    result.val.len = ret.val.len;
    memcpy(&result.val, &ret.val, sizeof(cmsgbuf));
  }
  return result;
}

opt_cmsgbuf cmsg_serialize(vartype *root) {
  opt_cmsgbuf result;
  OPT_INIT_ERROR();
  result.val.len = 0;
  
  uint16_t prefix = CMSG_PREFIX;

  opt_cmsgbuf ret = cmsg_var_serialize(root);
  if (ret.err != CMSG_ERR_OK) {
    OPT_PROPAGATE_ERROR(ret);
    return result;
  }
  result.val.len = ret.val.len + sizeof(prefix);
  result.val.data = malloc(result.val.len);
  memcpy(result.val.data, &prefix, sizeof(prefix));
  memcpy(result.val.data + sizeof(prefix), ret.val.data, ret.val.len);
  return result;
}

void cmsg_print_hex_buffer(uint8_t *buf, size_t len) {
  unsigned int i = 0;
  for (; i < len; i++) {
    if ((i > 0) && (i % 8 == 0)) {
      debug("\n");
    }
    debug("%02x[%c] ", buf[i], ((buf[i] > ' ') && (buf[i] <= '~') ? buf[i] : ' '));
  }
  debug("\n");
}

opt_vartype cmsg_new_var_uint8(uint8_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_U8;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(uint8_t));
  return result;
}

opt_vartype cmsg_new_var_int8(int8_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_I8;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(int8_t));
  return result;
}

opt_vartype cmsg_new_var_uint16(uint16_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_U16;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(uint16_t));
  return result;
}

opt_vartype cmsg_new_var_int16(int16_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_I16;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(int16_t));
  return result;
}

opt_vartype cmsg_new_var_uint32(uint32_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_U32;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(uint32_t));
  return result;
}

opt_vartype cmsg_new_var_int32(int32_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_I32;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(int32_t));
  return result;
}

opt_vartype cmsg_new_var_uint64(uint64_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_U64;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(uint64_t));
  return result;
}

opt_vartype cmsg_new_var_int64(int64_t val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_I64;
  result.val.val.ival = 0;
  memcpy(&result.val.val.ival, &val, sizeof(int64_t));
  return result;
}

opt_vartype cmsg_new_var_f(float val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_F;
  result.val.val.fval = val;
  return result;
}

opt_vartype cmsg_new_var_d(double val) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_D;
  result.val.val.dval = val;
  return result;
}

opt_vartype cmsg_new_str(char *str) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_STR;
  utstring_new(result.val.val.str);
  utstring_bincpy(result.val.val.str, str, strlen(str));
  return result;
}

opt_vartype cmsg_new_array(void) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_ARRAY;
  utarray_new(result.val.val.array, &vartype_icd);
  return result;
}

opt_cmsgiter cmsg_array_get_iter(vartype *array) {
  opt_cmsgiter result;
  OPT_INIT_ERROR();
  if (array->typ == CMSG_ARRAY) {
    result.val.aroot = array->val.array;
    result.val.ap = (vartype *)utarray_front(result.val.aroot);
  } else {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
  }
  return result;
}

opt_vartypep cmsg_array_next(cmsgiter *iter) {
  opt_vartypep result;
  OPT_INIT_ERROR();

  if (iter->ap == NULL) {
    OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
    return result;
  }
  result.val = iter->ap;
  iter->ap = (vartype *)utarray_next(iter->aroot, iter->ap);
  return result;
}

void cmsg_array_push(vartype *array, vartype *elt) {
  if (array->typ == CMSG_ARRAY) {
    utarray_push_back(array->val.array, elt);
  }
}

opt_u64 cmsg_array_size(vartype *array) {
  opt_u64 result;
  OPT_INIT_ERROR();

  if (array->typ == CMSG_ARRAY) {
    result.val = utarray_len(array->val.array);
  } else {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
  }

  return result;
}

opt_vartypep cmsg_array_get(vartype *array, int idx) {
  opt_vartypep result;
  OPT_INIT_ERROR();
  if (array->typ == CMSG_ARRAY) {
    if (array->val.array == NULL) {
      OPT_SET_ERROR(CMSG_ERR_NOT_INITIALIZED);
    } else {
      if (idx >= 0) {
        result.val = utarray_eltptr(array->val.array, (unsigned int)idx);
      } else if (idx < 0) {
        int l = utarray_len(array->val.array);
        int real_idx = -idx;
        if (real_idx <= l) {
          real_idx = l - real_idx;
          result.val = utarray_eltptr(array->val.array, (unsigned int)real_idx);
        }
      }
    }
  } else {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
  }

  return result;
}

void cmsg_array_pop_back(vartype *array) {
  if (array->typ == CMSG_ARRAY) {
    if (array->val.array != NULL) {
      utarray_pop_back(array->val.array);
    }
  }
}

opt_vartype cmsg_new_hash(void) {
  opt_vartype result;
  OPT_INIT_ERROR();

  result.val.typ = CMSG_HASH;
  result.val.val.hash = NULL;
  return result;
}

void cmsg_hash_add(vartype *hash, char *key, vartype *elt) {
  debug("cmsg_hash_add\n");
  if (hash->typ == CMSG_HASH) {
    hashdata *n = malloc(sizeof(hashdata));
    memset(n, 0, sizeof(hashdata));
    int l = strlen(key);
    n->k = malloc(l + 1);
    n->k[l] = 0;
    memcpy(n->k, key, l);
    vartype_copy(&n->v, elt);
    HASH_ADD_KEYPTR(hh, hash->val.hash, n->k, l, n);
  } else {
    debug("Error: cmsg_hash_add called on non-hash vartype\n");
  }
}

void cmsg_hash_remove(vartype *hash, char *key) {
  debug("cmsg_hash_remove\n");
  if (hash->typ == CMSG_HASH) {
    hashdata *p = NULL;
    HASH_FIND_STR(hash->val.hash, key, p);
    if (p != NULL) {
      HASH_DEL(hash->val.hash, p);
      free(p);
    }
  }
}

opt_u64 cmsg_hash_count(vartype *hash) {
  opt_u64 result;
  OPT_INIT_ERROR();

  if (hash->typ == CMSG_HASH) {
    result.val = HASH_COUNT(hash->val.hash);
  } else {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
  }

  return result;
}

opt_cmsgiter cmsg_hash_get_iter(vartype *hash) {
  opt_cmsgiter result;
  OPT_INIT_ERROR();

  if (hash->typ == CMSG_HASH) {
    result.val.root = hash->val.hash;
    result.val.p = hash->val.hash;
    return result;
  } else {
    OPT_SET_ERROR(CMSG_ERR_INVALID_TYPE);
  }
  return result;
}

opt_hashdatap cmsg_hash_next(cmsgiter *iter) {
  opt_hashdatap result;
  OPT_INIT_ERROR();

  if (iter->p == NULL) {
    OPT_SET_ERROR(CMSG_ERR_OUT_OF_BOUNDS);
    return result;
  }

  result.val = iter->p;
  iter->p = iter->p->hh.next;
  return result;
}

opt_hashdatap cmsg_hash_find(vartype *hash, char *k) {
  opt_hashdatap result;
  OPT_INIT_ERROR();

  HASH_FIND_STR(hash->val.hash, k, result.val);
  return result;
}

int decimal_length_u64(uint64_t v) {
  int l = 1;
  if (v >= 10) {
    while (v != 0) {
      v = v / 10;
      l++;
    }
  }
  return l;
}

int decimal_length_i64(int64_t v) {
  int l = 1;
  if (v < 0) {
    l++;
    v = -v;
  }
  if (v >= 10) {
    while (v != 0) {
      v = v / 10;
      l++;
    }
  }
  return l;
}

char *cmsg_hash_var_print(vartype *v) {
  char * s = NULL;

  if (v->typ != CMSG_HASH) {
    return s;
  }

  UT_string *ustr = NULL;
  utstring_new(ustr);
  utstring_printf(ustr, "{");
  debug("ustr: %s\n", utstring_body(ustr));

  opt_cmsgiter iter = cmsg_hash_get_iter(v);
  if (iter.err != CMSG_ERR_OK) {
    utstring_free(ustr);
    return NULL;
  }

  opt_u64 count = cmsg_hash_count(v);
  if (count.err != CMSG_ERR_OK) {
    utstring_free(ustr);
    return NULL;
  }

  while (1) {
    vartype *p = NULL;
    opt_hashdatap elt_ret = cmsg_hash_next(&iter.val);
    if (elt_ret.err == CMSG_ERR_OUT_OF_BOUNDS) {
      break;
    }
    char *k = elt_ret.val->k;
    utstring_printf(ustr, "\"%s\": ", k);
    p = &elt_ret.val->v;
    debug("p->typ: %i\n", p->typ);
    
    char *elt_str = NULL;
    switch (p->typ) {
    case CMSG_U8:
    case CMSG_I8:
    case CMSG_U16:
    case CMSG_I16:
    case CMSG_U32:
    case CMSG_I32:
    case CMSG_U64:
    case CMSG_I64:
      elt_str = cmsg_int_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_F:
    case CMSG_D:
      elt_str = cmsg_real_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_STR:
      elt_str = cmsg_str_var_print(p);
      utstring_printf(ustr, "\"%s\"", elt_str);
      break;
    case CMSG_ARRAY:
      elt_str = cmsg_array_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_HASH:
      elt_str = cmsg_hash_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    default:
      break;
    }
    debug("elt: %s\n", elt_str);
    if (elt_str != NULL) {
      free(elt_str);
    }
    count.val --;
    if (count.val > 0) {
      utstring_printf(ustr, ", ");
    }

    debug("ustr: %s\n", utstring_body(ustr));
  }

  utstring_printf(ustr, "}");

  int l = utstring_len(ustr);
  
  s = malloc(l + 1);
  memcpy(s, utstring_body(ustr), l);
  utstring_free(ustr);
  s[l] = 0;
  
  return s;
}

char *cmsg_array_var_print(vartype *v) {
  char * s = NULL;

  if (v->typ != CMSG_ARRAY) {
    return s;
  }

  UT_string *ustr = NULL;
  utstring_new(ustr);
  utstring_printf(ustr, "[");
  vartype *p = NULL;
  unsigned int ctr = 0;
  while((p = (vartype *)utarray_next(v->val.array, p))) {
    char *elt_str = NULL;
    switch (p->typ) {
    case CMSG_U8:
    case CMSG_I8:
    case CMSG_U16:
    case CMSG_I16:
    case CMSG_U32:
    case CMSG_I32:
    case CMSG_U64:
    case CMSG_I64:
      elt_str = cmsg_int_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_F:
    case CMSG_D:
      elt_str = cmsg_real_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_STR:
      elt_str = cmsg_str_var_print(p);
      utstring_printf(ustr, "\"%s\"", elt_str);
      break;
    case CMSG_ARRAY:
      elt_str = cmsg_array_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    case CMSG_HASH:
      elt_str = cmsg_hash_var_print(p);
      utstring_printf(ustr, "%s", elt_str);
      break;
    default:
      break;
    }
    if (elt_str != NULL) {
      free(elt_str);
    }
    ctr ++;
    if (ctr < (utarray_len(v->val.array))) {
      utstring_printf(ustr, ", ");
    }
    debug("ustr: %s\n", utstring_body(ustr));
  }

  utstring_printf(ustr, "]");
  debug("ustr: %s\n", utstring_body(ustr));

  int l = utstring_len(ustr);
  
  s = malloc(l + 1);
  memcpy(s, utstring_body(ustr), l);
  utstring_free(ustr);
  s[l] = 0;
  
  return s;
}

char *cmsg_str_var_print(vartype *v) {
  char * s = NULL;

  if (v->typ != CMSG_STR) {
    return s;
  }

  int l = utstring_len(v->val.str);
  
  s = malloc(l + 1);
  memcpy(s, utstring_body(v->val.str), l);
  s[l] = 0;
  
  return s;
}

char *cmsg_real_var_print(vartype *v) {
  char *s = NULL;
  switch (v->typ) {
  case CMSG_F:
  {
    char t[512];
    opt_float ret = cmsg_vartype_to_f(v);
    snprintf(t, 128, "%f", ret.val);
    int l = strlen(t);
    s = malloc(l + 1);
    memcpy(s, t, l);
    s[l] = 0;
  }
  break;
  case CMSG_D:
  {
    char t[512];
    opt_double ret = cmsg_vartype_to_d(v);
    snprintf(t, 128, "%f", ret.val);
    int l = strlen(t);
    s = malloc(l + 1);
    memcpy(s, t, l);
    s[l] = 0;
  }
  break;
  default:
    break;
  }
  return s;
}

char *cmsg_int_var_print(vartype *v) {
  char *s = NULL;
  switch (v->typ) {
  case CMSG_U8:
  {
    int l = decimal_length_u64(v->val.ival) + 1;
    opt_u8 ret = cmsg_vartype_to_u8(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
    debug("allocated %i bytes for uint8, value: %i\n", l, ret.val);
  }
  break;
  case CMSG_U16:
  {
    int l = decimal_length_u64(v->val.ival) + 1;
    opt_u16 ret = cmsg_vartype_to_u16(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_U32:
  {
    int l = decimal_length_u64(v->val.ival) + 1;
    opt_u32 ret = cmsg_vartype_to_u32(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_I8:
  {
    int l = decimal_length_i64(v->val.ival) + 1;
    opt_i8 ret = cmsg_vartype_to_i8(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_I16:
  {
    int l = decimal_length_i64(v->val.ival) + 1;
    opt_i16 ret = cmsg_vartype_to_i16(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_I32:    
  {
    int l = decimal_length_i64(v->val.ival) + 1;
    opt_i32 ret = cmsg_vartype_to_i32(v);
    s = malloc(l);
    snprintf(s, l, "%i", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_U64:
  {
    int l = decimal_length_u64(v->val.ival) + 1;
    opt_u64 ret = cmsg_vartype_to_u64(v);
    s = malloc(l);
    snprintf(s, l, "%"PRIu64"", ret.val);
    s[l] = 0;
  }
  break;
  case CMSG_I64:
  {
    int l = decimal_length_i64(v->val.ival) + 1;
    opt_i64 ret = cmsg_vartype_to_i64(v);
    s = malloc(l);
    snprintf(s, l, "%"PRIi64"", ret.val);
    s[l] = 0;
  }
  break;
  default:
    break;
  }
  return s;
}

char *cmsg_var_print(vartype *v) {
  char *s = NULL;

  switch (v->typ) {
  case CMSG_U8:
  case CMSG_I8:
  case CMSG_U16:
  case CMSG_I16:
  case CMSG_U32:
  case CMSG_I32:
  case CMSG_U64:
  case CMSG_I64:
    s = cmsg_int_var_print(v);
    break;
  case CMSG_F:
  case CMSG_D:
    s = cmsg_real_var_print(v);
    break;
  case CMSG_STR:
    s = cmsg_str_var_print(v);
    break;
  case CMSG_ARRAY:
    s = cmsg_array_var_print(v);
    break;
  case CMSG_HASH:
    s = cmsg_hash_var_print(v);
    break;
  default:
    break;
  }

  return s;
}

char *cmsg_vartype_to_str(vartype *v) {
  if (v->typ == CMSG_STR) {
    return utstring_body(v->val.str);
  }
  return NULL;
}

vartype *cmsg_hashdatap_to_var(hashdatap p) {
  return &p->v;
}

char *cmsg_hashdatap_to_key(hashdatap p) {
  return p->k;
}
