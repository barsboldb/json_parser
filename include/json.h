#ifdef BENCHMARK_MEMORY_TRACKING
#include "../benchmarks/include/mem_track.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/**
 * In JSON, values must be one of the following data types:
 * - a string
 * - a number
 * - an object (JSON object)
 * - an array
 * - a boolean
 * - null
*/

#define ARRAY_MIN_CAP 4

typedef enum {
  JSON_NULL,
  JSON_BOOL,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
} json_type_t;

typedef struct json_value json_value_t;
typedef struct json_object_entry json_object_entry;

struct json_value {
  json_type_t type;
  union {
    double number;
    char *string;
    bool boolean;
    struct {
      json_value_t *items;
      size_t len;
      size_t cap;
    } array;
    struct {
      json_object_entry *entries;
      size_t len;
      size_t cap;
    } object;
  };
};

struct json_object_entry {
  char *key;
  json_value_t value;
};

// Create new json_value_t
json_value_t json_value_init(json_type_t);

int json_value_cmp(json_value_t *a, json_value_t *b);

// Free json_value_t and all nested content
void json_value_free(json_value_t *);

// Create json_value_t
json_value_t json_value_bool(bool);
json_value_t json_value_number(double);
json_value_t json_value_string(char *);
json_value_t json_value_array(size_t);
json_value_t json_value_object(size_t size);

int json_array_cmp(json_value_t *a, json_value_t *b);

int json_object_cmp(json_value_t *a, json_value_t *b);

void json_object_set(json_value_t *, char *, json_value_t);
json_value_t json_object_get(json_value_t *, char *);
int json_object_delete(json_value_t *, char*);

// Handle json_value_array push and pop
void json_array_push(json_value_t *, json_value_t);
int json_array_pop(json_value_t *);
