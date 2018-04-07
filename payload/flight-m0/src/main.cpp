/**
main.cpp

*/

#include "global.h"

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <Adafruit_BMP280.h>

#include "../../common_setup.hpp"
#include "../../Coder.cpp"

#include "Logger.cpp"
#include "IMU.cpp"

#define SS_ALT 11
#define SS_ACCEL 12
#define SS_SD 13

// SPI control pins are correct
#define SPI_SCK 24
#define SPI_MISO 22
#define SPI_MOSI 23

#define GPSSerial Serial1

#define STATUS_LED LED_BUILTIN

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Status bools
bool alt_okay, gps_okay;

Coder coder;
uint8_t * to_send;
size_t len_to_send;

Logger logger;
IMU imu;
Adafruit_BMP280 bme(SS_ALT); //hardware SPI //, SPI_MOSI, SPI_MISO, SPI_SCK);

Adafruit_GPS GPS(&GPSSerial);
bool usingInterruptForGPS = false;
void useInterruptForGPS(boolean); // proto


// helper prototypes
void pullSlavesHighAndInit();
void GPSDebugPrint();
void printlnRawBytes(uint8_t *bytes, size_t* len);


void setup() {

	if (DEBUG) {
		while (!Serial);
		Serial.begin(115200);
		delay(100);
	}

	pinMode(STATUS_LED, OUTPUT);

	Serial.println("Flight M0 start up");

	pullSlavesHighAndInit();

	// radio setup and init

	common_radio_setup();
	radioInit(rf95);

	// GPS setup

	GPS.begin(9600);
	GPSSerial.begin(9600);

	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // turn on RMC (recommended minimum) and GGA (fix data) including altitude
	// GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);   // 1 Hz update rate, 5 and 10 available
	// Request updates on antenna status, comment out to keep quiet
	// GPS.sendCommand(PGCMD_ANTENNA);

	#ifdef __arm__
		usingInterruptForGPS = false;  //NOTE - we don't want to use interrupts on the Due
	#else
	 	useInterruptForGPS(true);
	#endif

	// Other sensor setup

	while (!bme.begin()) {
		Serial.println("Could not find a valid BMP280 sensor, check wiring!");
		delay(300);
  	}

	while(!imu.begin()) {
		Serial.println("No LSM9DS1 detected ... Check your wiring!");
		delay(300);
	}


	Serial.println("Setup done.");
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
		if (DEBUG && DEBUG_GPS_RAW)
			if (c) Serial.print(c);
	}


	if (GPS.newNMEAreceived()) {
		char * lastNMEA = GPS.lastNMEA();

		// Serial.print("RAW:");
		// Serial.println(lastNMEA); Serial.println("---");
		// if ((lastNMEA[3] == 'G' && lastNMEA[4] == 'G' && lastNMEA[5] == 'A') || (lastNMEA[3] == 'R' && lastNMEA[4] == 'M' && lastNMEA[5] == 'C')) {
		if (lastNMEA[5] == 'G'|| lastNMEA[5] == 'M') {
			bool parseOkay = GPS.parse(lastNMEA);  // this also sets the newNMEAreceived() flag to false

			gps_okay = parseOkay;

			if (parseOkay)
				GPSDebugPrint();
			else if (DEBUG) Serial.println("GPS Parse Failed.");
		}
  	}

	/* Sample 9DOF and update internal orientation filter. */
	imu.sample();
	float roll = imu.filter.getRoll();
	float pitch = imu.filter.getPitch();
	float yaw = imu.filter.getYaw();

	// imu.debugPrint();
	// imu.calibrationPrint();

	// generalDebugPrint();

	if (millis() - timer_2sec > 2000) { // every two seconds
		timer_2sec = millis(); // not used at the moment
	}

	/* Set telemetry variables in the coder */
	coder.arduino_millis = millis();
	coder.setStateFlags(logger.sdOkay, logger.sdOkay, gps_okay, (bool &)GPS.fix);
	coder.altimeter_alt = bme.readPressure(); // TODO, pressure for now, so we can calc post flight. bme.readAltitude(1010.82)

	if (GPS.fix) {
		coder.gps_hour = GPS.hour;
		coder.gps_min = GPS.minute;
		coder.gps_sec = GPS.seconds;
		coder.gps_millis = GPS.milliseconds;  // TODO probably not needed
		coder.latitude = GPS.latitudeDegrees;
		coder.longitude = GPS.longitudeDegrees;
		coder.altitude = GPS.altitude;
		coder.gps_speed = GPS.speed;
		coder.num_sats = GPS.satellites;
	}

	coder.tx_good = rf95.txGood(); // uint16_t total _sent_ packets can use to calc sent/received ratio

	coder.encode_telem(&to_send, &len_to_send);

	/* Log then Transmit if radio open */

	if (DEBUG) {
		printlnRawBytes(to_send, &len_to_send);
		delay(100);
	}

	// logger.log(to_send, &len_to_send);

	// transmitTelemIfRadioAvaliable();
}

/* Only queue/send the packet if not in the middle of transmitting. Returns immediately. */
void transmitTelemIfRadioAvaliable() {
	// if not transmitting (alt: rf95.mode() != RHGenericDriver::RHModeTX)
	if (rf95.mode() == RHGenericDriver::RHModeIdle) {
		// TODO IF this does not work (because waitPacketSent() is called by send()), use an interupt to create a pseudo-thread
		if (DEBUG) Serial.println("START telemetry transmission."); // TODO test

		rf95.send(to_send, len_to_send);
	}
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
			Serial.print(GPS.latitudeDegrees, 4);
			Serial.print(", ");
			Serial.println(GPS.longitudeDegrees, 4);

			Serial.print("Speed (knots): "); Serial.println(GPS.speed);
			Serial.print("Angle: "); Serial.println(GPS.angle);
			Serial.print("Altitude: "); Serial.println(GPS.altitude);
			Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
		}
		Serial.println();
	}
}

void printlnRawBytes(uint8_t *bytes, size_t* len) {
	for (size_t i=0; i<*len; i++) {
		Serial.print("\\x");
		Serial.print(bytes[i] < 16 ? "0" : "");
		Serial.print(bytes[i], HEX);
	}
	Serial.println("\n");

}
