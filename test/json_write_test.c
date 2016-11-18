#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "cerial.h"



void test_one_int(void)
{
  char output[1024];

  typedef struct {
    int meaning;
  } one_int;
  make_cerial(one_int, cerial_option_no_root,
    cerial_access(one_int, meaning, cerial_int)
  );

  one_int test_object;
  test_object.meaning = 42;
  char *expected = "{\"meaning\": 42}";

  size_t bytes = cerial_write_json(&one_int_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_root(void)
{
  char output[1024];
  char *expected = "{\"root\": {\"evil\": \"money\"}}";
  typedef struct {
    char evil[12];
  } root;

  make_cerial(root, 0,
    cerial_access(root, evil, cerial_str, {12})
  );

  root test;
  strcpy(test.evil, "money");

  size_t bytes = cerial_write_json(&root_cerial, &test, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_two_int(void)
{
  char output[1024];

  typedef struct {
    int first;
    int second;
  } two_int;
  make_cerial(two_int, cerial_option_no_root,
    cerial_access(two_int, first, cerial_int),
    cerial_access(two_int, second, cerial_int)
  );

  two_int test_object;
  test_object.first = 16;
  test_object.second = 1024;
  char *expected = "{\"first\": 16, \"second\": 1024}";

  size_t bytes = cerial_write_json(&two_int_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_string(void)
{
  char output[1024];

  typedef struct {
    char stuff[20];
  } string;
  make_cerial(string, cerial_option_no_root,
    cerial_access(string, stuff, cerial_str, {20})
  );

  string test_object;
  strcpy(test_object.stuff, "testor");
  char *expected = "{\"stuff\": \"testor\"}";

  size_t bytes = cerial_write_json(&string_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_bool(void)
{
  char output[1024];

  typedef struct {
    bool truthy;
    bool falsehood;
  } thruthiness_struct;

  make_cerial(thruthiness_struct, cerial_option_no_root,
    cerial_access(thruthiness_struct, truthy, cerial_bool),
    cerial_access(thruthiness_struct, falsehood, cerial_bool)
  );

  thruthiness_struct test_object;
  test_object.truthy = true;
  test_object.falsehood = false;

  char *expected = "{\"truthy\": true, \"falsehood\": false}";

  size_t bytes = cerial_write_json(&thruthiness_struct_cerial, &test_object, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_object(void)
{
  char output[1024];
  char *expected = "{\"me\": {\"is_awesome\": true}, \"noone\": {\"is_awesome\": false}}";

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
  test_people.me.is_awesome = true;
  test_people.noone.is_awesome = false;

  size_t bytes = cerial_write_json(&people_cerial, &test_people, output, 1024);
  assert(bytes == strlen(expected));
  assert(strcmp(expected, output) == 0);
}

void test_array(void)
{
  char output[1024];
  char *expected = "{\"numbers\": [22, 64]}";
  typedef struct {
    int numbers[2];
  } array;

  make_cerial(array, cerial_option_no_root,
    cerial_access(array, numbers, cerial_int, {2}, true)
  );

  array array_test;
  array_test.numbers[0] = 22;
  array_test.numbers[1] = 64;

  size_t bytes = cerial_write_json(&array_cerial, &array_test, output, 1024);
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