#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "cerial.h"

// #define DEBUG
#include "cerial_internal.h"

typedef struct {
  char *start;
  char *end;
  char *value;
} cerial_json_key;

static const char* cerial_json_read_value(cerial_accessor accessor, void *output, const char *value_start, const char *end);
static size_t cerial_json_read_object(cerial *self, void *output, const char *string, size_t size, bool read_root);
static const char* cerial_json_read_root(cerial *self, void *output, cerial_json_key key, const char *head, const char *end);
static const char* cerial_json_read_array(cerial_accessor accessor, void *output, const char *value_start, const char *end);
static cerial_json_key cerial_json_read_key(const char *head, const char *end);

size_t cerial_json_read(cerial *self, void *output, const char *string, size_t size)
{
  return cerial_json_read_object(self, output, string, size, true);
}

static size_t cerial_json_read_object(cerial *self, void *output, const char *string, size_t size, bool read_root)
{
  const char *end = string + size;
  const char *head = memchr(string, '{', size);
  if (!cerial_assert(head)) return 0;
  const char *value_end = NULL;

  while(1) {
    if (!cerial_assert(head < end)) return 0;
    cerial_json_key key = cerial_json_read_key(head, end);
    if (!cerial_assert(key.start)) return 0;
    if (!cerial_assert(key.end))   return 0;
    if (!cerial_assert(key.value)) return 0;

    if (read_root && !(self->options & cerial_option_no_root)) {
      value_end = cerial_json_read_root(self, output, key, head, end);
    }
    else for (int i=0; i<self->count; i++) {
      cerial_accessor accessor = self->accessors[i];
      size_t key_size = key.end - key.start - 1;
      size_t accessor_size = strlen(accessor.name);
      if (key_size == accessor_size && strncmp(accessor.name, key.start, accessor_size) == 0) {
        if (accessor.is_array) {
          value_end = cerial_json_read_array(accessor, output, key.value, end);
        }
        else {
          value_end = cerial_json_read_value(accessor, output, key.value, end);
        }
      }
    }
    if (!cerial_assert(value_end)) return 0;

    const char *next_key = memchr(value_end, ',', end-value_end);
    const char *next_object = memchr(value_end, '}', end-value_end);
    if (next_key && next_object && next_object < next_key) break;
    if (!next_key) break;
    head = next_key+1;
  }

  const char *tail = memchr(value_end, '}', end-value_end);
  if (!cerial_assert(tail)) return 0;
  tail++;

  while (isspace(*tail) && tail < end) tail++;

  return tail - string;
}

static const char* cerial_json_read_root(cerial *self, void *output, cerial_json_key key, const char *head, const char *end)
{
  size_t key_size = key.end - key.start - 1;
  size_t root_size = strlen(self->name);
  if (!cerial_assert(key_size == root_size)) return 0;
  if (!cerial_assert(strncmp(self->name, key.start, root_size) == 0)) return 0;
  size_t bytes = cerial_json_read_object(self, output, key.value, end-key.value, false);
  if (!cerial_assert(bytes)) return 0;
  return key.value + bytes;
}

static cerial_json_key cerial_json_read_key(const char *head, const char *end)
{
  cerial_json_key key = {0};

  key.start = memchr(head, '"', end - head);
  if (!cerial_assert(key.start)) return (cerial_json_key){0};
  key.start++;

  key.end = memchr(key.start, '"', end-key.start);
  if (!cerial_assert(key.end)) return (cerial_json_key){0};
  key.end++;

  key.value = memchr(key.end, ':', end-key.end);
  if (!cerial_assert(key.value)) return (cerial_json_key){0};
  key.value++;

  return key;
}

static const char* cerial_json_read_array(cerial_accessor accessor, void *output, const char *value_start, const char *end)
{
  const char *value_end = NULL;
  const char *array_start = memchr(value_start, '[', end-value_start);
  if (!cerial_assert(array_start)) return 0;
  array_start++;
  char *output_start = (char*)output;
  size_t output_increment = cerial_accessor_size(accessor);
  for(int array_size=0;;array_size++) {
    if (!cerial_assert(array_size <= accessor.buffer_size)) return 0;
    value_end = cerial_json_read_value(accessor, output_start, array_start, end);
    if (!cerial_assert(value_end)) return 0;
    array_start = memchr(value_end, ',', end-value_end);
    if (!array_start) {
      array_start = memchr(value_end, ']', end-value_end);
      if (!cerial_assert(array_start)) return 0;
      break;
    }
    array_start++;
    output_start += output_increment;
  }
  return array_start + 1;
}

static const char* cerial_json_read_value(cerial_accessor accessor, void *output, const char *value_start, const char *end)
{
  const char *value_end = NULL;
  if (accessor.type == cerial_int) {
    char *head = NULL;
    *(int*)((char*)output + accessor.offset) = strtol(value_start, &head, 0);
    if (!cerial_assert(head)) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_float) {
    char *head = NULL;
    *(float*)((char*)output + accessor.offset) = strtof(value_start, &head);
    if (!cerial_assert(head)) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_double) {
    char *head = NULL;
    *(double*)((char*)output + accessor.offset) = strtod(value_start, &head);
    if (!cerial_assert(head)) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_str) {
    value_start = memchr(value_start, '"', end-value_start);
    if (!cerial_assert(value_start)) return 0;
    value_start++;
    value_end = memchr(value_start, '"', end-value_start);
    if (!cerial_assert(value_end)) return 0;
    size_t size_to_copy = (value_end - value_start) < accessor.buffer_size ? (value_end - value_start) : accessor.buffer_size;
    strncpy((char*)output + accessor.offset, value_start, size_to_copy);
    if (size_to_copy < accessor.buffer_size) *((char*)output + accessor.offset + size_to_copy) = '\0';
    value_end++;
  }
  else if (accessor.type == cerial_bool) {
    while (isspace(*value_start) && value_start < end) value_start++;
    if (!cerial_assert(value_start < end)) return 0;
    if (strncmp(value_start, "true", 4 < end-value_start ? 4 : end-value_start) == 0) {
      *(bool*)((char*)output + accessor.offset) = true;
      value_end = value_start + 4;
    }
    else if (strncmp(value_start, "false", 5 < end-value_start ? 5 : end-value_start) == 0) {
      *(bool*)((char*)output + accessor.offset) = false;
      value_end = value_start + 5;
    }
    else {
      return 0;
    }
  }
  else if (accessor.type == cerial_object) {
    size_t super_bytes = cerial_json_read_object(accessor.super_cerial, (void*)((char*)output + accessor.offset), value_start, end-value_start, true);
    if (!cerial_assert(super_bytes)) return 0;
    value_end = value_start + super_bytes;
  }

  return value_end;
}
