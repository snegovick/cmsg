# CMSG - c library for data serialization/deserialization

This library solves the issue with modern *JSON* libs for C which are quite huge and complex to use nowadays. Besides that sometimes an option of **binary** variant is more desireable for easier transmission over UART for example.

Arrays and hash tables are implemented with the help of the amazing library [uthash](https://troydhanson.github.io/uthash/), which is included into this repository, but belongs to its owners.

## Supported types:

* signed and unsigned ints (8, 16, 32, 64)
* floats and doubles
* strings
* arrays
* maps (hash tables)

## Examples

1. Simple integer serialization and deserialization

```
  vartype t;
  t.typ = CMSG_U8;
  t.val.ival = 32;

  opt_cmsgbuf ser_ret = cmsg_serialize(&t);
  if (ser_ret.err != CMSG_ERR_OK) {
    printf("Oops: failed to serialize\n");
    return;
  }

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  if (par_ret.err != CMSG_ERR_OK) {
    printf("Oops: failed to deserialize\n");
    return;
  }

  opt_u8 val_ret = cmsg_vartype_to_u8(&par_ret.val);
  if (val_ret.err != CMSG_ERR_OK) {
    printf("Oops: failed to parse u8\n");
    return;
  }

  if (val_ret.val != t.val.ival) {
    printf("Oops: original and deserialized values do not match\n");
    return;
  }
```

2. Arrays

```
  opt_vartype e1 = cmsg_new_str("element 1");
  if (e1.err != CMSG_ERR_OK) {
    printf("Failed to alloc e1\n");
    return;
  }

  opt_vartype e2 = cmsg_new_str("element 2");
  // remember to check results

  opt_vartype e3 = cmsg_new_var_int8(-80);
  ...

  opt_vartype e4 = cmsg_new_str("test str");
  ...

  opt_vartype array = cmsg_new_array();
  ...

  cmsg_array_push(&array.val, &e1.val);

  cmsg_array_push(&array.val, &e2.val);

  cmsg_array_push(&array.val, &e3.val);

  cmsg_array_push(&array.val, &e4.val);

  opt_cmsgbuf ser_ret = cmsg_serialize(&array.val);
  if (ser_ret.err != CMSG_ERR_OK) {
    printf("failed to serialize data\n");
    return;
  }

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  ...

  if (par_ret.val.typ != CMSG_ARRAY) {
    printf("Error: array was expected\n");
    return;
  }

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  // Should be something like: ["element 1", "element 2", -80, "test str"]
  free(s);
```

3. Hash tables

```
  opt_vartype e1 = cmsg_new_str("element 1");
  // remember to check results

  opt_vartype e2 = cmsg_new_str("element 2");
  ...

  opt_vartype e3 = cmsg_new_var_int8(-80);
  ...

  opt_vartype e4 = cmsg_new_str("test str");
  ...

  opt_vartype root = cmsg_new_hash();
  ...

  cmsg_hash_add(&root.val, "k1", &e1.val);

  cmsg_hash_add(&root.val, "k2", &e2.val);

  cmsg_hash_add(&root.val, "num", &e3.val);

  cmsg_hash_add(&root.val, "str", &e4.val);

  opt_cmsgbuf ser_ret = cmsg_serialize(&root.val);
  ...

  opt_vartype par_ret = cmsg_parse(ser_ret.val.data, ser_ret.val.len);
  ...
  
  if (par_ret.val.typ != CMSG_HASH) {
    printf("Error: hash type was expected\n");
    return;
  }

  opt_cmsgiter hash_iter_ret = cmsg_hash_get_iter(&par_ret.val);
  ...

  while (1) {
    opt_hashdatap elt_ret = cmsg_hash_next(&hash_iter_ret.val);
    if (elt_ret.err == CMSG_ERR_OUT_OF_BOUNDS) {
      break;
    }
    if (elt_ret.err != CMSG_ERR_OK) {
        printf("Failed to get hash table element\n");
        return;
    }
  }

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  // Expected result: "{"k1": "element 1", "k2": "element 2", "num": -80, "str": "test str"}
  free(s);
```

4. JSON parsing

```
  char *text = <"JSON here">;
  int offt = 0;
  opt_vartype par_ret = cmj_parse_json(text, text_len, &offt);
  ...

  if (par_ret.err != CMSG_ERR_OK) {
    free(text);
    printf("err: %i, str: %i\n", par_ret.err, par_ret.line);
    return;
  }

  char *s = cmsg_var_print(&par_ret.val);
  printf("Decoded data: %s\n", s);
  
  free(text);
  free(s);
```

More usage examples can be found in test_* files.
