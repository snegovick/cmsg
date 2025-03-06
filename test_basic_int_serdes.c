#include <cgreen/cgreen.h>

Describe(BasicIntSerdes);
BeforeEach(BasicIntSerdes) {}
AfterEach(BasicIntSerdes) {}

#include "cmsg.h"

#include <stdint.h>
#include <inttypes.h>

Ensure(BasicIntSerdes, test_u8_serdes) {
  vartype t;
  t.typ = CMSG_U8;
  t.val.ival = 32;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_u8 val_ret = cmsg_vartype_to_u8(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == t.val.ival);
}

Ensure(BasicIntSerdes, test_i8_serdes) {
  vartype t;
  t.typ = CMSG_I8;
  t.val.ival = -32;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_i8 val_ret = cmsg_vartype_to_i8(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_i8 orig_val_ret = cmsg_vartype_to_i8(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);
}

Ensure(BasicIntSerdes, test_u16_serdes) {
  vartype t;
  t.typ = CMSG_U16;
  t.val.ival = 1200;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_u16 val_ret = cmsg_vartype_to_u16(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == t.val.ival);
}

Ensure(BasicIntSerdes, test_i16_serdes) {
  vartype t;
  t.typ = CMSG_I16;
  t.val.ival = -1200;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_i16 val_ret = cmsg_vartype_to_i16(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_i16 orig_val_ret = cmsg_vartype_to_i16(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);
}

Ensure(BasicIntSerdes, test_u32_serdes) {
  vartype t;
  t.typ = CMSG_U32;
  t.val.ival = 89123;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_u32 val_ret = cmsg_vartype_to_u32(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == t.val.ival);
}

Ensure(BasicIntSerdes, test_i32_serdes) {
  vartype t;
  t.typ = CMSG_I32;
  t.val.ival = -89123;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_i32 val_ret = cmsg_vartype_to_i32(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_i32 orig_val_ret = cmsg_vartype_to_i32(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);
}

Ensure(BasicIntSerdes, test_u64_serdes) {
  vartype t;
  t.typ = CMSG_U64;
  t.val.ival = 10000000000ULL;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_u64 val_ret = cmsg_vartype_to_u64(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == t.val.ival);
}

Ensure(BasicIntSerdes, test_i64_serdes) {
  vartype t;
  t.typ = CMSG_I64;
  t.val.ival = -10000000000LL;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_i64 val_ret = cmsg_vartype_to_i64(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_i64 orig_val_ret = cmsg_vartype_to_i64(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);

}

int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, BasicIntSerdes, test_u8_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_i8_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_u16_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_i16_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_u32_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_i32_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_u64_serdes);
  add_test_with_context(suite, BasicIntSerdes, test_i64_serdes);
    
  return run_test_suite(suite, create_text_reporter());
}
