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

Next call cerial_json_read with fish_pond_cerial and a string buffer.
```
char *json_input = "{\number_of_fish_in_pond\": 6, \"average_fish_weight\": 63.12}"
fish_pond lake_pleasant = {0};
cerial_json_read(&fish_pond_cerial, &lake_pleasant, json_input, strlen(json_input));
// assert(lake_pleasant.number_of_fish_in_pond == 6);
// assert(lake_pleasant.average_fish_weight == 63.12);
```

## JSON Support

This library has full, to-spec support for JSON.

## XML Support

XML support isn't complete. It is capable of writing all the same data types as the JSON side but is not a complete to-spec XML reader. It should read your XML schema if you're using it as a dictionary.

### XML Booleans

Booleans are `true` or `false`. Uppercase forms are not supported.

### XML Arrays

Arrays have a single node with the member name and many `value` nodes with the array contents.

```
<leg_colours>
  <value>0xffee55</value>
  <value>0x0055aa</value>
  <value>0x335599</value>
  <value>0x3498a7</value>
</leg_colours>
```

### Values in tags

Values or other keys embedded in a tag (`<colour value="red"/>`) are not currently supported.

## X.690 Support

X.690 support isn't complete. It's missing support for some string types, bitstring/data type and non-universal (custom) tags.

### Strings
Character strings (char[]) are always written out as VisibleString. They can be read in as UTF8String, IA5String or VisibleString. All other string types are not supported.

### Floating Point
This library has full support for binary, base-16 floats. Decimal and non-base-16 floats are not supported.


## Roadmap
 - Raw Data (bitstring) type
 - X690/ASN1 custom tags
 - Serialize directly from/to a file/stream
 - More-complete XML Support
