/**
 * In JSON, values must be one of the following data types:
 * - a string
 * - a number
 * - an object (JSON object)
 * - an array
 * - a boolean
 * - null
*/

typedef enum json_data_types {
  json_number,
  json_string,
  json_null,
  json_date,
  json_object,
  json_array,
  json_boolean,
} json_value;
