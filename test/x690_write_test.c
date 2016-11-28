#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include "cerial.h"



void test_bool(void)
{
  printf("test_bool\n");
  uint8_t expected[] = {0x01, 0x01, 0x01};
  uint8_t buffer[4096];

  typedef struct {
    bool this_is_true;
  } one_bool;

  make_cerial(one_bool, cerial_option_no_root,
    cerial_access(one_bool, this_is_true, cerial_bool)
  );

  one_bool not_true;
  not_true.this_is_true = true;
  
  size_t bytes_written = cerial_x690_write(&one_bool_cerial, &not_true, buffer, sizeof(buffer));
  assert(bytes_written == sizeof(expected));
  assert(memcmp(expected, buffer, bytes_written) == 0);
}

void test_two_int(void)
{
  printf("test_two_int\n");
  uint8_t expected[] = {0x02, 0x01, 0xFC, 0x02, 0x02, 0x10, 0x00};
  uint8_t buffer[4096];

  typedef struct {
    int negative;
    int large;
  } double_int;

  make_cerial(double_int, cerial_option_no_root,
    cerial_access(double_int, negative, cerial_int),
    cerial_access(double_int, large, cerial_int)
  );

  double_int test_object;
  test_object.negative = -4;
  test_object.large = 4096;

  size_t bytes = cerial_x690_write(&double_int_cerial, &test_object, buffer, sizeof(buffer));
  assert(bytes == sizeof(expected));
  assert(memcmp(expected, buffer, bytes) == 0);
}

void test_string(void)
{
  printf("test_string\n");
  uint8_t expected[] = {0x1A, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
  uint8_t buffer[4096];

  typedef struct {
    char text[12];
  } greeting;

  make_cerial(greeting, cerial_option_no_root,
    cerial_access(greeting, text, cerial_str, {12})
  );

  greeting morning;
  strcpy(morning.text, "Hello");

  size_t bytes = cerial_x690_write(&greeting_cerial, &morning, buffer, sizeof(buffer));
  assert(bytes == sizeof(expected));
  assert(memcmp(expected, buffer, bytes) == 0);
}

void test_float(void)
{
  printf("test_float\n");
  uint8_t expected[] = {
    0x09, 0x03, 0xA0, 0xFC, 0x10,
    0x09, 0x09, 0xE0, 0xCE, 0x10, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCD,
    0x09, 0x04, 0xA0, 0xFF, 0x15, 0x40
  };
  uint8_t buffer[4096];

  typedef struct {
    float bitterness;
    double sugar;
    double milk;
  } double_double;

  make_cerial(double_double, cerial_option_no_root,
    cerial_access(double_double, bitterness, cerial_float),
    cerial_access(double_double, sugar, cerial_double),
    cerial_access(double_double, milk, cerial_double),
  );

  double_double coffee;
  coffee.bitterness = 1.0f;
  coffee.sugar = -4.2;
  coffee.milk = 2720.0;

  size_t bytes_written = cerial_x690_write(&double_double_cerial, &coffee, buffer, sizeof(buffer));
  assert(bytes_written == sizeof(expected));
  assert(memcmp(expected, buffer, bytes_written) == 0);
}


void test_object(void)
{
  printf("test_object\n");
  uint8_t expected[] = {0x30, 0x06, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x30, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00};
  uint8_t buffer[4096];

  typedef struct {
    bool is_broken;
    bool round;
  } leg;

  typedef struct {
    leg first_leg;
    leg second_leg;
  } table;

  make_cerial(leg, cerial_option_no_root,
    cerial_access(leg, is_broken, cerial_bool),
    cerial_access(leg, round, cerial_bool)
  );

  make_cerial(table, cerial_option_no_root,
    cerial_access(table, first_leg, cerial_object, {.super_cerial=&leg_cerial}),
    cerial_access(table, second_leg, cerial_object, {.super_cerial=&leg_cerial})
  );

  table dining_room_table;
  dining_room_table.first_leg.is_broken = false;
  dining_room_table.first_leg.round = true;
  dining_room_table.second_leg.is_broken = true;
  dining_room_table.second_leg.round = false;


  size_t bytes_written = cerial_x690_write(&table_cerial, &dining_room_table, buffer, sizeof(buffer));
  assert(bytes_written == sizeof(expected));
  assert(memcmp(expected, buffer, bytes_written) == 0);
}

void test_array(void)
{
  printf("test_array\n");
  uint8_t buffer[4096];
  uint8_t expected[] = {
    0x31, 0x11,
      0x02, 0x03, 0x7F, 0xEE, 0x55,
      0x02, 0x02, 0x55, 0xAA,
      0x02, 0x01, 0x19,
      0x02, 0x03, 0x34, 0x98, 0xA7
    };

  typedef struct {
    int leg_colours[4];
  } table;

  make_cerial(table, cerial_option_no_root,
    cerial_access(table, leg_colours, cerial_int, {4}, true),
  );

  table dining_room_table;
  dining_room_table.leg_colours[0] = 0x7fee55;
  dining_room_table.leg_colours[1] = 0x0055aa;
  dining_room_table.leg_colours[2] = 0x000019;
  dining_room_table.leg_colours[3] = 0x3498a7;

  size_t bytes_written = cerial_x690_write(&table_cerial, &dining_room_table, buffer, sizeof(buffer));
  assert(bytes_written == sizeof(expected));
  assert(memcmp(expected, buffer, bytes_written) == 0);
}

void test_root(void)
{
  printf("test_root\n");
  uint8_t expected[] = {0x30, 0x03, 0x01, 0x01, 0x01};
  uint8_t buffer[4096];

  typedef struct {
    bool this_is_true;
  } one_bool;

  make_cerial(one_bool, 0,
    cerial_access(one_bool, this_is_true, cerial_bool)
  );

  one_bool not_true;
  not_true.this_is_true = true;
  
  size_t bytes_written = cerial_x690_write(&one_bool_cerial, &not_true, buffer, sizeof(buffer));
  assert(bytes_written == sizeof(expected));
  assert(memcmp(expected, buffer, bytes_written) == 0);
}

int main(void)
{
  test_bool();
  test_two_int();
  test_string();
  test_float();
  test_object();
  test_array();
  test_root();
  printf("pass\n");
}