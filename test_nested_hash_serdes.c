#include <cgreen/cgreen.h>

Describe(NestedHashSerdes);
BeforeEach(NestedHashSerdes) {}
AfterEach(NestedHashSerdes) {}

#include "cmsg.h"

#include <stdint.h>
#include <inttypes.h>

Ensure(NestedHashSerdes, test_nested_hash_serdes) {
  opt_vartype e1 = cmsg_new_str("element 1");
  assert_that(e1.err == CMSG_ERR_OK);

  opt_vartype e2 = cmsg_new_str("element 2");
  assert_that(e2.err == CMSG_ERR_OK);

  opt_vartype e3 = cmsg_new_var_int8(-80);
  assert_that(e3.err == CMSG_ERR_OK);

  opt_vartype e4 = cmsg_new_str("test str");
  assert_that(e4.err == CMSG_ERR_OK);

  opt_vartype nested = cmsg_new_hash();
  assert_that(nested.err == CMSG_ERR_OK);

  opt_vartype e5 = cmsg_new_str("test str 5");
  assert_that(e5.err == CMSG_ERR_OK);

  opt_vartype e6 = cmsg_new_str("test str 6");
  assert_that(e6.err == CMSG_ERR_OK);

  opt_vartype e7 = cmsg_new_var_uint8(0);
  assert_that(e7.err == CMSG_ERR_OK);

  opt_vartype root = cmsg_new_hash();
  assert_that(root.err == CMSG_ERR_OK);

  cmsg_hash_add(&root.val, "k1", &e1.val);

  cmsg_hash_add(&root.val, "k2", &e2.val);

  cmsg_hash_add(&root.val, "num", &e3.val);

  cmsg_hash_add(&root.val, "str", &e4.val);

  cmsg_hash_add(&nested.val, "k5", &e5.val);
  
  cmsg_hash_add(&nested.val, "k6", &e6.val);

  cmsg_hash_add(&nested.val, "k7", &e7.val);

  cmsg_hash_add(&root.val, "nested", &nested.val);

  opt_cmsgbuf ser_ret = cmsg_serialize(&root.val);
  assert_that(ser_ret.err == CMSG_ERR_OK);

  cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  assert_that(par_ret.val.typ == CMSG_HASH);

  opt_cmsgiter hash_iter_ret = cmsg_hash_get_iter(&par_ret.val);
  assert_that(hash_iter_ret.err == CMSG_ERR_OK);
  while (1) {
    opt_hashdatap elt_ret = cmsg_hash_next(&hash_iter_ret.val);
    if (elt_ret.err == CMSG_ERR_OUT_OF_BOUNDS) {
      break;
    }
    assert_that(elt_ret.err == CMSG_ERR_OK);
  }

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  assert_that(s, is_equal_to_string("{\"k1\": \"element 1\", \"k2\": \"element 2\", \"num\": -80, \"str\": \"test str\", \"nested\": {\"k5\": \"test str 5\", \"k6\": \"test str 6\", \"k7\": 0}}"));
  free(s);
  vartype_dtor(&e1);
  vartype_dtor(&e2);
  vartype_dtor(&e3);
  vartype_dtor(&e4);
  vartype_dtor(&e5);
  vartype_dtor(&e6);
  vartype_dtor(&nested);
  vartype_dtor(&root);
  vartype_dtor(&par_ret.val);
}

int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, NestedHashSerdes, test_nested_hash_serdes);

  return run_test_suite(suite, create_text_reporter());
}
