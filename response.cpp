#include "response.h"
#include "constants.h"
#include "hex.h"

Response::Response(PN532& nfc, uint8_t* command, size_t cmd_len) {
  this->data = new uint8_t[BUFFER_SIZE];
  this->len = BUFFER_SIZE;
  
  this->transmit_success = nfc.inDataExchange(command, cmd_len, this->data, &this->len);

  this->parsed_response = nullptr;
  if (!this->transmit_success) {
    this->len = 0;
    return;
  }

  // Parse data
  this->parsed_response = new ParsedResponse(this->data, this->len);
}

Response::~Response() {
  delete[] this->data;

  if (this->parsed_response != nullptr) {
    delete this->parsed_response;
  }
  
  Serial.println("Clean up Response");
}

bool Response::is_success() {
  if (!this->transmit_success) {
    return false;
  }

  if (this->parsed_response == nullptr) {
    return false;
  }

  return this->parsed_response->is_success();
}

void Response::print_hex_to_serial() {
  if (this->parsed_response != nullptr) {
    this->parsed_response->print_hex_to_serial();
    return;
  }
  
  for (size_t i = 0; i < this->len; i++) {
    print_hex_byte(this->data[i]);
  }
  Serial.println();
}

void Response::print_success_status_to_serial() {
  if (!this->transmit_success) {
    Serial.println("Failed to transcieve");
    return;
  } 

  if (this->parsed_response == nullptr) {
    Serial.println("Failed to parse");
    return;
  }

  this->parsed_response->print_success_status_to_serial();
}

bool Response::find_value(uint8_t** tags, uint8_t* tag_lens, size_t tags_len, uint8_t* value, size_t* value_len) {
  if (this->parsed_response == nullptr) {
    return false;
  }

  return this->parsed_response->find_value(tags, tag_lens, tags_len, value, value_len);
}

ParsedResponse::ParsedResponse(uint8_t* response, size_t response_len) {
  this->tlv = nullptr;
  if (response_len > 2) {
    this->tlv = TLV::parse_data(response, response_len - 2);
  }

  this->response_code = ((int)response[response_len - 2] << 8) | response[response_len - 1];
  this->response_code &= 0x0000FFFF;
}

ParsedResponse::~ParsedResponse() {
  if (this->tlv != nullptr) {
    delete this->tlv;
  }
}

bool ParsedResponse::is_success() {
  return (this->response_code == 0x9000);
}

void ParsedResponse::print_success_status_to_serial() {
    Serial.print("Response code: 0x");
    Serial.print(this->response_code, HEX);
    if (this->is_success()) {
      Serial.println("(SUCCESS)");
    } else {
      Serial.println("(FAILURE)");
    }
}

void ParsedResponse::print_hex_to_serial() {
  if (this->tlv != nullptr) {
    this->tlv->print_hex_to_serial();
  }
  
  Serial.println(this->response_code, HEX);
}

bool ParsedResponse::find_value(uint8_t** tags, uint8_t* tag_lens, size_t tags_len, uint8_t* value, size_t* value_len) {
  if (this->tlv == nullptr) {
    return false;
  }
  
  return this->tlv->find_value(tags, tag_lens, tags_len, value, value_len);
}
