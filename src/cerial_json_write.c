#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "cerial.h"

// #define DEBUG
#include "cerial_internal.h"

static char* cerial_write_json_value(cerial_accessor accessor, const void *object, char *head, const char *end);
static char* cerial_write_json_array(cerial_accessor accessor, const void *object, char *head, const char *end);

size_t cerial_write_json(cerial *self, const void *object, char *buffer, size_t size) {
  char *head = buffer;
  const char *end = head + size;

  if (!(self->options & cerial_option_no_root)) {
    size_t bytes_left = end - head;
    size_t bytes_written = snprintf(head, end - head, "{\"%s\": ", self->name);
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }

  if (end - head < 2) return 0;
  head += snprintf(head, end - head, "{");

  for (int i=0; i<self->count; i++) {
    cerial_accessor accessor = self->accessors[i];
    if (!cerial_assert(end - head >= strlen(accessor.name) + 5)) return 0;
    head += snprintf(head, end - head, "\"%s\": ", accessor.name);

    if (accessor.is_array) {
      head = cerial_write_json_array(accessor, object, head, end);
    }
    else {
      head = cerial_write_json_value(accessor, object, head, end);
    }
    if (!cerial_assert(head)) return 0;

    if (i < self->count - 1) {
      if (!cerial_assert(end - head >= 3)) return 0;
      head += snprintf(head, end - head, ", ");
    }
  }

  if (!cerial_assert(end - head >= 2)) return 0;
  head += snprintf(head, end - head, "}");

  if (!(self->options & cerial_option_no_root)) {
    if (!cerial_assert(end - head >= 2)) return 0;
    head += snprintf(head, end - head, "}");
  }

  return head - buffer;
}

static char* cerial_write_json_array(cerial_accessor accessor, const void *object, char *head, const char *end)
{
  if (!cerial_assert(end - head >= 2)) return 0;
  head += snprintf(head, end - head, "[");
  char *object_head = (char*)object;
  size_t object_increment = cerial_accessor_size(accessor);

  for (int j=0; j<accessor.buffer_size; j++) {
    head = cerial_write_json_value(accessor, object_head, head, end);
    if (!cerial_assert(head)) return 0;
    object_head += object_increment;
    if (j < accessor.buffer_size - 1) {
      if (!cerial_assert(end - head >= 3)) return 0;
      head += snprintf(head, end - head, ", ");
    }
  }

  if (!cerial_assert(end - head >= 2)) return 0;
  head += snprintf(head, end - head, "]");
  return head;
}

static char* cerial_write_json_value(cerial_accessor accessor, const void *object, char *head, const char *end) {
  if (accessor.type == cerial_int) {
    size_t bytes_left = end - head;
    size_t bytes_written = snprintf(head, bytes_left, "%d", *(int*)((char*)object + accessor.offset));
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_float) {
    size_t bytes_left = end - head;
    size_t bytes_written = snprintf(head, bytes_left, "%f", *(float*)((char*)object + accessor.offset));
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_double) {
    size_t bytes_left = end - head;
    size_t bytes_written = snprintf(head, bytes_left, "%f", *(double*)((char*)object + accessor.offset));
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_str) {
    size_t bytes_left = end - head;
    size_t bytes_written = snprintf(head, bytes_left, "\"%s\"", ((char*)object + accessor.offset));
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_bool) {
    size_t bytes_left = end - head;
    const char *value = *(bool*)((char*)object + accessor.offset) ? "true" : "false";
    size_t bytes_written = snprintf(head, bytes_left, "%s", value);
    if (!cerial_assert(bytes_left > bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_object) {
    size_t bytes_written = cerial_write_json(accessor.super_cerial, (const void*)((const char*)object + accessor.offset), head, end-head);
    if (!cerial_assert(bytes_written)) return 0;
    head += bytes_written;
  }

  return head;
}
