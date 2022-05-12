#include "hex.h"

void print_hex_byte(uint8_t b) {
  if (b < 0x10) {
    Serial.print("0");
  }
  Serial.print(b, HEX);
  Serial.print(" ");
}
