#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "cerial.h"

static const char* cerial_read_json_value(cerial_accessor accessor, void *output, const char *value_start, const char *end);

size_t cerial_read_json(cerial *self, void *output, const char *string, size_t size)
{
  const char *end = string + size;
  const char *head = memchr(string, '{', size);
  if (!head) return 0;
  const char *value_end;

  while(1) {
    const char *key_start = memchr(head, '"', end-head);
    if (!key_start) return 0;
    key_start++;

    const char *key_end = memchr(key_start, '"', end-key_start);
    if (!key_end) return 0;
    key_end++;

    const char *value_start = memchr(key_end, ':', end-key_end);
    if (!value_start) return 0;
    value_start++;

    value_end = NULL;
    for (int i=0; i<self->count; i++) {
      cerial_accessor accessor = self->accessors[i];
      size_t key_size = key_end - key_start - 1;
      size_t accessor_size = strlen(accessor.name);
      if (key_size == accessor_size && strncmp(accessor.name, key_start, accessor_size) == 0) {
        if (accessor.is_array) {
          const char *array_start = memchr(value_start, '[', end-value_start);
          if (!array_start) return 0;
          array_start++;
          char *output_start = (char*)output;
          size_t output_increment = cerial_accessor_size(accessor);
          for(int array_size=0;;array_size++) {
            if (array_size > accessor.buffer_size) return 0;
            value_end = cerial_read_json_value(accessor, output_start, array_start, end);
            if (!value_end) return 0;
            array_start = memchr(value_end, ',', end-value_end);
            if (!array_start) {
              array_start = memchr(value_end, ']', end-value_end);
              if (array_start) break;
              return 0;
            }
            array_start++;
            output_start += output_increment;
          }
          value_end = array_start + 1;
        }
        else {
          value_end = cerial_read_json_value(accessor, output, value_start, end);
        }
        if (!value_end) return 0;
      }
    }
    if (!value_end) return 0;

    const char *next_key = memchr(value_end, ',', end-value_end);
    const char *next_object = memchr(value_end, '}', end-value_end);
    if (next_key && next_object && next_object < next_key) break;
    if (!next_key) break;
    head = next_key+1;
  }

  const char *tail = memchr(value_end, '}', end-value_end);
  if (!tail) return 0;
  tail++;

  while (isspace(*tail) && tail < end) tail++;

  return tail - string;
}

static const char* cerial_read_json_value(cerial_accessor accessor, void *output, const char *value_start, const char *end)
{
  const char *value_end = NULL;
  if (accessor.type == cerial_int) {
    char *head = NULL;
    *(int*)((char*)output + accessor.offset) = strtol(value_start, &head, 0);
    if (!head) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_float) {
    char *head = NULL;
    *(int*)((char*)output + accessor.offset) = strtof(value_start, &head);
    if (!head) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_double) {
    char *head = NULL;
    *(int*)((char*)output + accessor.offset) = strtod(value_start, &head);
    if (!head) return 0;
    value_end = head;
  }
  else if (accessor.type == cerial_str) {
    value_start = memchr(value_start, '"', end-value_start);
    if (!value_start) return 0;
    value_start++;
    value_end = memchr(value_start, '"', end-value_start);
    if (!value_end) return 0;
    size_t size_to_copy = (value_end - value_start) < accessor.buffer_size ? (value_end - value_start) : accessor.buffer_size;
    strncpy((char*)output + accessor.offset, value_start, size_to_copy);
    value_end++;
  }
  else if (accessor.type == cerial_bool) {
    while (isspace(*value_start) && value_start < end) value_start++;
    if (value_start >= end) return 0;
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
    size_t super_bytes = cerial_read_json(accessor.super_cerial, (void*)((char*)output + accessor.offset), value_start, end-value_start);
    if (!super_bytes) return 0;
    value_end = value_start + super_bytes;
  }

  return value_end;
}

size_t cerial_accessor_size(cerial_accessor accessor)
{
  if (accessor.type == cerial_int) {
    return sizeof(int);
  }
  else if (accessor.type == cerial_float) {
    return sizeof(float);
  }
  else if (accessor.type == cerial_double) {
    return sizeof(double);
  }
  else if (accessor.type == cerial_str) {
    return accessor.buffer_size;
  }
  else if (accessor.type == cerial_bool) {
    return sizeof(bool);
  }
  else if (accessor.type == cerial_object) {
    return accessor.super_cerial->size;
  }
  return 0;
}