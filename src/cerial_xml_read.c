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
  const char *open_start;
  const char *open_end;
  const char *close_start;
  const char *close_end;
} cerial_xml_tag;

size_t cerial_xml_read_object(cerial *self, void *output, const char *string, size_t size, bool read_root);
static bool cerial_xml_read_value(cerial_accessor accessor, void *output, const char *value_start, const char *value_end);
static cerial_xml_tag cerial_xml_read_tag(const char *head, const char *end);


size_t cerial_xml_read(cerial *self, void *output, const char *string, size_t size)
{
  return cerial_xml_read_object(self, output, string, size, true);
}

size_t cerial_xml_read_object(cerial *self, void *output, const char *string, size_t size, bool read_root)
{
  const char *end = string + size;
  const char *head = string;
  if (!cerial_assert(head)) return 0;

  while (1) {
    cerial_xml_tag tag = cerial_xml_read_tag(head, end);
    if (!cerial_assert(tag.open_start)) return 0;

    size_t key_size = tag.open_end - tag.open_start - 1;
    const char *value_start = tag.open_end+1;
    const char *value_end = tag.close_start;

    if (read_root && !(self->options & cerial_option_no_root)) {
      size_t name_size = strlen(self->name);
      if (!cerial_assert(key_size == name_size)) return 0;
      if (!cerial_assert(strncmp(self->name, tag.open_start+1, name_size) == 0)) return 0;
      size_t bytes = cerial_xml_read_object(self, output, value_start, value_end - value_start, false);
      if (!cerial_assert(bytes == value_end - value_start)) return 0;
    }
    else for (int i=0; i<self->count; i++) {
      cerial_accessor accessor = self->accessors[i];
      size_t accessor_size = strlen(accessor.name);
      if (key_size == accessor_size && strncmp(accessor.name, tag.open_start+1, accessor_size) == 0) {
        if (accessor.is_array) {
          char *array_output = (char*)output;
          size_t array_output_increment = cerial_accessor_size(accessor);

          while (value_start < value_end) {
            cerial_xml_tag value_tag = cerial_xml_read_tag(value_start, value_end);
            if (!cerial_assert(value_tag.open_start)) return 0;
            size_t value_key_size = value_tag.open_end - value_tag.open_start - 1;
            if (!cerial_assert(5 == value_key_size)) return 0;
            if (!cerial_assert(strncmp("value", value_tag.open_start+1, 5) == 0)) return 0;
            const char *array_value_start = value_tag.open_end+1;
            const char *array_value_end = value_tag.close_start;
            if (!cerial_assert(cerial_xml_read_value(accessor, array_output, array_value_start, array_value_end))) return 0;
            value_start = value_tag.close_end + 1;
            while (isspace(*value_start) && value_start < value_end) value_start++;
            array_output += array_output_increment;
          }
        }
        else {
          if (!cerial_assert(cerial_xml_read_value(accessor, output, value_start, value_end))) return 0;
        }
      }
    }
    head = tag.close_end + 1;

    while (isspace(*head) && head < end) head++;
    if (head == end) break;
  }

  return head - string;
}

static cerial_xml_tag cerial_xml_read_tag(const char *head, const char *end)
{
  cerial_xml_tag tag = {0};
  if (!cerial_assert(head < end)) return (cerial_xml_tag){0};
  tag.open_start = memchr(head, '<', end - head);
  if (!cerial_assert(tag.open_start)) return (cerial_xml_tag){0};
  tag.open_end = memchr(tag.open_start, '>', end - tag.open_start);
  if (!cerial_assert(tag.open_end)) return (cerial_xml_tag){0};

  size_t key_size = tag.open_end - tag.open_start - 1;
  tag.close_start = tag.open_end;
  while (tag.close_start < end) {
    tag.close_start = memchr(tag.close_start, '<', end - tag.close_start);
    if (!cerial_assert(tag.close_start - end >= key_size)) return (cerial_xml_tag){0};
    if (tag.close_start[1] == '/' && memcmp(tag.open_start+1, tag.close_start+2, key_size) == 0) {
      tag.close_end = memchr(tag.close_start, '>', end - tag.close_start);
      if (!cerial_assert(tag.close_end)) return (cerial_xml_tag){0};
      break;
    }
    tag.close_start++;
  }
  if (!cerial_assert(tag.close_end)) return (cerial_xml_tag){0};
  return tag;
}

static bool cerial_xml_read_value(cerial_accessor accessor, void *output, const char *value_start, const char *value_end)
{
  if (accessor.type == cerial_int) {
    char *int_end = NULL;
    *(int*)((char*)output + accessor.offset) = strtol(value_start, &int_end, 0);
    if (!cerial_assert(int_end)) return 0;
    if (!cerial_assert(int_end <= value_end)) return 0;
  }
  else if (accessor.type == cerial_float) {
    char *int_end = NULL;
    *(int*)((char*)output + accessor.offset) = strtof(value_start, &int_end);
    if (!cerial_assert(int_end)) return 0;
    if (!cerial_assert(int_end <= value_end)) return 0;
  }
  else if (accessor.type == cerial_double) {
    char *int_end = NULL;
    *(int*)((char*)output + accessor.offset) = strtod(value_start, &int_end);
    if (!cerial_assert(int_end)) return 0;
    if (!cerial_assert(int_end <= value_end)) return 0;
  }
  else if (accessor.type == cerial_str) {
    size_t size_to_copy = (value_end - value_start) < accessor.buffer_size ? (value_end - value_start) : accessor.buffer_size;
    strncpy((char*)output + accessor.offset, value_start, size_to_copy);
    if (size_to_copy < accessor.buffer_size) *((char*)output + accessor.offset + size_to_copy) = '\0';
  }
  else if (accessor.type == cerial_bool) {
    size_t value_size = value_end - value_start;
    if (value_size >= 4 && strncmp(value_start, "true", 4) == 0) {
      *(bool*)((char*)output + accessor.offset) = true;
    }
    else if (value_size >= 5 && strncmp(value_start, "false", 5) == 0) {
      *(bool*)((char*)output + accessor.offset) = false;
    }
    else {
      cerial_assert(false);
      return 0;
    }
  }
  else if (accessor.type == cerial_object) {
    size_t super_bytes = cerial_xml_read_object(accessor.super_cerial, (void*)((char*)output + accessor.offset), value_start, value_end - value_start, true);
    if (!cerial_assert(super_bytes)) return 0;
    if (!cerial_assert(super_bytes <= value_end - value_start)) return 0;
  }
  return true;
}
