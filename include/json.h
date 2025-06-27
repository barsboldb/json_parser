#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
} json_type_t;

typedef struct json_value json_value_t;

typedef struct {
  char *key;
  json_value_t *value;
} json_object_entry;

struct json_value {
  json_type_t type;
  union {
    double number;
    char *string;
    bool boolean;
    struct {
      json_value_t **items;
      size_t length;
    } array;
    struct {
      json_object_entry *entries;
      size_t length;
    } object;
  };
};

// Create new json_value_t
json_value_t *json_value_init(json_type_t);

// Free json_value_t and all nested content
void json_value_free(json_value_t *);

// Create json_value_t
json_value_t *json_value_bool(bool);
json_value_t *json_value_number(double);
json_value_t *json_value_string(char *);
json_value_t *json_value_array(size_t);
