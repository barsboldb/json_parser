#include "../include/json.h"


int json_array_cmp(json_value_t *a, json_value_t *b) {
  if (!(a && b)) return -1;
  if (a->type != JSON_ARRAY || b->type != JSON_ARRAY) return -1;
  if (a->array.len != b->array.len) return -1;
  
  for (int i = 0; i < a->array.len; i++) {
    int res = json_value_cmp(&a->array.items[i], &b->array.items[i]);
    if (res != 0) {
      return res;
    }
  }

  return 0;
}

int json_object_cmp(json_value_t *a, json_value_t *b) {
  if (a->type != JSON_OBJECT || b->type != JSON_OBJECT) return -1;
  if (a->object.size != b->object.size) return -1;

  // For each entry in a, check if it exists in b with the same value
  for (size_t i = 0; i < a->object.capacity; i++) {
    hash_entry_t *entry = a->object.buckets[i];
    while (entry) {
      // Look up the same key in b
      json_value_t *b_val = hash_table_get(&b->object, entry->key, entry->key_len);
      if (!b_val) return -1;  // Key not found in b

      // Compare values
      int res = json_value_cmp(entry->value, b_val);
      if (res != 0) return res;

      entry = entry->next;
    }
  }

  return 0;
}

int json_value_cmp(json_value_t *a, json_value_t *b) {
  if (a->type != b->type) return -1;

  switch(a->type) {
    case JSON_NULL:
      return 0;
    case JSON_NUMBER:
      return a->number - b->number;
    case JSON_BOOL:
      return a->boolean - b->boolean; 
    case JSON_STRING:
      return strcmp(a->string, b->string);
    case JSON_ARRAY:
      return json_array_cmp(a, b);
    case JSON_OBJECT:
      return json_object_cmp(a, b);
  }
}

static int json_array_resize(json_value_t *val, size_t nsize) {
  if (!val || !val->array.items) return -1;

  json_value_t *temp = realloc(val->array.items, nsize * sizeof(json_value_t));
  if (!temp) return -1;

  val->array.items = temp;
  val->array.cap = nsize;
  return 0;
}

json_value_t json_value_init(json_type_t type) {
  json_value_t val;
  val.type = type;
  return val;
}

void json_value_free(json_value_t *val) {
  if (!val) return;

  if (val->type == JSON_STRING) {
    free(val->string);
    val->string = NULL;
  }
  if (val->type == JSON_ARRAY) {
    // Free nested values in array
    for (size_t i = 0; i < val->array.len; i++) {
      json_value_free(&val->array.items[i]);
    }
    free(val->array.items);
    val->array.items = NULL;
  }
  if (val->type == JSON_OBJECT) {
    // Free hash table entries (keys, values, and buckets)
    hash_table_free_entries(&val->object);
  }
  // Note: Do not free val itself, as it may be stack-allocated
}

// Initialize a hash table in-place (for embedded structs)
int hash_table_init_inplace(hash_table_t *table, size_t initial_size) {
  if (!table) return -1;

  size_t capacity = 16;
  while (capacity < initial_size) {
    capacity <<= 1;
  }

  table->buckets = calloc(capacity, sizeof(hash_entry_t *));
  if (!table->buckets) {
    return -1;
  }

  table->capacity = capacity;
  table->size = 0;
  table->load_factor = 0.75f;
  return 0;
}

// Allocate and initialize a hash table on heap
hash_table_t *hash_table_init(size_t initial_size) {
  hash_table_t *table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if (!table) return NULL;

  if (hash_table_init_inplace(table, initial_size) != 0) {
    free(table);
    return NULL;
  }

  return table;
}

// Free only entries and buckets (for embedded structs)
void hash_table_free_entries(hash_table_t *table) {
  if (!table || !table->buckets) return;

  for (size_t i = 0; i < table->capacity; i++) {
    hash_entry_t *entry = table->buckets[i];
    while (entry) {
      hash_entry_t *next = entry->next;
      free(entry->key);
      json_value_free(entry->value);  // Free the value
      free(entry->value);             // Free the value pointer
      free(entry);
      entry = next;
    }
  }

  free(table->buckets);
  table->buckets = NULL;
  table->size = 0;
  table->capacity = 0;
}

// Free entire hash table (for heap-allocated tables)
void hash_table_free(hash_table_t *table) {
  if (!table) return;
  hash_table_free_entries(table);
  free(table);
}

int hash_table_insert(hash_table_t *table, const char *key, size_t key_len, json_value_t *value) {
  if ((float)table->size / table->capacity >= table->load_factor) {
    if (hash_table_resize(table) != 0) {
      return -1;
    }
  }

  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);
  
  hash_entry_t *entry = table->buckets[index];
  while(entry) {
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      return 1;  // Duplicate key found
    }
    entry = entry->next;
  }

  hash_entry_t *new_entry = malloc(sizeof(hash_entry_t));
  if (!new_entry) return -1;

  new_entry->key = malloc(key_len + 1);
  if (!new_entry->key) {
    free(new_entry);
    return -1;
  }

  memcpy(new_entry->key, key, key_len);
  new_entry->key[key_len] = '\0';
  new_entry->key_len = key_len;
  new_entry->value = value;

  new_entry->next = table->buckets[index];
  table->buckets[index] = new_entry;
  table->size++;

  return 0;
}

void json_object_set(json_value_t *obj, char *key, json_value_t val) {
  if (obj->type != JSON_OBJECT) return;

  size_t key_len = strlen(key);

  // Check if key already exists
  json_value_t *existing = hash_table_get(&obj->object, key, key_len);
  if (existing) {
    // Update existing value
    json_value_free(existing);
    *existing = val;
    free(key);  // Free the duplicate key
    return;
  }

  // Allocate value on heap for hash table storage
  json_value_t *heap_val = malloc(sizeof(json_value_t));
  if (!heap_val) return;
  *heap_val = val;

  // Insert into hash table (key is copied by hash_table_insert)
  int result = hash_table_insert(&obj->object, key, key_len, heap_val);
  if (result != 0) {
    free(heap_val);
  }
  free(key);  // hash_table_insert copies the key
}

json_value_t *hash_table_get(hash_table_t *table, const char *key, size_t key_len) {
  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);

  hash_entry_t *entry = table->buckets[index];
  while (entry) {
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }

  return NULL;
}

int hash_table_delete(hash_table_t *table, const char *key, size_t key_len) {
  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);

  hash_entry_t *entry = table->buckets[index];
  hash_entry_t *prev = NULL;

  while (entry) {
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      // Found the entry to delete
      if (prev) {
        prev->next = entry->next;
      } else {
        table->buckets[index] = entry->next;
      }

      // Free the entry
      free(entry->key);
      json_value_free(entry->value);
      free(entry->value);
      free(entry);
      table->size--;
      return 0;
    }
    prev = entry;
    entry = entry->next;
  }

  return -1;  // Key not found
}

json_value_t json_object_get(json_value_t *obj, char *key) {
  if (obj->type != JSON_OBJECT) return json_value_init(JSON_NULL);

  size_t key_len = strlen(key);
  json_value_t *result = hash_table_get(&obj->object, key, key_len);

  if (result) {
    return *result;
  }

  return json_value_init(JSON_NULL);
}

int json_object_delete(json_value_t *obj, char *key) {
  if (obj->type != JSON_OBJECT) return -1;
  size_t key_len = strlen(key);
  return hash_table_delete(&obj->object, key, key_len);
}

size_t json_object_size(json_value_t *obj) {
  if (obj->type != JSON_OBJECT) return 0;
  return obj->object.size;
}

int json_object_has(json_value_t *obj, char *key) {
  if (obj->type != JSON_OBJECT) return 0;
  size_t key_len = strlen(key);
  return hash_table_get(&obj->object, key, key_len) != NULL;
}

/*
 * TODO: For robust object operation, following functions can be added:
 * json_object_clear(json_value_t *obj)
 * json_object_keys(json_value_t *obj)
 */

void json_array_push(json_value_t *arr, json_value_t val) {
  if ((float)arr->array.len >= (float)arr->array.cap * 0.75) {
    arr->array.cap *= 2;
    arr->array.items = realloc(arr->array.items, arr->array.cap * sizeof(json_value_t));
    if (!arr->array.items) {
      return;
    }
  }

  arr->array.items[arr->array.len++] = val;
}

int json_array_pop(json_value_t *arr) {
  if (!arr || arr->type != JSON_ARRAY || arr->array.len == 0 ||
      !arr->array.items)
    return -1;

  arr->array.len--;

  if (arr->array.len > 0 && arr->array.len < arr->array.cap / 4) {
    size_t new_cap = arr->array.cap / 2;
    if (new_cap < ARRAY_MIN_CAP) new_cap = ARRAY_MIN_CAP;
    json_array_resize(arr, new_cap);
  }

  return 0;
}

json_value_t json_value_string(char *str) {
  json_value_t val = json_value_init(JSON_STRING);
  val.string = str;
  return val;
}

json_value_t json_value_number(double number) {
  json_value_t val = json_value_init(JSON_NUMBER);
  val.number = number;
  return val;
}

json_value_t json_value_bool(bool boolean) {
  json_value_t val = json_value_init(JSON_BOOL);
  val.boolean = boolean;
  return val;
}

json_value_t json_value_array(size_t size) {
  json_value_t val = json_value_init(JSON_ARRAY);
  // Ensure minimum capacity of ARRAY_MIN_CAP
  size_t cap = size < ARRAY_MIN_CAP ? ARRAY_MIN_CAP : size;
  val.array.items = malloc(sizeof(json_value_t) * cap);
  val.array.len = 0;
  val.array.cap = cap;
  return val;
}
json_value_t json_value_object(size_t size) {
  json_value_t val = json_value_init(JSON_OBJECT);
  hash_table_init_inplace(&val.object, size);
  return val;
}
