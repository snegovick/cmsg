#include <cgreen/cgreen.h>

Describe(ArraySerdes);
BeforeEach(ArraySerdes) {}
AfterEach(ArraySerdes) {}

#include "cmsg.h"

#include <stdint.h>
#include <inttypes.h>

#define debug printf

Ensure(ArraySerdes, test_array_serdes) {
  opt_vartype e1 = cmsg_new_str("element 1");
  assert_that(e1.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(e1, "failed to create e1\n");

  opt_vartype e2 = cmsg_new_str("element 2");
  assert_that(e2.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(e2, "failed to create e2\n");

  opt_vartype e3 = cmsg_new_var_int8(-80);
  assert_that(e3.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(e3, "failed to create e3\n");

  opt_vartype e4 = cmsg_new_str("test str");
  assert_that(e4.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(e4, "failed to create e4\n");

  opt_vartype array = cmsg_new_array();
  assert_that(array.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(array, "failed to create array\n");

  cmsg_array_push(&array.val, &e1.val);

  cmsg_array_push(&array.val, &e2.val);

  cmsg_array_push(&array.val, &e3.val);

  cmsg_array_push(&array.val, &e4.val);

  opt_cmsgbuf ser_ret = cmsg_serialize(&array.val);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(ser_ret, "failed to serialize data\n");

  cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);
  CHECK_RETURNV_OPT(par_ret, "failed to parse data\n");

  assert_that(par_ret.val.typ == CMSG_ARRAY);

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  assert_that(s, is_equal_to_string("[\"element 1\", \"element 2\", -80, \"test str\"]"));
  free(s);
}

int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, ArraySerdes, test_array_serdes);
    
  return run_test_suite(suite, create_text_reporter());
}
