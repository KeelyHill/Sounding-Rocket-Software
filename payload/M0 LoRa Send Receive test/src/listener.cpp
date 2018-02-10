/** Base station */

#include "compile_picker.cpp"

#if DOLISTENER == true

#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);

	while (!Serial);
	Serial.begin(9600);
	delay(100);

	Serial.println("Feather LoRa RX Test!");

	// manual reset
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);

	while (!rf95.init()) {
		Serial.println("LoRa radio init failed");
		while (1);
	}
	Serial.println("LoRa radio init OK!");

	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
	if (!rf95.setFrequency(RF95_FREQ)) {
		Serial.println("setFrequency failed");
		while (1);
	}
	Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

	// Defaults after init are 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	rf95.setTxPower(23, false);
}

unsigned long time_at_last_recv = 0;
unsigned long delta_time = 0;

void loop() {
	if (rf95.available()) {
	    // is a new message available?

		uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	    uint8_t len = sizeof(buf);

	    if (rf95.recv(buf, &len)) {
			// len NOT necessarily RH_RF95_MAX_MESSAGE_LEN

			#if CSV_NOT_HUMAN==true
				delta_time = millis() - time_at_last_recv;
				time_at_last_recv = millis();

				Serial.print((char*)buf); // expecting a csv line
				Serial.print(",");
				Serial.print(rf95.lastRssi(), DEC);
				Serial.print(",");
				Serial.print(rf95.lastSNR(), DEC);
				Serial.print(",");
				Serial.print(delta_time);
				Serial.println();
			#else
				RH_RF95::printBuffer("Received: ", buf, len); // raw bytes

				Serial.print("msg: ");
				Serial.println((char*)buf);

				Serial.print("RSSI: ");
				Serial.println(rf95.lastRssi(), DEC);

				Serial.print("S/N: ");
				Serial.println(rf95.lastSNR(), DEC);

				Serial.println("----------\n");
			#endif

			/*
			// Send a reply
			delay(200); // may or may not be needed
			uint8_t data[] = "And hello back to you";
			rf95.send(data, sizeof(data));
			rf95.waitPacketSent();
			Serial.println("Sent a reply");
			*/
	    } else {
			Serial.println("Receive failed");
	    }
	}
}


#endif
