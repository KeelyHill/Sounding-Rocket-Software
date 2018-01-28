
#include <SPI.h>
#include <RH_RF95.h>
#include <RHGenericDriver.h> // for enum RHMode

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 915.0

void common_radio_setup() {

	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);

	// manual radio reset TODO test if needed
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);
}
