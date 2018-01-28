/** */

#define DEBUG false

#include <Arduino.h>
#include <Adafruit_GPS.h>

#include "../../common_setup.hpp"
#include "../../Coder.cpp"

#include "Logger.cpp"

// TODO real SS pins
#define SS_ALT 2
#define SS_ACCEL 3
#define SS_SD 4

#define GPSSerial Serial1

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Status bools
bool alt_okay, gps_okay;

Coder coder;
uint8_t * to_send;
size_t len_to_send;

Logger logger;

Adafruit_GPS GPS(&GPSSerial);
bool usingInterruptForGPS = false;
void useInterruptForGPS(boolean); // proto

// helper prototypes
void pullSlavesHighAndInit();
void GPSDebugPrint();

void radioInit() {
	while (!rf95.init()) {
		Serial.println("LoRa radio init failed!!");
		while (1);  // <-- TODO add some outside indicator (like LED or buzzer sequence) in loop, perhase remove this block
		// TODO see what happens in a failed case
	}
	if (DEBUG) Serial.println("LoRa radio init OK!");

	if (!rf95.setFrequency(RF95_FREQ)) {
		Serial.println("setFrequency failed");
		while (1); // <-- TODO add some outside indicator (like LED or buzzer sequence) in loop, perhase remove this block
		// TODO see what happens in a failed case
	}
	if (DEBUG) Serial.print("Freq set to: "); Serial.println(RF95_FREQ);

	// can set transmitter powers from 5 to 23 dBm:
	rf95.setTxPower(23, false);

	if (DEBUG) Serial.println("LoRa radio READY.");
}

void setup() {
	Serial.begin(115200);
	Serial.println("Flight M0 start up");

	pullSlavesHighAndInit();

	common_radio_setup();
	radioInit();

	GPS.begin(9600);
	GPSSerial.begin(9600);

	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate, 5 and 10 available

	// Request updates on antenna status, comment out to keep quiet
	GPS.sendCommand(PGCMD_ANTENNA);

	#ifdef __arm__
		usingInterruptForGPS = false;  //NOTE - we don't want to use interrupts on the Due
	#else
	 	useInterruptForGPS(true);
	#endif

	logger.begin(SS_SD, DEBUG);

	Serial.println("Setup done.");

	pinMode(LED_BUILTIN, OUTPUT);

}

#ifdef __AVR__

	// This interrupt is (auto) called once a millisecond, looks for any new GPS data, and stores it
	SIGNAL(TIMER0_COMPA_vect) {
		char c = GPS.read(); // TODO test what happens when this is removed


		#ifdef UDR0
			if (DEBUG)
				if (c) UDR0 = c;
				// writing direct to UDR0 is much much faster than Serial.print
				// but only one character can be written at a time.
		#endif
	}

	void useInterruptForGPS(boolean v) {
		if (v) {
			// Timer0 is already used for millis() - we'll just interrupt somewhere
			// in the middle and call the "Compare A" function above
			OCR0A = 0xAF;
			TIMSK0 |= _BV(OCIE0A);
			usingInterruptForGPS = true;
		} else {
			// do not call the interrupt function COMPA anymore
			TIMSK0 &= ~_BV(OCIE0A);
			usingInterruptForGPS = false;
		}
	}

#endif //#ifdef__AVR__

uint32_t timer_2sec = millis();

void loop() {

	// 'hand query' the GPS, not suggested :(
	if (! usingInterruptForGPS) {
		// read data from the GPS in the 'main loop'
		char c = GPS.read();
		if (DEBUG)
			if (c) Serial.print(c);
	}


	if (GPS.newNMEAreceived()) {
    	bool parseOkay = GPS.parse(GPS.lastNMEA());  // this also sets the newNMEAreceived() flag to false

		gps_okay = parseOkay;

		if (parseOkay)
			GPSDebugPrint();
		else if (DEBUG) Serial.println("GPS Parse Failed.");

  	}


	if (millis() - timer_2sec > 2000) { // every two seconds
		timer_2sec = millis();
	}

	// set telemetry variables in the coder
	coder.arduino_millis = millis();
	coder.setStateFlags(logger.sdOkay, logger.sdOkay, gps_okay, (bool &)GPS.fix);

	coder.encode_telem(&to_send, &len_to_send);


	/* log and send if radio open */

	logger.log(to_send, &len_to_send);

	/* Only queue/send the packet if not in the middle of transmitting */
	// If not transmitting (alt: rf95.mode() != RHGenericDriver::RHModeTX)
	if (rf95.mode() == RHGenericDriver::RHModeIdle) {
		// TODO if this does not work (because waitPacketSent() is called by send()), use an interupt to create a pseudo-thread
		if (DEBUG) Serial.println("START telemetry transmission."); // TODO test
		rf95.send(to_send, len_to_send);
	}

	delay(10);
	// Serial.println("-TICK-");
}

void pullSlavesHighAndInit() {

	pinMode(RFM95_CS, OUTPUT);
	digitalWrite(RFM95_CS, HIGH); // TODO test to ensure RadioHead pulls to low when trying to talk to the LoRa.

	pinMode(SS_ALT, OUTPUT);
	digitalWrite(SS_ALT, HIGH);

	pinMode(SS_ACCEL, OUTPUT);
	digitalWrite(SS_ACCEL, HIGH);

	pinMode(SS_SD, OUTPUT);
	digitalWrite(SS_SD, HIGH);

	delay(1);
}

void GPSDebugPrint() {
	if (DEBUG) {
		Serial.print("\nTime: ");
		Serial.print(GPS.hour, DEC); Serial.print(':');
		Serial.print(GPS.minute, DEC); Serial.print(':');
		Serial.print(GPS.seconds, DEC); Serial.print('.');
		Serial.println(GPS.milliseconds);
		Serial.print("Date (d/m/y): ");
		Serial.print(GPS.day, DEC); Serial.print('/');
		Serial.print(GPS.month, DEC); Serial.print("/20");
		Serial.println(GPS.year, DEC);
		Serial.print("Fix: "); Serial.print((int)GPS.fix);
		Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

		if (GPS.fix) {
			Serial.print("Location: ");
			Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
			Serial.print(", ");
			Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);

			Serial.print("Speed (knots): "); Serial.println(GPS.speed);
			Serial.print("Angle: "); Serial.println(GPS.angle);
			Serial.print("Altitude: "); Serial.println(GPS.altitude);
			Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
		}
	}
}
