#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "cerial.h"



void test_one_int(void)
{
  printf("test_one_int\n");
  char *sample = "<meaning>42</meaning>";
  typedef struct {
    int meaning;
  } one_int;

  make_cerial(one_int, cerial_option_no_root,
    cerial_access(one_int, meaning, cerial_int)
  );

  one_int test_object;
  test_object.meaning = 0xcc;

  size_t bytes = cerial_xml_read(&one_int_cerial, &test_object, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_object.meaning == 42);
}

void test_root(void)
{
  printf("test_root\n");
  char *sample = "<one_int>\n"
                 "  <meaning>42</meaning>\n"
                 "</one_int>\n";
  typedef struct {
    int meaning;
  } one_int;

  make_cerial(one_int, 0,
    cerial_access(one_int, meaning, cerial_int)
  );

  one_int test_object;
  test_object.meaning = 0xcc;

  size_t bytes = cerial_xml_read(&one_int_cerial, &test_object, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_object.meaning == 42);
}

void test_two_int(void)
{
  printf("test_two_int\n");
  char *sample = "<car_shop>\n"
                 "  <broken_tires>21</broken_tires>\n"
                 "  <fixed_tires>6</fixed_tires>\n"
                 "</car_shop>\n";
  typedef struct {
    int broken_tires;
    int fixed_tires;
  } car_shop;

  make_cerial(car_shop, 0,
    cerial_access(car_shop, broken_tires, cerial_int),
    cerial_access(car_shop, fixed_tires, cerial_int)
  );

  car_shop my_shop;
  my_shop.broken_tires = 0xcc;
  my_shop.fixed_tires = 0xcc;

  size_t bytes = cerial_xml_read(&car_shop_cerial, &my_shop, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(my_shop.broken_tires == 21);
  assert(my_shop.fixed_tires == 6);
}

void test_string(void)
{
  printf("test_string\n");
  char *sample = "<badge>\n"
                 "  <serial>x609</serial>\n"
                 "</badge>\n";
  typedef struct {
    char serial[12];
  } badge;

  make_cerial(badge, 0,
    cerial_access(badge, serial, cerial_str, {12})
  );

  badge the_badge;
  strcpy(the_badge.serial, "cccccc");

  size_t bytes = cerial_xml_read(&badge_cerial, &the_badge, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(strcmp(the_badge.serial, "x609") == 0);
}

void test_bool(void)
{
  printf("test_bool\n");
  char *sample = "<table>\n"
                 "  <has_legs>true</has_legs>\n"
                 "</table>\n";
  typedef struct {
    bool has_legs;
  } table;

  make_cerial(table, 0,
    cerial_access(table, has_legs, cerial_bool)
  );

  table dining_room_table = {0};

  size_t bytes = cerial_xml_read(&table_cerial, &dining_room_table, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(dining_room_table.has_legs == true);
}

void test_object(void)
{
  printf("test_object\n");
  char *sample = "<table>\n"
                 "  <first_leg>\n"
                 "    <is_broken>false</is_broken>\n"
                 "    <round>true</round>\n"
                 "  </first_leg>\n"
                 "  <second_leg>\n"
                 "    <is_broken>true</is_broken>\n"
                 "    <round>false</round>\n"
                 "  </second_leg>\n"
                 "</table>\n";
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
  dining_room_table.first_leg.is_broken = true;
  dining_room_table.first_leg.round = false;
  dining_room_table.second_leg.is_broken = false;
  dining_room_table.second_leg.round = true;


  size_t bytes = cerial_xml_read(&table_cerial, &dining_room_table, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(dining_room_table.first_leg.is_broken == false);
  assert(dining_room_table.first_leg.round == true);
  assert(dining_room_table.second_leg.is_broken == true);
  assert(dining_room_table.second_leg.round == false);
}

void test_array(void)
{
  printf("test_array\n");
  char *sample = "<table>\n"
                 "  <leg_colours>\n"
                 "    <value>0xffee55</value>\n"
                 "    <value>0x0055aa</value>\n"
                 "    <value>0x335599</value>\n"
                 "    <value>0x3498a7</value>\n"
                 "  </leg_colours>\n"
                 "</table>\n";

  typedef struct {
    int leg_colours[4];
  } table;

  make_cerial(table, 0,
    cerial_access(table, leg_colours, cerial_int, {4}, true),
  );

  table dining_room_table;
  dining_room_table.leg_colours[0] = 0xcccccc;
  dining_room_table.leg_colours[1] = 0xcccccc;
  dining_room_table.leg_colours[2] = 0xcccccc;
  dining_room_table.leg_colours[3] = 0xcccccc;

  size_t bytes = cerial_xml_read(&table_cerial, &dining_room_table, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(dining_room_table.leg_colours[0] == 0xffee55);
  assert(dining_room_table.leg_colours[1] == 0x0055aa);
  assert(dining_room_table.leg_colours[2] == 0x335599);
  assert(dining_room_table.leg_colours[3] == 0x3498a7);
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