#include "../include/json.h"

json_value_t *json_value_init(json_type_t type) {
  json_value_t *val = malloc(sizeof(json_value_t));
  val->type = type;
  return val;
}

void json_value_free(json_value_t *val) {
  if (val->type == JSON_STRING) free(val->string);
  free(val);
}

void json_array_push(json_value_t *arr, json_value_t *val) {
  if ((float)arr->array.len >= (float)arr->array.cap * 0.75) {
    arr->array.cap *= 2;
    arr->array.items = realloc(arr->array.items, arr->array.cap);
    if (!arr->array.items) {
      return;
    }
  }

  arr->array.items[arr->array.len++] = val;
}

json_value_t *json_array_pop(json_value_t *arr) {
  if ((float)--arr->array.len < (float)arr->array.cap * 0.25) {
    arr->array.cap /= 2;
    arr->array.items = realloc(arr->array.items, arr->array.cap);
    if (!arr->array.items) {
      return NULL;
    }
  }
  return arr->array.items[--arr->array.len];
}

json_value_t *json_value_string(char *str) {
  json_value_t *val = json_value_init(JSON_STRING);
  size_t len = strlen(str);
  val->string = malloc(len + 1);
  strncpy(val->string, str, len);
  val->string[len] = '\0';
  return val;
}

json_value_t *json_value_number(double number) {
  json_value_t *val = json_value_init(JSON_NUMBER);
  val->number = number;
  return val;
}

json_value_t *json_value_bool(bool boolean) {
  json_value_t *val = json_value_init(JSON_BOOL);
  val->boolean = boolean;
  return val;
}

json_value_t *json_value_array(size_t size) {
  json_value_t *val = json_value_init(JSON_ARRAY);
  val->array.items = malloc(sizeof(json_value_t *) * size);
  val->array.len = 0;
  val->array.cap = size;
  return val;
}
json_value_t *json_value_object(size_t size) {
  json_value_t *val = json_value_init(JSON_OBJECT);
  val->object.entries = malloc(sizeof(json_object_entry) * size);
  val->object.len = 0;
  val->object.cap = size;
  return val;
}
