#include <cgreen/cgreen.h>

Describe(BasicStrSerdes);
BeforeEach(BasicStrSerdes) {}
AfterEach(BasicStrSerdes) {}

#include "cmsg.h"

#include <stdint.h>
#include <inttypes.h>

Ensure(BasicStrSerdes, test_str_serdes) {
  opt_vartype str_ret = cmsg_new_str("test str");
  assert_that(str_ret.err == CMSG_ERR_OK);  

  opt_cmsgbuf ser_ret = cmsg_serialize(&str_ret.val);
  assert_that(ser_ret.err == CMSG_ERR_OK);
  // cmsg_print_hex_buffer(ser_ret.val.data, ser_ret.val.len);

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  assert_that(par_ret.err == CMSG_ERR_OK);

  char * s = cmsg_str_var_print(&par_ret.val);

  assert_that(s, is_equal_to_string("test str"));

  free(s);
}

int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, BasicStrSerdes, test_str_serdes);
    
  return run_test_suite(suite, create_text_reporter());
}
