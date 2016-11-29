# Cerial
A serialization library for C.

The goals for this project were two: use zero calls to malloc and make all memory access sequential for cache-friendly behaviour. 

Malloc: Sucess! There are no calls to malloc.

Sequential-Access: Partial :( JSON writes and X.690 reads are perfectly sequential. The others all jump around a bit.

## Usage
Make your struct as normal:
```
typedef struct {
  int number_of_fish_in_pond;
  float average_fish_weight;
} fish_pond;
```

Then make cereal! ... Then make_cerial!
```
  make_cerial(fish_pond, cerial_option_no_root,
    cerial_access(fish_pond, number_of_fish_in_pond, cerial_int),
    cerial_access(fish_pond, average_fish_weight, cerial_float)
  );
```


This will create a new `cerial` instance called `fish_pond_cerial` that holds all the information needed to serialize your struct.

### JSON Usage Example

```
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
```

### XML Usage Example

```
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
```

### X.690 Usage Example

```
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
```

## More Examples

Look in the test files in the /test folder for even more usage examples!

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

X.690 support isn't complete. It's missing support for some string types, the bitstring type and non-universal (application-specific) tags.

### Strings
Character strings (char[]) are always written out as VisibleString. They can be read in as UTF8String, IA5String or VisibleString. All other string types are not supported.

### Floating Point
This library has full support for binary, base-16 floats. Decimal and non-base-16 floats are not supported.

## Error-handling
On any error, a zero length is returned. Compile with `DEBUG` to print the error to STDOUT.

## Installing
This project is meant to be included as source in your build but running `make` will build a linkable library as `/objects/libcerial.a`.

The source files in `/src` share no code and can be individually included in your project. For example, if you only need to read JSON and write X690, include only cerial_json_read.c and cerial_x690_write.c.

## Testing
All source files in the `/test` folder are intended to build a standalone executable. If the files finishes running successfully, all the tests in it have passed. Use `make test` as a shortcut to build and run all tests.

## Contributing
Fork this repo, make your change, add any relevant tests then open a Pull Request. The tests can be run with `make test` or `make valgrind` if Valgrind is installed.

## Roadmap
 - Raw Data (bitstring) type
 - X690/ASN1 custom tags
 - Serialize directly from/to a file/stream/socket
 - More-complete XML Support
