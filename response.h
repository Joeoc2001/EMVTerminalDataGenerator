#ifndef responseh
#define responseh "response"

#include "tlv.h"

#include <stdint.h>
#include "Arduino.h"
#include <PN532.h>

/* 
 *  A parsed response, if we got one.
 *  Split into a seperate class because not all responses will be parsable (e.g. a timeout)
 */
class ParsedResponse {
private:
  int response_code;
  TLV* tlv; // Owned

public:
  ParsedResponse(uint8_t* response, size_t response_len);
  ~ParsedResponse();

  bool is_success();
  void print_success_status_to_serial();
  void print_hex_to_serial();
};

/*
 *  Represents an interaction with the PICC, and contains the response, parsed if possible.
 *  Owns the data that the PICC returns.
 */
class Response {
private:
  bool transmit_success;
  uint8_t* data; // Owned by this object
  uint8_t len;

  ParsedResponse* parsed_response; // Owned, may be nullptr

public:
  // Sends a command to the PICC and parses the response
  Response(PN532 &nfc, uint8_t* command, size_t cmd_len);
  ~Response();

  bool is_success();
  
  void print_success_status_to_serial();
  void print_hex_to_serial();
};

#endif
