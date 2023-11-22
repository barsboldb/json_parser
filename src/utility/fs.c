#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* path) {
  /**
   * It's very important that the returned string should be freed.
   * Unless it cause memory leak. Because the
   * `char* read_file(const char* path)` function uses memory
   * allocation / `void *malloc(size_t __size)` /.
   */

  // Open file with read permition
  FILE* json_file = fopen(path, "r");

  // Move file pointer until the end
  fseek(json_file, 0, SEEK_END);
  // Get file pointer position
  size_t file_size = ftell(json_file);
  fseek(json_file, 0, SEEK_SET);
  // Allocate buffer on heap memory
  char* content = malloc(file_size + 1);

  // Read all content of file
  fread(content, file_size, 1, json_file);

  // Close file to prevent memory leak
  fclose(json_file);

  return content;
}
