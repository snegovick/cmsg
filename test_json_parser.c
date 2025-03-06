#include <cgreen/cgreen.h>

Describe(JsonParser);
BeforeEach(JsonParser) {}
AfterEach(JsonParser) {}


#include "cmsg.h"
#include "cmj.h"

#include <stdint.h>
#include <inttypes.h>

Ensure(JsonParser, test_json_parser) {
  char *text = malloc(10240);
  FILE *f = fopen("test.json", "r");
  size_t text_len = fread(text, 1, 10240, f);
  size_t actual_len = 0;
  while (text[actual_len] != '\n') {
    actual_len ++;
  }
  int offt = 0;
  opt_vartype par_ret = cmj_parse_json(text, text_len, &offt);
  assert_that(par_ret.err == CMSG_ERR_OK);
  if (par_ret.err != CMSG_ERR_OK) {
    free(text);
    printf("err: %i, str: %i\n", par_ret.err, par_ret.line);
    return;
  }

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  assert_that(actual_len == strlen(s));
  
  printf("source text len: %li, parsed len: %li\n", actual_len, strlen(s));
  text[actual_len] = 0;
  assert_that(s, is_equal_to_string(text));
  if (actual_len == strlen(s)) {
    for (size_t i = 0; i < actual_len; i++) {
      if (text[i] != s[i]) {
        assert_that(text[i] == s[i]);
        printf("mismatch @%li src: %02x, parsed: %02x\n", i, text[i], s[i]);
      }
    }
  }
  free(text);
  free(s);
}

int main(int argc, char **argv) {
  TestSuite *suite = create_test_suite();
  add_test_with_context(suite, JsonParser, test_json_parser);

  return run_test_suite(suite, create_text_reporter());
}
