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
    hash_bucket_t *bucket = &a->object.buckets[i];
    for (size_t j = 0; j < bucket->len; j++) {
      hash_entry_t *entry = &bucket->items[j];
      // Look up the same key in b
      json_value_t *b_val = hash_table_get(&b->object, entry->key, entry->key_len);
      if (!b_val) return -1;  // Key not found in b

      // Compare values
      int res = json_value_cmp(&entry->value, b_val);
      if (res != 0) return res;
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

static int json_array_resize(json_value_t *val, size_t nsize, mem_pool_t *pool) {
  if (!val || !val->array.items) return -1;

  json_value_t *temp = pool_alloc(pool, nsize * sizeof(json_value_t));
  if (!temp) return -1;

  memcpy(temp, val->array.items, val->array.len * sizeof(json_value_t));
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
int hash_table_init_inplace(hash_table_t *table, size_t initial_size, mem_pool_t *pool) {
  if (!table) return -1;

  size_t capacity = 16;
  while (capacity < initial_size) {
    capacity <<= 1;
  }

  table->buckets = pool_alloc(pool, capacity * sizeof(hash_bucket_t));
  if (!table->buckets) {
    return -1;
  }
  memset(table->buckets, 0, capacity * sizeof(hash_bucket_t));

  table->capacity = capacity;
  table->size = 0;
  return 0;
}

// Allocate and initialize a hash table on heap
// Note: This function is deprecated and not used with memory pools
hash_table_t *hash_table_init(size_t initial_size) {
  hash_table_t *table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if (!table) return NULL;

  // Create a temporary pool for this table (not recommended)
  mem_pool_t *pool = pool_create();
  if (!pool) {
    free(table);
    return NULL;
  }

  if (hash_table_init_inplace(table, initial_size, pool) != 0) {
    pool_destroy(pool);
    free(table);
    return NULL;
  }

  return table;
}

// Free only entries and buckets (for embedded structs)
void hash_table_free_entries(hash_table_t *table) {
  if (!table || !table->buckets) return;

  for (size_t i = 0; i < table->capacity; i++) {
    hash_bucket_t *bucket = &table->buckets[i];
    for (size_t j = 0; j < bucket->len; j++) {
      hash_entry_t *entry = &bucket->items[j];
      free(entry->key);
      json_value_free(&entry->value);  // Free nested content (value is embedded)
    }
    free(bucket->items);  // Free the bucket's items array
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

#define HASH_TABLE_LOAD_FACTOR 0.75f

// Resize hash table when load factor exceeded
static int hash_table_resize(hash_table_t *table, mem_pool_t *pool) {
  size_t new_capacity = table->capacity * 2;
  hash_bucket_t *new_buckets = pool_alloc(pool, new_capacity * sizeof(hash_bucket_t));
  if (!new_buckets) return -1;
  memset(new_buckets, 0, new_capacity * sizeof(hash_bucket_t));

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
        hash_entry_t *new_items = pool_alloc(pool, new_cap * sizeof(hash_entry_t));
        if (!new_items) {
          return -1;
        }
        if (new_bucket->items) {
          memcpy(new_items, new_bucket->items, new_bucket->len * sizeof(hash_entry_t));
        }
        new_bucket->items = new_items;
        new_bucket->cap = new_cap;
      }
      new_bucket->items[new_bucket->len++] = *entry;  // copy entry (key pointer + value)
    }
  }

  table->buckets = new_buckets;
  table->capacity = new_capacity;

  return 0;
}

int hash_table_insert(hash_table_t *table, const char *key, size_t key_len, json_value_t value, mem_pool_t *pool) {
  if ((float)table->size / table->capacity >= HASH_TABLE_LOAD_FACTOR) {
    if (hash_table_resize(table, pool) != 0) {
      return -1;
    }
  }

  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);

  hash_bucket_t *bucket = &table->buckets[index];

  // Check for duplicate key
  for (size_t i = 0; i < bucket->len; i++) {
    if (bucket->items[i].key_len == key_len &&
        memcmp(bucket->items[i].key, key, key_len) == 0) {
      return 1;  // Duplicate key found
    }
  }

  // Grow bucket array if needed
  if (bucket->len >= bucket->cap) {
    size_t new_cap = bucket->cap == 0 ? 2 : bucket->cap * 2;
    hash_entry_t *new_items = pool_alloc(pool, new_cap * sizeof(hash_entry_t));
    if (!new_items) return -1;
    if (bucket->items) {
      memcpy(new_items, bucket->items, bucket->len * sizeof(hash_entry_t));
    }
    bucket->items = new_items;
    bucket->cap = new_cap;
  }

  // Allocate and copy key
  char *key_copy = pool_alloc(pool, key_len + 1);
  if (!key_copy) return -1;
  memcpy(key_copy, key, key_len);
  key_copy[key_len] = '\0';

  // Add entry to bucket
  hash_entry_t *entry = &bucket->items[bucket->len++];
  entry->key = key_copy;
  entry->key_len = key_len;
  entry->value = value;  // copy by value

  table->size++;
  return 0;
}

// Pooled version (for parser use)
void json_object_set_pooled(json_value_t *obj, char *key, json_value_t val, mem_pool_t *pool) {
  if (obj->type != JSON_OBJECT) return;

  size_t key_len = strlen(key);

  // Check if key already exists
  json_value_t *existing = hash_table_get(&obj->object, key, key_len);
  if (existing) {
    // Update existing value
    json_value_free(existing);
    *existing = val;
    return;
  }

  // Insert into hash table (key is copied, value is copied by value)
  hash_table_insert(&obj->object, key, key_len, val, pool);
}

// Public API version (uses malloc)
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

  // Create a temporary pool for this operation (not ideal, but maintains API)
  mem_pool_t *temp_pool = pool_create();
  if (!temp_pool) return;

  // Insert into hash table (key is copied, value is copied by value)
  hash_table_insert(&obj->object, key, key_len, val, temp_pool);
  free(key);  // hash_table_insert copies the key
  // Note: We don't destroy the pool as the hash table now owns the allocated memory
}

json_value_t *hash_table_get(hash_table_t *table, const char *key, size_t key_len) {
  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);

  hash_bucket_t *bucket = &table->buckets[index];
  for (size_t i = 0; i < bucket->len; i++) {
    hash_entry_t *entry = &bucket->items[i];
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      return &entry->value;  // return pointer to embedded value
    }
  }

  return NULL;
}

int hash_table_delete(hash_table_t *table, const char *key, size_t key_len) {
  uint32_t hash = hash_string(key, key_len);
  size_t index = hash & (table->capacity - 1);

  hash_bucket_t *bucket = &table->buckets[index];

  for (size_t i = 0; i < bucket->len; i++) {
    hash_entry_t *entry = &bucket->items[i];
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      // Free the entry's data
      free(entry->key);
      json_value_free(&entry->value);

      // Move last entry to this position (swap-remove)
      if (i < bucket->len - 1) {
        bucket->items[i] = bucket->items[bucket->len - 1];
      }
      bucket->len--;
      table->size--;
      return 0;
    }
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

// Pooled version (for parser use)
void json_array_push_pooled(json_value_t *arr, json_value_t val, mem_pool_t *pool) {
  if ((float)arr->array.len >= (float)arr->array.cap * 0.75) {
    size_t new_cap = arr->array.cap * 2;
    json_value_t *new_items = pool_alloc(pool, new_cap * sizeof(json_value_t));
    if (!new_items) {
      return;
    }
    memcpy(new_items, arr->array.items, arr->array.len * sizeof(json_value_t));
    arr->array.items = new_items;
    arr->array.cap = new_cap;
  }

  arr->array.items[arr->array.len++] = val;
}

// Public API version (uses realloc)
void json_array_push(json_value_t *arr, json_value_t val) {
  if ((float)arr->array.len >= (float)arr->array.cap * 0.75) {
    size_t new_cap = arr->array.cap * 2;
    json_value_t *new_items = realloc(arr->array.items, new_cap * sizeof(json_value_t));
    if (!new_items) {
      return;
    }
    arr->array.items = new_items;
    arr->array.cap = new_cap;
  }

  arr->array.items[arr->array.len++] = val;
}

int json_array_pop(json_value_t *arr) {
  if (!arr || arr->type != JSON_ARRAY || arr->array.len == 0 ||
      !arr->array.items)
    return -1;

  arr->array.len--;

  // Note: We don't resize down when using memory pool since
  // the pool doesn't support deallocation

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

// Pooled versions (for parser use)
json_value_t json_value_array_pooled(size_t size, mem_pool_t *pool) {
  json_value_t val = json_value_init(JSON_ARRAY);
  // Ensure minimum capacity of ARRAY_MIN_CAP
  size_t cap = size < ARRAY_MIN_CAP ? ARRAY_MIN_CAP : size;
  val.array.items = pool_alloc(pool, sizeof(json_value_t) * cap);
  val.array.len = 0;
  val.array.cap = cap;
  return val;
}

json_value_t json_value_object_pooled(size_t size, mem_pool_t *pool) {
  json_value_t val = json_value_init(JSON_OBJECT);
  hash_table_init_inplace(&val.object, size, pool);
  return val;
}

// Public API versions (use malloc/calloc)
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

  // Create a temporary pool for the hash table (not ideal, but maintains API)
  mem_pool_t *temp_pool = pool_create();
  if (!temp_pool) return val;

  hash_table_init_inplace(&val.object, size, temp_pool);
  // Note: We don't destroy the pool as the hash table now owns the allocated memory
  return val;
}
