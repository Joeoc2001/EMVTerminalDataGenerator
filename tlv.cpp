#include "tlv.h"
#include "hex.h"

TLV* TLV::parse_data(uint8_t* data, size_t len) {
  size_t pos = 0;
  return new TLV(data, len, &pos);
}

TLV::TLV(uint8_t* data, size_t len, size_t* pos) {
  // TLV data may be TLV encoded
  bool is_tlv_encoded = (data[*pos] & 0b00100000);
  
  // Tag may be one or many bytes
  this->tag = &(data[*pos]);
  this->tag_len = 1;
  if ((data[*pos] & 0b00011111) == 0b00011111) {
    this->tag_len += 1;
    while (data[*pos + this->tag_len - 1] & 0b10000000) {
      this->tag_len += 1;
    }
  }

  // Look at len field
  *pos += this->tag_len;

  // Get len
  size_t data_len = (size_t)data[*pos];
  *pos += 1;
  if (data_len > 0x80) {
    uint8_t len_bytes = data_len & 0b00000111;

    data_len = 0;
    for (uint8_t i = 0; i < len_bytes; i++) {
      data_len <<= 8;
      data_len |= data[*pos];
      
      *pos += 1;
    }
  }
  this->data_len = data_len;

  // Get value
  this->data = &(data[*pos]);
  *pos += data_len;

  // Decode children if needed
  this->first_child = nullptr;
  if (is_tlv_encoded) {
    TLV* child = TLV::parse_data(this->data, this->data_len);
    this->first_child = child;
  }

  // Decode next sibling
  this->next_sibling = nullptr;
  if (*pos < len) {
    TLV* sibling = new TLV(data, len, pos);
    this->next_sibling = sibling;
  }
}

TLV::~TLV() {
  if (this->first_child != nullptr) {
    delete this->first_child;
  }
  if (this->next_sibling != nullptr) {
    delete this->next_sibling;
  }
}

void TLV::print_hex_to_serial(uint8_t indents) {
  for (uint8_t i = 0; i < indents; i++) {
    Serial.print("  ");
  }
  for (uint8_t i = 0; i < this->tag_len; i++) {
    print_hex_byte(this->tag[i]);
  }
  
  Serial.print(": ");
  if (this->data_len <= 0xFF) {
    print_hex_byte(this->data_len);
  } else {
    Serial.print(this->data_len, HEX);
  }
  Serial.println();

  if (this->first_child == nullptr) { // Need to print data as is
    for (uint8_t i = 0; i < indents + 1; i++) {
      Serial.print("  ");
    }
    for (uint8_t i = 0; i < this->data_len; i++) {
      print_hex_byte(this->data[i]);
    }
    Serial.println();
  } else {
    this->first_child->print_hex_to_serial(indents + 1);
  }

  if (this->next_sibling != nullptr) {
    this->next_sibling->print_hex_to_serial(indents);
  }
}

void TLV::print_hex_to_serial() {
  this->print_hex_to_serial(0);
}
