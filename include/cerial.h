#ifndef CERIAL_H
#define CERIAL_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  cerial_int,
  cerial_float,
  cerial_double,
  cerial_str,
  cerial_bool,
  cerial_object
} cerial_type;

typedef enum {
  cerial_option_none = 0,
  cerial_option_no_root = 0x1
} cerial_options;

typedef enum {
  cerial_accessor_option_none = 0,
  cerial_accessor_option_array = 0x1,
  cerial_accessor_option_pointer = 0x2
} cerial_accessor_options;

typedef struct cerial_accessor cerial_accessor;
typedef struct cerial cerial;

struct cerial_accessor {
  char *name;
  size_t offset;
  cerial_type type;
  union {
    size_t buffer_size; //used for array length and string size
    cerial *super_cerial; //used for sub-objects
  };
  cerial_accessor_options is_array;
};

struct cerial {
  char *name;
  int count; //number of accessors
  cerial_accessor *accessors;
  size_t size; //size of destination struct
  cerial_options options;
};

#define cerial_access(struct, member, type...) { #member, offsetof(struct, member), type }

#define make_cerial(name, options, ...) \
  cerial_accessor name##_cerial_accessors[] = {__VA_ARGS__};\
  cerial name##_cerial = {\
    #name,\
    sizeof(name##_cerial_accessors)/sizeof(cerial_accessor),\
    name##_cerial_accessors,\
    sizeof(name),\
    options\
  };

size_t cerial_json_read(cerial *self, void *output, const char *string, size_t size);
size_t cerial_json_write(cerial *self, const void *object, char *output, size_t size);
size_t cerial_xml_read(cerial *self, void *output, const char *string, size_t size);
size_t cerial_xml_write(cerial *self, const void *object, char *output, size_t size);
size_t cerial_x690_read(cerial *self, void *output, const uint8_t *bytes, size_t size);
size_t cerial_x690_write(cerial *self, const void *object, uint8_t *output, size_t size);

#endif /* CERIAL_H */