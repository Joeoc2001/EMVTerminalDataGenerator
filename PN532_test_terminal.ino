#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
#include "response.h"
#include "tlv.h"
#include "tags.h"

SoftwareSerial SWSerial( 10, 11 ); // RX, TX

PN532_SWHSU pn532swhsu( SWSerial );

PN532 nfc( pn532swhsu );


void setup() {
  Serial.begin(250000);

  nfc.begin();

  uint32_t version_data = nfc.getFirmwareVersion();
  if (!version_data) {
    while (1) {}
  }

  // From PN532 examples - https://github.com/Seeed-Studio/PN532/blob/arduino/PN532/examples/android_hce/android_hce.ino
  Serial.print("Found chip PN5"); Serial.println((version_data>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((version_data>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((version_data>>8) & 0xFF, DEC);

  // Configure board for ISO 14443
  nfc.SAMConfig();

  Serial.println("Ready");
}

void loop() {
  bool success = nfc.inListPassiveTarget();

  if (!success)
  {
    return;
  }

  Serial.println("Found target");

  uint8_t selectPPSE[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, '2', 'P', 'A', 'Y', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1', 0x00};

  Response resp = Response(nfc, selectPPSE, sizeof(selectPPSE));

  if (!resp.is_success())
  {
    Serial.println("Failed sending SELECT PPSE");
    resp.print_success_status_to_serial();
    return;
  }

  // Extract supported AID
  uint8_t aid[32];
  size_t aid_len = sizeof(aid);
  uint8_t* aid_select_tags[]    = {TAG_6F, TAG_A5, TAG_BF0C, TAG_61, TAG_4F};
  uint8_t aid_select_tag_lens[] = {sizeof(TAG_6F), sizeof(TAG_A5), sizeof(TAG_BF0C), sizeof(TAG_61), sizeof(TAG_4F)};
  if (!resp.find_value(aid_select_tags, aid_select_tag_lens, sizeof(aid_select_tag_lens), aid, &aid_len))
  {
    Serial.println("Failed to extract AID");
    return;
  }

  // Build select command
  size_t selectAIDLen = 6 + aid_len;
  uint8_t selectAIDPreamble[] = {0x00, 0xA4, 0x04, 0x00};
  uint8_t* selectAID = new uint8_t[selectAIDLen];
  memcpy(selectAID, selectAIDPreamble, sizeof(selectAIDPreamble));
  selectAID[sizeof(selectAIDPreamble)] = aid_len;
  uint8_t* aidloc = &(selectAID[sizeof(selectAIDPreamble) + 1]);
  memcpy(aidloc, aid, aid_len);
  selectAID[sizeof(selectAID) - 1] = 0x00;

  // Transmit
  resp = Response(nfc, selectAID, selectAIDLen);

  // Clean up
  delete[] selectAID;

  // Handle response to select
  resp.print_hex_to_serial();
  if (!resp.is_success())
  {
    Serial.println("Failed sending SELECT AID");
    resp.print_success_status_to_serial();
    return;
  }
  
  resp.print_hex_to_serial();

  uint8_t gpo[] = {0x80, 0xA8, 0x00, 0x00, 0x23, 0x83, 0x21, 0x36, 0xA0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x26, 0x22, 0x03, 0x09, 0x00, 0x97, 0x82, 0x2D, 0xA6, 0x00};

  resp = Response(nfc, gpo, sizeof(gpo));

  if (!resp.is_success())
  {
    Serial.println("Failed sending GPO");
    return;
  }

  resp.print_hex_to_serial();  
}
