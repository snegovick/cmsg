#include <cgreen/cgreen.h>

Describe(BasicRealSerdes);
BeforeEach(BasicRealSerdes) {}
AfterEach(BasicRealSerdes) {}

#include "cmsg.h"

#include <stdint.h>
#include <inttypes.h>

Ensure(BasicRealSerdes, test_float_serdes) {
  vartype t;
  t.typ = CMSG_F;
  t.val.fval = 1.023f;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_float val_ret = cmsg_vartype_to_f(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_float orig_val_ret = cmsg_vartype_to_f(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);
}

Ensure(BasicRealSerdes, test_double_serdes) {
  vartype t;
  t.typ = CMSG_D;
  t.val.dval = 1.02000003;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  //cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  opt_double val_ret = cmsg_vartype_to_d(&par_ret.val);
  assert_that(val_ret.err == CMSG_ERR_OK);

  opt_double orig_val_ret = cmsg_vartype_to_d(&par_ret.val);
  assert_that(orig_val_ret.err == CMSG_ERR_OK);

  assert_that(val_ret.val == orig_val_ret.val);
}


int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, BasicRealSerdes, test_float_serdes);
  add_test_with_context(suite, BasicRealSerdes, test_double_serdes);
    
  return run_test_suite(suite, create_text_reporter());
}
