#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "cerial.h"

// #define DEBUG
#include "cerial_internal.h"

static char* cerial_xml_write_value(cerial_accessor accessor, const void *object, char *head, const char *end);
static char* cerial_xml_write_array(cerial_accessor accessor, const void *object, char *head, const char *end);


size_t cerial_xml_write(cerial *self, const void *object, char *output, size_t size)
{
  char *head = output;
  const char *end = head + size;

  if (!(self->options & cerial_option_no_root)) {
    head += snprintf(head, end - head, "<%s>", self->name);
    if (!cerial_assert(head <= end)) return 0;
  }

  for (int i=0; i<self->count; i++) {
    cerial_accessor accessor = self->accessors[i];
    head += snprintf(head, end - head, "<%s>", accessor.name);
    if (!cerial_assert(head <= end)) return 0;

    if (accessor.is_array) {
      head = cerial_xml_write_array(accessor, object, head, end);
    }
    else {
      head = cerial_xml_write_value(accessor, object, head, end);
    }
    if (!cerial_assert(head)) return 0;

    head += snprintf(head, end - head, "</%s>", accessor.name);
    if (!cerial_assert(head <= end)) return 0;
  }


  if (!(self->options & cerial_option_no_root)) {
    head += snprintf(head, end - head, "</%s>", self->name);
    if (!cerial_assert(head <= end)) return 0;
  }

  return head - output;
}

static char* cerial_xml_write_array(cerial_accessor accessor, const void *object, char *head, const char *end)
{
  if (!cerial_assert(end - head >= 2)) return 0;
  char *object_head = (char*)object;
  size_t object_increment = cerial_accessor_size(accessor);

  for (int j=0; j<accessor.buffer_size; j++) {
    head += snprintf(head, end - head, "<value>");
    if (!cerial_assert(head <= end)) return 0;

    head = cerial_xml_write_value(accessor, object_head, head, end);
    if (!cerial_assert(head)) return 0;

    head += snprintf(head, end - head, "</value>");
    if (!cerial_assert(head <= end)) return 0;

    object_head += object_increment;
  }

  return head;
}

static char* cerial_xml_write_value(cerial_accessor accessor, const void *object, char *head, const char *end)
{
  if (accessor.type == cerial_int) {
    head += snprintf(head, end - head, "%d", *(int*)((char*)object + accessor.offset));
  }
  else if (accessor.type == cerial_float) {
    head += snprintf(head, end - head, "%f", *(float*)((char*)object + accessor.offset));
  }
  else if (accessor.type == cerial_double) {
    head += snprintf(head, end - head, "%f", *(double*)((char*)object + accessor.offset));
  }
  else if (accessor.type == cerial_str) {
    head += snprintf(head, end - head, "%s", ((char*)object + accessor.offset));
  }
  else if (accessor.type == cerial_bool) {
    const char *value = *(bool*)((char*)object + accessor.offset) ? "true" : "false";
    head += snprintf(head, end - head, "%s", value);
  }
  else if (accessor.type == cerial_object) {
    size_t bytes_written = cerial_xml_write(accessor.super_cerial, (const void*)((const char*)object + accessor.offset), head, end-head);
    if (!cerial_assert(bytes_written)) return 0;
    head += bytes_written;
  }

  if (!cerial_assert(head <= end)) return 0;
  return head;
}
