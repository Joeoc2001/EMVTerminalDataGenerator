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

bool TLV::tag_matches(uint8_t* tag, uint8_t tag_len) {
  if (tag_len != this->tag_len) {
    return false;
  }

  for (uint8_t i = 0; i < tag_len; i++) {
    if (this->tag[i] != tag[i]) {
      return false;
    }
  }

  return true;
}

bool TLV::find_value(uint8_t** tags, uint8_t* tag_lens, size_t tags_len, uint8_t* value, size_t* value_len) {
  if (tags_len == 0) {
    return false;
  }

  uint8_t tag_len = tag_lens[0];
  uint8_t* tag = tags[0];
  if (this->tag_matches(tag, tag_len)) {
    if (tags_len == 1) { // We want this value
      if (*value_len < this->data_len) {
        return false;
      }
      memcpy(value, this->data, this->data_len);
      *value_len = this->data_len;
      return true;
    }

    // Else we want a child value
    if (this->first_child == nullptr) {
      return false;
    }
    uint8_t** child_tags = &(tags[1]);
    uint8_t* child_tag_lens = &(tag_lens[1]);
    return this->first_child->find_value(child_tags, child_tag_lens, tags_len - 1, value, value_len);
  }

  // Else it's in a sibling
  if (this->next_sibling == nullptr) {
    return false;
  }
  return this->next_sibling->find_value(tags, tag_lens, tags_len, value, value_len);
}
