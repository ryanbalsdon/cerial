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

  typedef struct {
    bool this_is_true;
  } one_bool;

  make_cerial(one_bool, cerial_option_no_root,
    cerial_access(one_bool, this_is_true, cerial_bool)
  );

  {
    one_bool not_true;
    not_true.this_is_true = false;
    uint8_t definite_short[] = {0x01, 0x01, 0x01};
    size_t bytes_read = cerial_read_x690(&one_bool_cerial, &not_true, definite_short, sizeof(definite_short));
    assert(bytes_read == sizeof(definite_short));
    assert(not_true.this_is_true);
  }

  {
    one_bool not_true;
    not_true.this_is_true = false;
    uint8_t definite_long[] = {0x01, 0x82, 0x00, 0x01, 0xFF};
    size_t bytes_read = cerial_read_x690(&one_bool_cerial, &not_true, definite_long, sizeof(definite_long));
    assert(bytes_read == sizeof(definite_long));
    assert(not_true.this_is_true);
  }

  {
    one_bool not_true;
    not_true.this_is_true = false;
    uint8_t indefinite[] = {0x01, 0x80, 0x01, 0x00, 0x00};
    size_t bytes_read = cerial_read_x690(&one_bool_cerial, &not_true, indefinite, sizeof(indefinite));
    assert(bytes_read == sizeof(indefinite));
    assert(not_true.this_is_true);
  }
}

void test_two_int(void)
{
  printf("test_two_int\n");
  uint8_t sample[] = {0x02, 0x01, 0xFC, 0x02, 0x02, 0x10, 0x00};
  typedef struct {
    int negative;
    int large;
  } double_int;

  make_cerial(double_int, cerial_option_no_root,
    cerial_access(double_int, negative, cerial_int),
    cerial_access(double_int, large, cerial_int)
  );

  double_int test_object;
  test_object.negative = 0xCCCCCCCC;
  test_object.large = 0xCCCCCCCC;

  size_t bytes = cerial_read_x690(&double_int_cerial, &test_object, sample, sizeof(sample));
  assert(bytes == sizeof(sample));
  assert(test_object.negative == -4);
  assert(test_object.large == 4096);
}

void test_float(void)
{
  printf("test_float\n");
  uint8_t sample[] = {0x09, 0x00, 0x09, 0x01, 0x40, 0x09, 0x03, 0xA9, 0x02, 0xAA};
  typedef struct {
    float bitterness;
    double sugar;
    double milk;
  } double_double;

  make_cerial(double_double, cerial_option_no_root,
    cerial_access(double_double, bitterness, cerial_float),
    cerial_access(double_double, sugar, cerial_double),
    cerial_access(double_double, milk, cerial_double)
  );

  double_double coffee;
  coffee.sugar = (float)0xCCCCCCCC;
  coffee.milk = (float)0xCCCCCCCC;

  size_t bytes = cerial_read_x690(&double_double_cerial, &coffee, sample, sizeof(sample));
  assert(bytes == sizeof(sample));
  assert(coffee.bitterness == 0.0);
  assert(coffee.sugar == INFINITY);
  assert(coffee.milk == 2720.0);
}

void test_string(void)
{
  printf("test_string\n");
  uint8_t sample[] = {0x1A, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
  typedef struct {
    char text[12];
  } greeting;

  make_cerial(greeting, cerial_option_no_root,
    cerial_access(greeting, text, cerial_str, {12})
  );

  greeting morning;
  strcpy(morning.text, "Goodbye");

  size_t bytes = cerial_read_x690(&greeting_cerial, &morning, sample, sizeof(sample));
  assert(bytes == sizeof(sample));
  assert(strcmp(morning.text, "Hello") == 0);
}

void test_object(void)
{
  printf("test_object\n");
  uint8_t sample[] = {0x30, 0x06, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x30, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00};
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
  dining_room_table.first_leg.is_broken = true;
  dining_room_table.first_leg.round = false;
  dining_room_table.second_leg.is_broken = false;
  dining_room_table.second_leg.round = true;


  size_t bytes = cerial_read_x690(&table_cerial, &dining_room_table, sample, sizeof(sample));
  assert(bytes == sizeof(sample));
  assert(dining_room_table.first_leg.is_broken == false);
  assert(dining_room_table.first_leg.round == true);
  assert(dining_room_table.second_leg.is_broken == true);
  assert(dining_room_table.second_leg.round == false);
}

void test_array(void)
{
  printf("test_array\n");
  uint8_t sample[] = {
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
  dining_room_table.leg_colours[0] = 0xcccccc;
  dining_room_table.leg_colours[1] = 0xcccccc;
  dining_room_table.leg_colours[2] = 0xcccccc;
  dining_room_table.leg_colours[3] = 0xcccccc;

  size_t bytes = cerial_read_x690(&table_cerial, &dining_room_table, sample, sizeof(sample));
  assert(bytes == sizeof(sample));
  assert(dining_room_table.leg_colours[0] == 0x7fee55);
  assert(dining_room_table.leg_colours[1] == 0x0055aa);
  assert(dining_room_table.leg_colours[2] == 0x000019);
  assert(dining_room_table.leg_colours[3] == 0x3498a7);
}

void test_root(void)
{
  printf("test_root\n");

  typedef struct {
    bool this_is_true;
  } one_bool;

  make_cerial(one_bool, 0,
    cerial_access(one_bool, this_is_true, cerial_bool)
  );

  {
    one_bool not_true;
    not_true.this_is_true = false;
    uint8_t definite_short[] = {0x30, 0x03, 0x01, 0x01, 0x01};
    size_t bytes_read = cerial_read_x690(&one_bool_cerial, &not_true, definite_short, sizeof(definite_short));
    assert(bytes_read == sizeof(definite_short));
    assert(not_true.this_is_true);
  }

  {
    one_bool not_true;
    not_true.this_is_true = false;
    uint8_t indefinite[] = {0x30, 0x80, 0x01, 0x01, 0x01, 0x00, 0x00};
    size_t bytes_read = cerial_read_x690(&one_bool_cerial, &not_true, indefinite, sizeof(indefinite));
    assert(bytes_read == sizeof(indefinite));
    assert(not_true.this_is_true);
  }
}

int main(void)
{
  test_bool();
  test_two_int();
  test_float();
  test_string();
  test_object();
  test_array();
  test_root();
  printf("pass\n");
}