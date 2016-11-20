
#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...) false
#endif

#define cerial_assert(condition) ((condition) ? true : (debug_printf("%s:%d Failed assertion: %s\n", __FILE__, __LINE__, #condition), false))
size_t cerial_accessor_size(cerial_accessor accessor);
