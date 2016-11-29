#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include "cerial.h"

// #define DEBUG
#include "cerial_internal.h"

static size_t cerial_x690_get_int_length(long long integer);
static size_t cerial_x690_write_int(uint8_t *head, long long integer, size_t byte_count);
static size_t cerial_x690_write_length(uint8_t *head, const uint8_t *end, long long length);
static size_t cerial_x690_write_float(uint8_t *head, const uint8_t *end, double value);
static size_t cerial_x690_update_length(uint8_t *length_address, uint8_t *value_address, size_t length);
static void cerial_x690_get_float(double value, int *exponent, long long *mantissa);
static size_t cerial_x690_write_value(cerial_accessor accessor, const void *object, uint8_t *head, const uint8_t *end);
static size_t cerial_x690_write_array(cerial_accessor accessor, const void *object, uint8_t *head, const uint8_t *end);
static size_t cerial_x690_write_object(cerial *self, const void *object, uint8_t *output, size_t size, bool write_root);


size_t cerial_x690_write(cerial *self, const void *object, uint8_t *output, size_t size)
{
  return cerial_x690_write_object(self, object, output, size, !(self->options & cerial_option_no_root));
}

static size_t cerial_x690_write_object(cerial *self, const void *object, uint8_t *output, size_t size, bool write_root)
{
  uint8_t *head = output;
  const uint8_t *end = head + size;
  uint8_t *value_address = NULL;
  uint8_t *length_address = NULL;

  if (write_root) {
    if (!cerial_assert(head < end)) return 0;
    *head++ = 0x30;

    size_t length = end - head;
    size_t length_bytes = cerial_x690_write_length(head, end, length);
    if (!cerial_assert(length_bytes)) return 0;
    length_address = head;
    head += length_bytes;
    value_address = head;
  }

  for (int i=0; i<self->count; i++) {
    cerial_accessor accessor = self->accessors[i];
    if (accessor.is_array) {
      size_t bytes_written = cerial_x690_write_array(accessor, object, head, end);
      if (!cerial_assert(bytes_written)) return 0;
      head += bytes_written;
    }
    else {
      size_t bytes_written = cerial_x690_write_value(accessor, object, head, end);
      if (!cerial_assert(bytes_written)) return 0;
      head += bytes_written;
    }
  }

  if (write_root) {
      size_t new_length_bytes = cerial_x690_update_length(length_address, value_address, head - value_address);
      head += new_length_bytes - (value_address - length_address);
  }

  return head - output;
}

static size_t cerial_x690_write_value(cerial_accessor accessor, const void *object, uint8_t *head, const uint8_t *end)
{
  uint8_t *start = head;

  if (accessor.type == cerial_bool) {
    if (!cerial_assert(head+3 < end)) return 0;
    const uint8_t value = *(bool*)((char*)object + accessor.offset) ? 0x01 : 0x00;
    *head++ = 0x01;
    *head++ = 0x01;
    *head++ = value;
  }
  else if (accessor.type == cerial_int) {
    int value = *(int*)((char*)object + accessor.offset);

    if (!cerial_assert(head+1 < end)) return 0;
    *head++ = 0x02;

    size_t length = cerial_x690_get_int_length(value);
    size_t length_bytes = cerial_x690_write_length(head, end, length);
    if (!cerial_assert(length_bytes)) return 0;
    head += length_bytes;

    if (!cerial_assert(head+length < end)) return 0;
    head += cerial_x690_write_int(head, value, length);
  }
  else if (accessor.type == cerial_str) {
    char *value = ((char*)object + accessor.offset);
    size_t length = strlen(value);
    if (accessor.buffer_size < length) length = accessor.buffer_size;
    if (!cerial_assert(head+1 < end)) return 0;
    *head++ = 0x1A;

    size_t length_bytes = cerial_x690_write_length(head, end, length);
    if (!cerial_assert(length_bytes)) return 0;
    head += length_bytes;

    if (!cerial_assert(head+length < end)) return 0;
    memcpy(head, value, length);
    head += length;
  }
  else if (accessor.type == cerial_float || accessor.type == cerial_double) {
    double value = 0.0;
    if (accessor.type == cerial_float) {
      value = *(float*)((char*)object + accessor.offset);
    }
    else if (accessor.type == cerial_double) {
      value = *(double*)((char*)object + accessor.offset);
    }

    size_t bytes_written = cerial_x690_write_float(head, end, value);
    if (!cerial_assert(bytes_written)) return 0;
    head += bytes_written;
  }
  else if (accessor.type == cerial_object) {
    size_t bytes_written = cerial_x690_write_object(accessor.super_cerial, (const void*)((const char*)object + accessor.offset), head, end-head, true);
    if (!cerial_assert(bytes_written)) return 0;
    head += bytes_written;
  }

  return head - start;
}

static size_t cerial_x690_write_array(cerial_accessor accessor, const void *object, uint8_t *head, const uint8_t *end)
{
  uint8_t *start = head;

  if (!cerial_assert(head < end)) return 0;
  *head++ = 0x31;

  size_t length_guess = end - head;
  size_t length_bytes = cerial_x690_write_length(head, end, length_guess);
  if (!cerial_assert(length_bytes)) return 0;
  uint8_t *length_address = head;
  head += length_bytes;
  uint8_t *value_address = head;

  char *object_head = (char*)object;
  size_t object_increment = cerial_accessor_size(accessor);
  for (int j=0; j<accessor.buffer_size; j++) {
    size_t bytes_written = cerial_x690_write_value(accessor, object_head, head, end);
    if (!cerial_assert(bytes_written)) return 0;
    head += bytes_written;
    object_head += object_increment;
  }

  if (head - value_address != length_guess) {
    size_t new_length_bytes = cerial_x690_update_length(length_address, value_address, head - value_address);
    head += new_length_bytes - length_bytes;
  }

  return head - start;
}

static size_t cerial_x690_get_int_length(long long value)
{
  size_t length = 0;
  for (length = sizeof(long long); length > 1; length--) {
    uint8_t byte = (value  >> (length-1)*8) & 0xFF;
    if (byte == 0x00) continue;
    if (byte == 0xFF && length == 1) break;
    if (byte == 0xFF) {
      uint8_t next_byte = (value >> (length-2)*8) & 0xFF;
      if (next_byte & 0x80) continue;
      break;
    }
    break;
  }
  return length;
}

static size_t cerial_x690_write_int(uint8_t *head, long long value, size_t length)
{
  for (int i=length-1; i>=0; i--) {
    uint8_t byte = (value >> i*8) & 0xFF;
    *head++ = byte;
  }
  return length;
}

static size_t cerial_x690_write_length(uint8_t *head, const uint8_t *end, long long length)
{
  uint8_t *start = head;
  if (length < 0x80) {
    if (!cerial_assert(head < end)) return 0;
    *head++ = length;
  }
  else {
    size_t length_of_length = cerial_x690_get_int_length(length);
    if (!cerial_assert(length_of_length < 0x80)) return 0;
    if (!cerial_assert(head+length_of_length < end)) return 0;
    *head++ = length_of_length | 0x80;
    head += cerial_x690_write_int(head, length, length_of_length);
  }

  return head - start;
}

static size_t cerial_x690_write_float(uint8_t *head, const uint8_t *end, double value)
{
  uint8_t *start = head;
  if (!cerial_assert(!isnan(value))) return 0;
  else if (isinf(value)){
    if (!cerial_assert(head+2 < end)) return 0;
    *head++ = 0x09;
    if (signbit(value)) *head++ = 0x41;
    else *head++ = 0x40;
  }
  else {
    int exponent = 0;
    long long mantissa = 0;
    cerial_x690_get_float(value, &exponent, &mantissa);

    // {1 S bb ff ee} {Octets for E} {Octets for N}
    size_t length_of_exponent = cerial_x690_get_int_length(exponent);
    size_t length_of_mantissa = cerial_x690_get_int_length(mantissa);
    size_t length = 1 + (length_of_exponent > 3 ? 1 : 0) + length_of_exponent + length_of_mantissa;

    if (!cerial_assert(head+1 < end)) return 0;
    *head++ = 0x09;

    size_t length_bytes = cerial_x690_write_length(head, end, length);
    if (!cerial_assert(length_bytes)) return 0;
    head += length_bytes;

    if (!cerial_assert(head+1 < end)) return 0;
    *head++ = (0x80) | (signbit(value) << 6) | (0x20) | (0x00) | (length_of_exponent > 3 ? 0x03 : length_of_exponent - 1);
    if (length_of_exponent > 3) {
      if (!cerial_assert(length_of_exponent < 0xFF)) return 0;
      if (!cerial_assert(head+1 < end)) return 0;
      *head++ = length_of_exponent;
    }

    if (!cerial_assert(head+length_of_exponent+length_of_mantissa < end)) return 0;
    head += cerial_x690_write_int(head, exponent, length_of_exponent);
    head += cerial_x690_write_int(head, mantissa, length_of_mantissa);
  }

  return head - start;
}

static void cerial_x690_get_float(double value, int *out_exponent, long long *out_mantissa)
{
  int exponent = 0;
  double floating_mantissa = frexp(value, &exponent);
  long long mantissa = trunc(ldexp(floating_mantissa, DBL_MANT_DIG));
  if (signbit(mantissa)) mantissa *= -1;
  exponent = exponent - DBL_MANT_DIG;
  for (int i = 0; i < DBL_MANT_DIG / 8; i++) {
    uint8_t byte = mantissa & 0xFF;
    if (byte == 0x00) {
      mantissa >>= 8;
      exponent += 8;
    }
    else break;
  }
  *out_exponent = exponent;
  *out_mantissa = mantissa;
}

static size_t cerial_x690_update_length(uint8_t *length_address, uint8_t *value_address, size_t length)
{
  size_t old_length_bytes = value_address - length_address;
  size_t new_length_bytes = cerial_x690_write_length(length_address, value_address, length);
  if (!cerial_assert(new_length_bytes)) return 0;

  if (old_length_bytes == new_length_bytes) return new_length_bytes;
  int difference = new_length_bytes - old_length_bytes;
  for (int i=0; i<length; i++) {
    value_address[i+difference] = value_address[i];
  }
  return new_length_bytes;
}
