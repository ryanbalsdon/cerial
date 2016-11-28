
#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...) false
#endif

#define cerial_assert(condition) ((condition) ? true : (debug_printf("%s:%d Failed assertion: %s\n", __FILE__, __LINE__, #condition), false))
#define cerial_accessor_size(accessor) (\
  accessor.type == cerial_int    ? sizeof(int) :\
  accessor.type == cerial_float  ? sizeof(float) :\
  accessor.type == cerial_double ? sizeof(double) :\
  accessor.type == cerial_str    ? accessor.buffer_size :\
  accessor.type == cerial_bool   ? sizeof(bool) :\
  accessor.type == cerial_object ? accessor.super_cerial->size :\
  0\
)
