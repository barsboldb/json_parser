#include <stdlib.h>
/**
 * In JSON, values must be one of the following data types:
 * - a string
 * - a number
 * - an object (JSON object)
 * - an array
 * - a boolean
 * - null
*/

typedef enum {
  JSON_NULL,
  JSON_BOOL,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
} json_type;

typedef struct json_value json_value;

typedef struct {
  char *key;
  json_value *value;
} json_object_entry;

struct json_value {
  json_type type;
  union {
    double number;
    char *string;
    int boolean;
    struct {
      json_value **items;
      size_t length;
    } array;
    struct {
      json_object_entry *entries;
      size_t length;
    } object;
  };
};
