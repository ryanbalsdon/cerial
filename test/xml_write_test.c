#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "cerial.h"



void test_one_int(void)
{
  printf("test_one_int\n");
  char *expected = "<meaning>42</meaning>";
  char output[1024];

  typedef struct {
    int meaning;
  } one_int;

  make_cerial(one_int, cerial_option_no_root,
    cerial_access(one_int, meaning, cerial_int)
  );

  one_int test_object;
  test_object.meaning = 42;

  size_t bytes = cerial_write_xml(&one_int_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_root(void)
{
  printf("test_root\n");
  char *expected = "<one_int><meaning>42</meaning></one_int>";
  char output[1024];

  typedef struct {
    int meaning;
  } one_int;

  make_cerial(one_int, 0,
    cerial_access(one_int, meaning, cerial_int)
  );

  one_int test_object;
  test_object.meaning = 42;

  size_t bytes = cerial_write_xml(&one_int_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_two_int(void)
{
  printf("test_two_int\n");
  char *expected = "<car_shop><broken_tires>21</broken_tires><fixed_tires>6</fixed_tires></car_shop>";
  char output[1024];

  typedef struct {
    int broken_tires;
    int fixed_tires;
  } car_shop;

  make_cerial(car_shop, 0,
    cerial_access(car_shop, broken_tires, cerial_int),
    cerial_access(car_shop, fixed_tires, cerial_int)
  );

  car_shop my_shop;
  my_shop.broken_tires = 21;
  my_shop.fixed_tires = 6;

  size_t bytes = cerial_write_xml(&car_shop_cerial, &my_shop, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_string(void)
{
  printf("test_string\n");
  char *expected = "<badge><serial>x609</serial></badge>";
  char output[1024];

  typedef struct {
    char serial[12];
  } badge;

  make_cerial(badge, 0,
    cerial_access(badge, serial, cerial_str, {12})
  );

  badge the_badge;
  strcpy(the_badge.serial, "x609");

  size_t bytes = cerial_write_xml(&badge_cerial, &the_badge, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_bool(void)
{
  printf("test_bool\n");
  char *expected = "<table><has_legs>true</has_legs></table>";
  char output[1024];

  typedef struct {
    bool has_legs;
  } table;

  make_cerial(table, 0,
    cerial_access(table, has_legs, cerial_bool)
  );

  table dining_room_table = {true};

  size_t bytes = cerial_write_xml(&table_cerial, &dining_room_table, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_object(void)
{
  printf("test_object\n");
  char output[1024];
  char *expected = "<table>"
                   "<first_leg>"
                     "<is_broken>false</is_broken>"
                     "<round>true</round>"
                   "</first_leg>"
                   "<second_leg>"
                     "<is_broken>true</is_broken>"
                     "<round>false</round>"
                   "</second_leg>"
                 "</table>";

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

  make_cerial(table, 0,
    cerial_access(table, first_leg, cerial_object, {.super_cerial=&leg_cerial}),
    cerial_access(table, second_leg, cerial_object, {.super_cerial=&leg_cerial})
  );

  table dining_room_table;
  dining_room_table.first_leg.is_broken = false;
  dining_room_table.first_leg.round = true;
  dining_room_table.second_leg.is_broken = true;
  dining_room_table.second_leg.round = false;

  size_t bytes = cerial_write_xml(&table_cerial, &dining_room_table, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_array(void)
{
  printf("test_array\n");
  char output[1024];
  char *expected = "<table>"
                   "<leg_colours>"
                     "<value>16772693</value>"
                     "<value>21930</value>"
                     "<value>3364249</value>"
                     "<value>3446951</value>"
                   "</leg_colours>"
                 "</table>";

  typedef struct {
    int leg_colours[4];
  } table;

  make_cerial(table, 0,
    cerial_access(table, leg_colours, cerial_int, {4}, true),
  );

  table dining_room_table;
  dining_room_table.leg_colours[0] = 0xffee55;
  dining_room_table.leg_colours[1] = 0x0055aa;
  dining_room_table.leg_colours[2] = 0x335599;
  dining_room_table.leg_colours[3] = 0x3498a7;

  size_t bytes = cerial_write_xml(&table_cerial, &dining_room_table, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

int main(void)
{
  test_one_int();
  test_root();
  test_two_int();
  test_string();
  test_bool();
  test_object();
  test_array();
  printf("pass\n");
}