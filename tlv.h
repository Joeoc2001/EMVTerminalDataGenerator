#ifndef tlvh
#define tlvh "tlv"

#include "Arduino.h"

/*
 * This class represents a sort of annotation of data received from the PICC.
 * The data pointer points to data not owned by this class (for speed, to avoid copying) so care
 * must be taken to ensure that the data pointed to by this class is not modified while objects
 * of this class still exist.
 */
class TLV {
private:
  uint8_t* tag; // Not owned
  uint8_t tag_len;
  uint8_t* data; // Not owned
  size_t data_len;
  TLV* first_child; // Owned, may be nullptr
  TLV* next_sibling; // Owned, may be nullptr

  // Constructs with a rolling pointer into the data
  TLV(uint8_t* data, size_t len, size_t* pos);

  // Recursive indent tracking
  void print_hex_to_serial(uint8_t indents);
public:
  // Calls the private constructor
  static TLV* parse_data(uint8_t* data, size_t len);
  ~TLV();

  // Calls private recursive method
  void print_hex_to_serial();

  bool tag_matches(uint8_t* tag, uint8_t tag_len);
  bool find_value(uint8_t** tags, uint8_t* tag_lens, size_t tags_len, uint8_t* value, size_t* value_len);
};

#endif
