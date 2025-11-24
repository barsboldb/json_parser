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

// Forward declarations - only for pointers
typedef struct json_value json_value_t;
typedef struct hash_entry hash_entry_t;
typedef struct hash_bucket hash_bucket_t;

// hash_bucket only contains pointer to entries, so can be defined now
struct hash_bucket {
  hash_entry_t *items;  // pointer is OK with incomplete type
  size_t len;
  size_t cap;
};

typedef struct {
  hash_bucket_t *buckets;
  size_t capacity;
  size_t size;
} hash_table_t;

// Now json_value is complete
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
    hash_table_t object;
  };
};

// Now hash_entry can use json_value_t by value
struct hash_entry {
  char *key;
  size_t key_len;
  json_value_t value;  // stored by value
};


static inline uint32_t hash_string(const char *str, size_t len) {
  uint32_t hash = 2166136261u;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619u;
  }

  return hash;
}

hash_table_t *hash_table_init(size_t);
int hash_table_init_inplace(hash_table_t *table, size_t initial_size);
void hash_table_free(hash_table_t *);
void hash_table_free_entries(hash_table_t *);
int hash_table_insert(hash_table_t *, const char *, size_t, json_value_t);
int hash_table_delete(hash_table_t *, const char *, size_t);
json_value_t *hash_table_get(hash_table_t *, const char *, size_t);

// Resize hash table when load factor exceeded
static int hash_table_resize(hash_table_t *table) {
  size_t new_capacity = table->capacity * 2;
  hash_bucket_t *new_buckets = calloc(new_capacity, sizeof(hash_bucket_t));
  if (!new_buckets) return -1;

  // Rehash all entries from old buckets to new buckets
  for (size_t i = 0; i < table->capacity; i++) {
    hash_bucket_t *bucket = &table->buckets[i];
    for (size_t j = 0; j < bucket->len; j++) {
      hash_entry_t *entry = &bucket->items[j];

      // Calculate new bucket index
      uint32_t hash = hash_string(entry->key, entry->key_len);
      size_t new_index = hash & (new_capacity - 1);

      // Insert into new bucket (grow array if needed)
      hash_bucket_t *new_bucket = &new_buckets[new_index];
      if (new_bucket->len >= new_bucket->cap) {
        size_t new_cap = new_bucket->cap == 0 ? 2 : new_bucket->cap * 2;
        hash_entry_t *new_items = realloc(new_bucket->items, new_cap * sizeof(hash_entry_t));
        if (!new_items) {
          // Cleanup on failure
          for (size_t k = 0; k < new_capacity; k++) {
            free(new_buckets[k].items);
          }
          free(new_buckets);
          return -1;
        }
        new_bucket->items = new_items;
        new_bucket->cap = new_cap;
      }
      new_bucket->items[new_bucket->len++] = *entry;  // copy entry (key pointer + value)
    }
    // Free old bucket's items array (keys are now owned by new buckets)
    free(bucket->items);
  }

  free(table->buckets);
  table->buckets = new_buckets;
  table->capacity = new_capacity;

  return 0;
}

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
int json_object_delete(json_value_t *, char *);
size_t json_object_size(json_value_t *);
int json_object_has(json_value_t *, char *);

// Handle json_value_array push and pop
void json_array_push(json_value_t *, json_value_t);
int json_array_pop(json_value_t *);
