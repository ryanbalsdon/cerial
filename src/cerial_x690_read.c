#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "cerial.h"

// #define DEBUG
#include "cerial_internal.h"

typedef struct {
  uint8_t class;
  uint8_t constructed;
  uint32_t number;
} cerial_x609_tag;

typedef struct {
  const uint8_t *start;
  size_t length;
} cerial_x609_value;

static long long cerial_x690_read_integer(const uint8_t *bytes, size_t length);
static size_t cerial_x690_read_tag(const uint8_t *tag_start, const uint8_t *end, cerial_x609_tag *tag);
static size_t cerial_x690_read_length(const uint8_t *tag_start, const uint8_t *end, cerial_x609_value *value);
static size_t cerial_x690_read_value(cerial_accessor accessor, cerial_x609_tag tag, void *output, const uint8_t *value_start, size_t length);
size_t cerial_x690_read_object(cerial *self, void *output, const uint8_t *bytes, size_t size, bool read_root);

size_t cerial_x690_read(cerial *self, void *output, const uint8_t *bytes, size_t size)
{
  return cerial_x690_read_object(self, output, bytes, size, true);
}

size_t cerial_x690_read_object(cerial *self, void *output, const uint8_t *bytes, size_t size, bool read_root)
{
  const uint8_t *end = bytes + size;
  const uint8_t *head = bytes;
  if (!cerial_assert(head)) return 0;

  if (read_root && !(self->options & cerial_option_no_root)) {
    cerial_x609_tag tag = {0};
    size_t tag_size = cerial_x690_read_tag(head, end, &tag);
    if (!cerial_assert(tag_size)) return 0;
    head += tag_size;

    cerial_x609_value value = {0};
    size_t length_size = cerial_x690_read_length(head, end, &value);
    if (!cerial_assert(length_size)) return 0;
    head += length_size;

    if (!cerial_assert(tag.class == 0x00)) return 0;
    if (!cerial_assert(tag.constructed == 0x01)) return 0;
    if (!cerial_assert(tag.number == 16)) return 0;

    size_t object_bytes = cerial_x690_read_object(self, output, value.start, value.length, false);
    if (!cerial_assert(object_bytes == value.length)) return 0;
    return head - bytes;
  }

  for (int i=0; i<self->count; i++) {
    cerial_accessor accessor = self->accessors[i];
    cerial_x609_tag tag = {0};
    size_t tag_size = cerial_x690_read_tag(head, end, &tag);
    if (!cerial_assert(tag_size)) return 0;
    head += tag_size;

    cerial_x609_value value = {0};
    size_t length_size = cerial_x690_read_length(head, end, &value);
    if (!cerial_assert(length_size)) return 0;
    head += length_size;

    if (accessor.is_array) {
      char *array_output = (char*)output;
      size_t array_output_increment = cerial_accessor_size(accessor);
      const uint8_t *array_head = value.start;
      const uint8_t *array_end = array_head + value.length;

      while (array_head < array_end) {
        cerial_x609_tag array_tag = {0};
        size_t tag_size = cerial_x690_read_tag(array_head, array_end, &array_tag);
        if (!cerial_assert(tag_size)) return 0;
        array_head += tag_size;

        cerial_x609_value array_value = {0};
        size_t length_size = cerial_x690_read_length(array_head, array_end, &array_value);
        if (!cerial_assert(length_size)) return 0;
        array_head += length_size;

        size_t bytes_read = cerial_x690_read_value(accessor, array_tag, array_output, array_value.start, array_value.length);
        if (!cerial_assert(bytes_read == array_value.length)) return 0;

        array_output += array_output_increment;
      }
    }
    else {
      size_t bytes_read = cerial_x690_read_value(accessor, tag, output, value.start, value.length);
      if (!cerial_assert(bytes_read == value.length)) return 0;
    }
  }

  return head - bytes;
}

static size_t cerial_x690_read_tag(const uint8_t *head, const uint8_t *end, cerial_x609_tag *tag)
{
  const uint8_t *tag_start = head;
  tag->class = (*head & 0xC0) >> 6;
  tag->constructed = (*head & 0x20) >> 5;
  tag->number = *head & 0x1F;
  head++;
  if (!cerial_assert(head <= end)) return 0;

  if (tag->number == 0x1F) {
    uint8_t tag_number_byte = *head;
    head++;
    if (!cerial_assert(head <= end)) return 0;
    while (tag_number_byte & 0x80) {
      tag->number = (tag->number << 7) | (tag_number_byte & 0x7F);
      head++;
      if (!cerial_assert(head <= end)) return 0;
    }
  }

  return head - tag_start;
}

static size_t cerial_x690_read_length(const uint8_t *head, const uint8_t *end, cerial_x609_value *value)
{
  const uint8_t *length_start = head;
  size_t length_of_length = *head;
  size_t length = 0;
  head++;
  if (!cerial_assert(head <= end)) return 0;
  if ((length_of_length & 0x80) && (length_of_length & 0x7F)) {
    length_of_length &= 0x7F;
    if (!cerial_assert(length_of_length <= sizeof(size_t))) return 0;
    for (int i=0; i<length_of_length; i++) {
      length = (length << 8) | *head;
      head++;
      if (!cerial_assert(head <= end)) return 0;
    }
  }
  else if ((length_of_length & 0x80) == 0x00) {
    length = length_of_length;
  }

  const uint8_t *value_start = head;
  if (length_of_length == 0x80) {
    for (head++; head<end; head++) {
      if (head[0] == 0x00 && head[1] == 0x00) {
        length = head - value_start;
        head+=2;
        break;
      }
    }
  }
  else {
    head += length;
    if (!cerial_assert(head <= end)) return 0;
  }

  value->length = length;
  value->start = value_start;
  return head - length_start;
}

static size_t cerial_x690_read_value(cerial_accessor accessor, cerial_x609_tag tag, void *output, const uint8_t *value_start, size_t length)
{
  if (!cerial_assert(tag.class == 0x00)) return 0;
  if (accessor.type == cerial_bool) {
    if (!cerial_assert(tag.constructed == 0x00)) return 0;
    if (!cerial_assert(tag.number == 0x01)) return 0;
    if (!cerial_assert(length == 1)) return 0;
    *(bool*)((char*)output + accessor.offset) = *value_start ? true : false;
  }
  else if (accessor.type == cerial_int) {
    if (!cerial_assert(length <= cerial_accessor_size(accessor))) return 0;
    if (!cerial_assert(tag.constructed == 0x00)) return 0;
    if (!cerial_assert(tag.number == 0x02)) return 0;
    long long thre = cerial_x690_read_integer(value_start, length);
    *(int*)((char*)output + accessor.offset) = (int)thre;
  }
  else if (accessor.type == cerial_float || accessor.type == cerial_double) {
    if (!cerial_assert(tag.constructed == 0x00)) return 0;
    if (!cerial_assert(tag.number == 0x09)) return 0;
    const uint8_t *value_head = value_start;
    double value = 0;

    if (length == 1 && *value_head == 0x40) value = +INFINITY;
    else if (length == 1 && *value_head == 0x41) value = -INFINITY;
    else if (length > 0) {
      if (!cerial_assert(*value_head & 0x80)) return 0; //binary only
      // {1 S bb ff ee} {Octets for E} {Octets for N}
      uint8_t negative = *value_head & 0x40;
      uint8_t base = (*value_head & 0x30) >> 4;
      if (!cerial_assert(base == 0x02)) return 0; // base 16 only
      uint8_t scaling_factor = (*value_head & 0x0C) >> 2;
      uint8_t exponent_length = *value_head & 0x03;
      if (exponent_length == 0x03) {
        if (!cerial_assert(length > 1)) return 0;
        value_head++;
        exponent_length = 3 + *value_head;
      }
      else exponent_length++;
      value_head++;
      if (!cerial_assert(length > exponent_length + 1)) return 0;
      long long exponent = cerial_x690_read_integer(value_head, exponent_length);
      value_head += exponent_length;
      unsigned long long mantissa = 0;
      for (const uint8_t *integer_byte = value_head; integer_byte < value_start + length; integer_byte++) {
        mantissa = (mantissa << 8) | *integer_byte;
      }
      value = ldexp(ldexp(mantissa, scaling_factor), exponent);
      if (negative) value *= -1.0;
    }
    if (accessor.type == cerial_float) {
      *(float*)((char*)output + accessor.offset) = (float)value;
    }
    else if (accessor.type == cerial_double) {
      *(double*)((char*)output + accessor.offset) = (double)value;
    }
  }
  else if (accessor.type == cerial_str) {
    if (!cerial_assert(tag.number == 12 || tag.number == 22 || tag.number == 26)) return 0;
    size_t size_to_copy = length < accessor.buffer_size ? length : accessor.buffer_size;
    memcpy((char*)output + accessor.offset, value_start, size_to_copy);
    if (size_to_copy < accessor.buffer_size) *((char*)output + accessor.offset + size_to_copy) = '\0';
  }
  else if (accessor.type == cerial_object) {
    if (!cerial_assert(tag.constructed == 0x01)) return 0;
    if (!cerial_assert(tag.number == 16)) return 0;
    size_t super_bytes = cerial_x690_read(accessor.super_cerial, (void*)((char*)output + accessor.offset), value_start, length);
    if (!cerial_assert(super_bytes == length)) return 0;
  }

  return length;
}

static long long cerial_x690_read_integer(const uint8_t *bytes, size_t length)
{
  long long value = 0;
  for (size_t i=0; i<length; i++) {
    value = (value << 8) | bytes[i];
  }
  long long sign = (bytes[0] & 0x80) ? 0xFF : 0x00;
  if (sign) for (size_t i=length; i<sizeof(long long); i++) {
    value |= sign << i*8;
  }
  return value;
}
