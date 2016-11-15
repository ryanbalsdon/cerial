#include <stddef.h>
#include <stdbool.h>

  typedef enum {
    cerial_int,
    cerial_float,
    cerial_double,
    cerial_str,
    cerial_bool,
    cerial_object
  } cerial_type;

  typedef struct cerial_accessor cerial_accessor;
  typedef struct cerial cerial;

  struct cerial_accessor {
    char *name;
    size_t offset;
    cerial_type type;
    union {
      size_t buffer_size;
      cerial *super_cerial;
    };
    bool is_array;
  };

  struct cerial {
    int count;
    cerial_accessor *accessors;
    size_t size;
  };

  #define c_access(struct, member, type...) { #member, offsetof(struct, member), type }

  #define make_cerial(name, ...) \
    cerial_accessor name##_cerial_accessors[] = {__VA_ARGS__};\
    cerial name##_cerial = {\
      sizeof(name##_cerial_accessors)/sizeof(cerial_accessor),\
      name##_cerial_accessors,\
      sizeof(name)\
    };

  size_t cerial_read_json(cerial *self, void *output, const char *string, size_t size);
  size_t cerial_write_json(cerial *self, const void *object, char *output, size_t size);
  size_t cerial_accessor_size(cerial_accessor accessor);
