#ifndef __CMSG_H__
#define __CMSG_H__

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include "utstring.h"
#include "utarray.h"
#include "uthash.h"

#define CMSG_PREFIX 0x1601

enum CMSG_ERR {
  CMSG_ERR_OK = 0,
  CMSG_ERR_NO_STR_LEN,
  CMSG_ERR_BAD_STR_LEN,
  CMSG_ERR_VALUE_IS_SIGNED,
  CMSG_ERR_INVALID_TYPE,
  CMSG_ERR_OUT_OF_BOUNDS,
  CMSG_ERR_HASH_PARSER_FAILED,
  CMSG_ERR_ARRAY_PARSER_FAILED,
  CMSG_ERR_UNKNOWN_PREFIX,
  CMSG_ERR_CAN_BE_TOO_LARGE,
  CMSG_ERR_NOT_INITIALIZED,
  CMSG_ERR_OPEN_FAILED,
  CMSG_ERR_READ_FAILED,
  CMSG_ERR_PARSER_FAILED,
  CMSG_ERR_N
};

#define DECL_OPT(typ)                           \
  typedef struct {                              \
    int err;                                    \
    int line;                                   \
    typ val;                                    \
  } opt_##typ;

enum CMSG_TYPE {
  CMSG_NONE = 0,
  CMSG_U8=2,
  CMSG_I8=4,
  CMSG_U16=6,
  CMSG_I16=8,
  CMSG_U32=10,
  CMSG_I32=12,
  CMSG_U64=14,
  CMSG_I64=16,
  CMSG_F=32,
  CMSG_D=33,
  CMSG_STR=64,
  CMSG_ARRAY=128,
  CMSG_HASH=129,
};

#define CMSG_INT_MASK 0x1e
#define CMSG_REAL_MASK 0x21
#define CMSG_STR_MASK 0x40
#define CMSG_CONTAINER_MASK 0x81

#define OPT_SET_ERROR(error)                      \
  result.err = error;                             \
  if (result.line == 0) {                         \
    result.line = __LINE__;                       \
  }

#define OPT_PROPAGATE_ERROR(opt)                  \
  result.err = opt.err;                           \
  result.line = opt.line;

#define OPT_SET_ERROR_FORCE(error)                \
  result.err = error;                             \
  result.line = __LINE__;

#define OPT_INIT_ERROR()                          \
  result.err = 0;                                 \
  result.line = 0;

#define CHECK_BUFFER_LEN(size, requested_len, error)          \
  if (size < requested_len) {                                 \
    result.err = error;                                       \
    result.line = __LINE__;                                   \
    return result;                                            \
  }

#define CHECK_RETURN_OPT(opt, errmsg, retval)               \
  if (opt.err != CMSG_ERR_OK) {                             \
    debug("Error[%i@%i]: %s", opt.err, opt.line, errmsg);   \
    return retval;                                          \
  }

#define CHECK_RETURNV_OPT(opt, errmsg)                     \
  if (opt.err != CMSG_ERR_OK) {                            \
    debug("Error[%i@%i]: %s", opt.err, opt.line, errmsg);  \
    return;                                                \
  }

struct hashdata;
typedef struct hashdata_s hashdata;
struct vartype_s {
  uint8_t typ;
  union {
    uint64_t ival;
    float fval;
    double dval;
    UT_string *str;
    UT_array *array;
    hashdata *hash;
  } val;
};
typedef struct vartype_s vartype;
typedef struct vartype_s * vartypep;
typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint8_t u8;
typedef int8_t i8;
typedef UT_string * utstrp;
typedef char * charp;

struct cmsg_buffer_s {
  uint32_t len;
  uint8_t *data;
};
typedef struct cmsg_buffer_s cmsgbuf;

struct cmsg_iter_s {
  hashdata *root;
  hashdata *p;
  UT_array *aroot;
  vartype *ap;
};
typedef struct cmsg_iter_s cmsgiter;

DECL_OPT(cmsgiter);
DECL_OPT(vartype);
DECL_OPT(vartypep);
DECL_OPT(u64);
DECL_OPT(i64);
DECL_OPT(u32);
DECL_OPT(i32);
DECL_OPT(u16);
DECL_OPT(i16);
DECL_OPT(u8);
DECL_OPT(i8);
DECL_OPT(float);
DECL_OPT(double);
DECL_OPT(utstrp);
DECL_OPT(charp);
DECL_OPT(cmsgbuf);

struct hashdata_s {
  char *k;
  vartype v;
  UT_hash_handle hh;
};
typedef struct hashdata_s hashdata;
typedef struct hashdata_s * hashdatap;

DECL_OPT(hashdatap);

// Copies basic vartypes i.e. integers, floats and str.
extern void vartype_copy(void *_dst, const void *_src);
extern void vartype_dtor(void *_elt);

// CMSG procedures forward declarations
extern char *cmsg_err_to_str(enum CMSG_ERR err);
extern vartype *cmsg_vartype_new(void);
extern void cmsg_vartype_free(vartype *v);
extern opt_u64 cmsg_int_var_parse(uint8_t *buffer, size_t size, vartype *v);
extern opt_u64 cmsg_real_var_parse(uint8_t *buffer, size_t size, vartype *v);
extern opt_u64 cmsg_vartype_to_u64(vartype *v);
extern opt_u32 cmsg_vartype_to_u32(vartype *v);
extern opt_u16 cmsg_vartype_to_u16(vartype *v);
extern opt_u8 cmsg_vartype_to_u8(vartype *v);
extern opt_i64 cmsg_vartype_to_i64(vartype *v);
extern opt_i32 cmsg_vartype_to_i32(vartype *v);
extern opt_i16 cmsg_vartype_to_i16(vartype *v);
extern opt_i8 cmsg_vartype_to_i8(vartype *v);
extern opt_float cmsg_vartype_to_f(vartype *v);
extern opt_double cmsg_vartype_to_d(vartype *v);
extern char *cmsg_vartype_to_str(vartype *v);
extern opt_u64 cmsg_str_var_parse(uint8_t *buffer, size_t size, vartype *v);
extern opt_u64 cmsg_container_var_parse(uint8_t *buffer, size_t size, vartype *v);
extern opt_u64 cmsg_var_parse(uint8_t *buffer, size_t size, vartype *v);
extern opt_vartype cmsg_parse(uint8_t *buffer, size_t size);
extern opt_cmsgbuf cmsg_int_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_real_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_hash_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_array_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_str_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_var_serialize(vartype *v);
extern opt_cmsgbuf cmsg_serialize(vartype *root);
extern void cmsg_print_hex_buffer(uint8_t *buf, size_t len);
extern opt_vartype cmsg_new_var_uint8(uint8_t val);
extern opt_vartype cmsg_new_var_int8(int8_t val);
extern opt_vartype cmsg_new_var_uint16(uint16_t val);
extern opt_vartype cmsg_new_var_int16(int16_t val);
extern opt_vartype cmsg_new_var_uint32(uint32_t val);
extern opt_vartype cmsg_new_var_int32(int32_t val);
extern opt_vartype cmsg_new_var_uint64(uint64_t val);
extern opt_vartype cmsg_new_var_int64(int64_t val);
extern opt_vartype cmsg_new_var_f(float val);
extern opt_vartype cmsg_new_var_d(double val);
extern opt_vartype cmsg_new_str(char *str);
extern opt_vartype cmsg_new_array(void);
extern opt_cmsgiter cmsg_array_get_iter(vartype *array);
extern opt_vartypep cmsg_array_next(cmsgiter *iter);
extern opt_vartypep cmsg_array_get(vartype *array, int idx);
extern void cmsg_array_pop_back(vartype *array);
extern void cmsg_array_push(vartype *array, vartype *elt);
extern opt_u64 cmsg_array_size(vartype *array);
extern opt_vartype cmsg_new_hash(void);
extern void cmsg_hash_add(vartype *hash, char *key, vartype *elt);
extern opt_u64 cmsg_hash_count(vartype *hash);
extern opt_cmsgiter cmsg_hash_get_iter(vartype *hash);
extern opt_hashdatap cmsg_hash_next(cmsgiter *iter);
extern opt_hashdatap cmsg_hash_find(vartype *hash, char *k);
extern vartype *cmsg_hashdatap_to_var(hashdatap p);
extern char *cmsg_hashdatap_to_key(hashdatap p);
extern void cmsg_hash_remove(vartype *hash, char *k);
extern char *cmsg_hash_var_print(vartype *v);
extern char *cmsg_array_var_print(vartype *v);
extern char *cmsg_str_var_print(vartype *v);
extern char *cmsg_real_var_print(vartype *v);
extern char *cmsg_int_var_print(vartype *v);
extern char *cmsg_var_print(vartype *v);
extern int decimal_length_u64(uint64_t v);
extern int decimal_length_i64(int64_t v);

#endif/*__CMSG_H__*/
