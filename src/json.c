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
  // TODO: Current implementation only checks ordered object. It should be
  // ehanced to work with unordered objects; 
  if (a->type != JSON_OBJECT || b->type != JSON_OBJECT) return -1;
  if (a->object.len != b->object.len) return -1;

  for (int i = 0; i < a->object.len; i++) {
    int str_res = strcmp(a->object.entries[i].key, b->object.entries[i].key);
    int res = json_value_cmp(a->object.entries[i].value, b->object.entries[i].value);
    if (str_res != 0 || res != 0)
      return res;
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
  if (val->type == JSON_STRING) free(val->string);
  if (val->type == JSON_ARRAY) free(val->array.items);
  // Note: Do not free val itself, as it may be stack-allocated
}

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
    json_array_resize(arr->array.items, new_cap);
  }

  return 0;
}

json_value_t json_value_string(char *str) {
  json_value_t val = json_value_init(JSON_STRING);
  size_t len = strlen(str);
  val.string = malloc(len + 1);
  strncpy(val.string, str, len);
  val.string[len] = '\0';
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
  val.object.entries = malloc(sizeof(json_object_entry) * size);
  val.object.len = 0;
  val.object.cap = size;
  return val;
}
