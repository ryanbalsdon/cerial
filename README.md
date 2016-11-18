# Cerial
A serialization library for C.

## Usage
Make your struct as normal:
```
typedef struct {
  int number_of_fish_in_pond;
  float average_fish_weight;
} fish_pond;
```

Then make cereal! Obviously, that's Cinnamon Toast Crunch with banana slices.
Then make_cerial!
```
  make_cerial(fish_pond, cerial_option_no_root,
    cerial_access(fish_pond, number_of_fish_in_pond, cerial_int),
    cerial_access(fish_pond, average_fish_weight, cerial_float)
  );
```

This will create a new cerial instance called fish_pond_cerial.

Next call either cerial_read_json or cerial_write_json with fish_pond_cerial and a string buffer.
```
char *json_input = "{\number_of_fish_in_pond\": 6, \"average_fish_weight\": 63.12}"
fish_pond lake_pleasant = {0};
cerial_read_json(&fish_pond_cerial, &lake_pleasant, json_input, strlen(json_input));
// assert(lake_pleasant.number_of_fish_in_pond == 6);
// assert(lake_pleasant.average_fish_weight == 63.12);
```

## Roadmap
 - Serialize directly from/to a file/stream
 - XML Support
 - X690/ASN1 support
