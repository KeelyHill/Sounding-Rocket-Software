
#include <SPI.h>
#include <RH_RF95.h>
#include <RHGenericDriver.h> // for enum RHMode

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 915.0

void commonRadioSetup() {

	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);

	// manual radio reset TODO test if needed
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);
}

bool radioInit(RH_RF95 &rf95) {
	if (!rf95.init()) {
		// digitalWrite(RFM95_CS, LOW);
		Serial.println("LoRa radio init failed!!");

		// TODO see what happens in a failed case

		return false;
	}
	if (DEBUG) Serial.println("LoRa radio init OK!");

	if (!rf95.setFrequency(RF95_FREQ)) {
		Serial.println("setFrequency failed");

		// TODO see what happens in a failed case

		return false;
	}
	if (DEBUG) Serial.print("Freq set to: "); Serial.println(RF95_FREQ);

	// can set transmitter powers from 5 to 23 dBm:
	rf95.setTxPower(23, false);

	// rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128); // Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
	// rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512); // Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range.
	rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096); // Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+very_long range.

	if (DEBUG) Serial.println("LoRa radio READY.");

	return true;
}

/** Print bytes as their hex representation (in ASCII) */
void printlnRawBytes(uint8_t *bytes, size_t* len) {
	for (size_t i=0; i<*len; i++) {
		Serial.print("\\x");
		Serial.print(bytes[i] < 16 ? "0" : "");
		Serial.print(bytes[i], HEX);
	}
	Serial.println("\n");
}
