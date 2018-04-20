/**
main.cpp - Ground LoRa M0 receiver and USB forwarder

Listens on frequency for LoRa packets. Forwards the packet over USB (Serial 57600 baud) along with RSSI and SNR.
`[2 bytes RSSI][2 bytes SNR][telem-packet-len: raw data]`

Status LED on when activly receiving.
*/

#define DEBUG false // if true, the ground computer will not be able to decode packets as Serial will have debug prints

#include <Arduino.h>
#include "../../common_setup.hpp"

#define STATUS_LED_PIN LED_BUILTIN // TODO actual external pin?

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
	pinMode(STATUS_LED_PIN, OUTPUT);
	digitalWrite(STATUS_LED_PIN, LOW);

	Serial.begin(57600);

	commonRadioSetup();
	radioInit(rf95);

	// Serial.print("***");
}

uint8_t recvBuf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t recvLen = sizeof(recvBuf);
uint8_t rssiAndSNR[4];

void loop() {
	if (rf95.available()) { // new message available?

		digitalWrite(STATUS_LED_PIN, HIGH);

		if (rf95.recv(recvBuf, &recvLen)) {

			// TODO check packet len consistant with telem packet len (otherwise fail state)

			int16_t rssi = rf95.lastRssi();
			int16_t snr = rf95.lastSNR();

			rssiAndSNR[0] = rssi >> 8;
			rssiAndSNR[1] = rssi;
			rssiAndSNR[2] = snr >> 8;
			rssiAndSNR[3] = snr;

			Serial.write(rssiAndSNR, 4);
			Serial.write(recvBuf, recvLen);

		} else { // receive failed
			// TODO maybe send all 0's as indicator?
			if (DEBUG) Serial.println("Receive failed.");
		}

		digitalWrite(STATUS_LED_PIN, LOW); // status led off
	}
}
