#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "cerial.h"



void test_one_int(void)
{
  printf("test_one_int\n");
  char *sample = " { \"meaning\": 42 } ";
  typedef struct {
    int meaning;
  } one_int_struct;

  make_cerial(one_int_struct, cerial_option_no_root,
    cerial_access(one_int_struct, meaning, cerial_int)
  );

  one_int_struct test_struct;
  test_struct.meaning = 0xcc;

  size_t bytes = cerial_json_read(&one_int_struct_cerial, &test_struct, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_struct.meaning == 42);
}

void test_root(void)
{
  printf("test_root\n");
  char *sample = "{\"root\":{\"evil\": \"money\"} }";
  typedef struct {
    char evil[12];
  } root;

  make_cerial(root, 0,
    cerial_access(root, evil, cerial_str, {12})
  );

  root test;
  strcpy(test.evil, "ccccccccc");

  size_t bytes = cerial_json_read(&root_cerial, &test, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(strcmp(test.evil, "money") == 0);
}

void test_two_int(void)
{
  printf("test_two_int\n");
  char *sample = "{\"first\":16, \"second\": 1024}";

  typedef struct {
    int first;
    int second;
  } two_int_struct;

  make_cerial(two_int_struct, cerial_option_no_root,
    cerial_access(two_int_struct, first, cerial_int),
    cerial_access(two_int_struct, second, cerial_int)
  );

  two_int_struct test_struct;
  test_struct.first  = 0xcc;
  test_struct.second = 0xcc;

  size_t bytes = cerial_json_read(&two_int_struct_cerial, &test_struct, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_struct.first == 16);
  assert(test_struct.second == 1024);
}

void test_string(void)
{
  printf("test_string\n");
  char *sample = " { \"food\": \"pizza\" } ";
  typedef struct {
    char food[12];
  } foods_struct;

  make_cerial(foods_struct, cerial_option_no_root,
    cerial_access(foods_struct, food, cerial_str, {12})
  );

  foods_struct test_struct;

  size_t bytes = cerial_json_read(&foods_struct_cerial, &test_struct, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(strcmp(test_struct.food, "pizza") == 0);
}

void test_bool(void)
{
  printf("test_bool\n");
  char *sample = "{ \"truthy\": true, \"falsehood\": false }";
  typedef struct {
    bool truthy;
    bool falsehood;
  } thruthiness_struct;

  make_cerial(thruthiness_struct, cerial_option_no_root,
    cerial_access(thruthiness_struct, truthy, cerial_bool),
    cerial_access(thruthiness_struct, falsehood, cerial_bool)
  );

  thruthiness_struct test_struct;

  size_t bytes = cerial_json_read(&thruthiness_struct_cerial, &test_struct, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_struct.truthy);
  assert(!test_struct.falsehood);
}

void test_object(void)
{
  printf("test_object\n");
  char *sample = "{ \"me\": { \"is_awesome\": true }, \"noone\": {\"is_awesome\": false}}";
  typedef struct {
    bool is_awesome;
  } person;

  typedef struct {
    person me;
    person noone;
  } people;

  make_cerial(person, cerial_option_no_root,
    cerial_access(person, is_awesome, cerial_bool)
  );

  make_cerial(people, cerial_option_no_root,
    cerial_access(people, me, cerial_object, {.super_cerial=&person_cerial}),
    cerial_access(people, noone, cerial_object, {.super_cerial=&person_cerial})
  );

  people test_people;

  size_t bytes = cerial_json_read(&people_cerial, &test_people, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(test_people.me.is_awesome);
  assert(!test_people.noone.is_awesome);
}

void test_array(void)
{
  printf("test_array\n");
  char *sample = "{ \"numbers\": [22 , 64 ] }";
  typedef struct {
    int numbers[2];
  } array;

  make_cerial(array, cerial_option_no_root,
    cerial_access(array, numbers, cerial_int, {2}, true)
  );

  array array_test;

  size_t bytes = cerial_json_read(&array_cerial, &array_test, sample, strlen(sample));
  assert(bytes == strlen(sample));
  assert(array_test.numbers[0] == 22);
  assert(array_test.numbers[1] == 64);
}

int main(void)
{
  test_one_int();
  test_root();
  test_two_int();
  test_string();
  test_bool();
  test_array();
  test_object();
  printf("pass\n");
}