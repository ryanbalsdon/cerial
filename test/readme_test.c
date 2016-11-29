#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "cerial.h"


typedef struct {
  int number_of_fish_in_pond;
  float average_fish_weight;
} fish_pond;

make_cerial(fish_pond, cerial_option_no_root,
  cerial_access(fish_pond, number_of_fish_in_pond, cerial_int),
  cerial_access(fish_pond, average_fish_weight, cerial_float)
);

void test_example_json(void)
{
  printf("test_example_json\n");

  fish_pond lake_pleasant = {0};
  char *input = "{\"number_of_fish_in_pond\": 42, \"average_fish_weight\": -4.200000}";
  size_t bytes_read = cerial_json_read(&fish_pond_cerial, &lake_pleasant, input, strlen(input));
  assert(bytes_read == strlen(input));
  assert(lake_pleasant.number_of_fish_in_pond == 42);
  assert(lake_pleasant.average_fish_weight > -4.3 && lake_pleasant.average_fish_weight < -4.1);

  char output[1024];
  size_t bytes_written = cerial_json_write(&fish_pond_cerial, &lake_pleasant, output, sizeof(output));
  assert(bytes_written == bytes_read);
  assert(memcmp(output, input, bytes_written) == 0);
}

void test_example_xml(void)
{
  printf("test_example_xml\n");

  fish_pond lake_pleasant = {0};
  char *input = "<number_of_fish_in_pond>42</number_of_fish_in_pond><average_fish_weight>-4.200000</average_fish_weight>";
  size_t bytes_read = cerial_xml_read(&fish_pond_cerial, &lake_pleasant, input, strlen(input));
  assert(bytes_read == strlen(input));
  assert(lake_pleasant.number_of_fish_in_pond == 42);
  assert(lake_pleasant.average_fish_weight > -4.3 && lake_pleasant.average_fish_weight < -4.1);

  char output[1024];
  size_t bytes_written = cerial_xml_write(&fish_pond_cerial, &lake_pleasant, output, sizeof(output));
  assert(bytes_written == bytes_read);
  assert(memcmp(output, input, bytes_written) == 0);
}

void test_example_x690(void)
{
  printf("test_example_x690\n");

  fish_pond lake_pleasant = {0};
  uint8_t input[] = {0x02, 0x01, 0x2A, 0x09, 0x06, 0xE0, 0xE6, 0x10, 0xCC, 0xCC, 0xC0};
  size_t bytes_read = cerial_x690_read(&fish_pond_cerial, &lake_pleasant, input, sizeof(input));
  assert(bytes_read == sizeof(input));
  assert(lake_pleasant.number_of_fish_in_pond == 42);
  assert(lake_pleasant.average_fish_weight > -4.3 && lake_pleasant.average_fish_weight < -4.1);

  uint8_t output[1024];
  size_t bytes_written = cerial_x690_write(&fish_pond_cerial, &lake_pleasant, output, sizeof(output));
  assert(bytes_written == bytes_read);
  assert(memcmp(output, input, bytes_written) == 0);
}

int main(void)
{
  test_example_json();
  test_example_xml();
  test_example_x690();

  printf("pass\n");
}