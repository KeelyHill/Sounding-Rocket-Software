/**
main.cpp

*/


#ifndef __arm__
#warning "This code was written to work with the ARM Cortex M0 -- GPS reading interupt will not fire."
#endif

#include "global.h"

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <Adafruit_BMP280.h>

#include "../../common_setup.hpp"
#include "../../Coder.cpp"

#include "Logger.cpp"
#include "IMU.cpp"

#include "InteruptTimer.hpp"

#define SS_BMP 11
#define SS_SD 12

#define GPSSerial Serial1

#define STATUS_LED LED_BUILTIN

#define VBATPIN A7 // pin used on Feather for reading battery voltage

// SPI control pins are correct
#define SPI_SCK 24
#define SPI_MISO 22
#define SPI_MOSI 23


// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Status bools
bool bmp_okay, gps_okay;
bool radioInitSuccess;
bool lowBattery = false;

// telemetry
Coder coder; // TODO maybe have 2, one for saving, one for sending
uint8_t * bytes_to_send;
size_t len_bytes_to_send;

// devices
Logger logger;
IMU imu;
Adafruit_BMP280 bme(SS_BMP); //hardware SPI //, SPI_MOSI, SPI_MISO, SPI_SCK);

Adafruit_GPS GPS(&GPSSerial);


// prototypes
void transmitTelemIfRadioAvaliable();
void pullSlavesHighAndInit();
void GPSDebugPrint();
void generalDebugPrint();

// For constant reading of GPS serial data (as recommended)
void interuptTimerCallback() {
	GPS.read(); //char c = GPS.read();
}


// Unused, here for reference
int statusLEDCounter = 0;
void statusLEDUpdate() {
	if (radioInitSuccess) { // slow blink
		statusLEDCounter = statusLEDCounter > 7 ? 0 : statusLEDCounter+1;
		digitalWrite(STATUS_LED, !statusLEDCounter);
	} else { // rapid blink if failed
		statusLEDCounter = !statusLEDCounter;
		digitalWrite(STATUS_LED, statusLEDCounter);
	}
}

void setup() {

	// if (DEBUG) {
	// 	while (!Serial);
	// 	Serial.begin(115200);
	// 	delay(100);
	// }

	pinMode(STATUS_LED, OUTPUT);
	digitalWrite(STATUS_LED, HIGH); // on if failed to init

	Serial.println("Flight M0 start up");

	pullSlavesHighAndInit();

	// radio setup and init

	commonRadioSetup();
	radioInitSuccess = radioInit(rf95);
	digitalWrite(STATUS_LED, !radioInitSuccess); // on if failed to init


	/* GPS setup */

	GPS.begin(9600);
	GPSSerial.begin(9600);

	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // turn on RMC (recommended minimum) and GGA (fix data) including altitude
	// GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);   // 1 Hz update rate, 5 and 10 available
	// Request updates on antenna status, comment out to keep quiet
	// GPS.sendCommand(PGCMD_ANTENNA);

	// starts GPS serial read interupt timer
	startTimer(1000);  // 1000 Hz

	/* Other sensor setup */


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



// TODO not used at the moment
float readSelfCalibratedAltitude() {
	static float seaLevel_hP = bme.readPressure() / 100; // read once on first call
 	return bme.readAltitude(seaLevel_hP);
}


uint32_t ptimer_10sec = millis(); // pseudo timer 'thread'
uint32_t ptimer_100ms = millis(); // pseudo timer 'thread', 'execute on next opportunity'

void pseudo_thread_main_check() {
	if (GPS.newNMEAreceived()) {
		char * lastNMEA = GPS.lastNMEA();

		// Serial.print("RAW:");  Serial.println(lastNMEA);
		if (lastNMEA[5] == 'G'|| lastNMEA[5] == 'M') {
			bool parseOkay = GPS.parse(lastNMEA);  // this also sets the newNMEAreceived() flag to false

			gps_okay = parseOkay;

			if (parseOkay)
				GPSDebugPrint();
			else if (DEBUG) Serial.println("GPS Parse Failed.");
		}
	}

	// generalDebugPrint();

	/* Set telemetry variables in the coder */
	coder.arduino_millis = millis();
	coder.setStateFlags(bmp_okay, logger.sdOkay, gps_okay, (bool &)GPS.fix);

	ATOMIC_BLOCK_START; // needed for some reason, readPressure /sometimes/ freezes system when radio is doing stuff
	coder.altimeter_alt = bme.readPressure() / 100; //TODO pressure for now, awaiting proper wiring for readSelfCalibratedAltitude();
	ATOMIC_BLOCK_END;

	// generalDebugPrint();


	if (GPS.fix) {
		coder.gps_hour = GPS.hour;
		coder.gps_min = GPS.minute;
		coder.gps_sec = GPS.seconds;
		coder.latitude = GPS.latitudeDegrees;
		coder.longitude = GPS.longitudeDegrees;
		coder.altitude = GPS.altitude;
		coder.gps_speed = GPS.speed;
		coder.num_sats = GPS.satellites;
	}

	coder.tx_good = rf95.txGood(); // uint16_t total _sent_ packets can use to calc sent/received ratio

	coder.encode_telem(&bytes_to_send, &len_bytes_to_send);

	/* Log then Transmit if radio open */

	logger.log(bytes_to_send, &len_bytes_to_send);

	transmitTelemIfRadioAvaliable();
}

void loop() {

	// could 'hand query' GPS, not suggested -- using InteruptTimer now (works with M0)

	/* Sample 9DOF and update internal orientation filter. */
	// imu.sample();
	// float roll = imu.filter.getRoll();
	// float pitch = imu.filter.getPitch();
	// float yaw = imu.filter.getYaw();

	// imu.debugPrint();
	// imu.calibrationPrint();

	if (millis() - ptimer_100ms > 100) { // every 100 millis (lazy execute)
		ptimer_100ms = millis();

		pseudo_thread_main_check();
	}

	if (millis() - ptimer_10sec > 10000) { // every 10 seconds
		ptimer_10sec = millis();

		float measuredvbat = analogRead(VBATPIN);
		measuredvbat *= 6.6;    // divided by 2 * 3.3V, so multiply back and * reference voltage
		measuredvbat /= 1024; // convert to voltage

		lowBattery = measuredvbat < 3.4;  // 3.2V is when protection circuitry kicks in
	}
}


/* Only queue/send the packet if not in the middle of transmitting. Returns immediately. */
void transmitTelemIfRadioAvaliable() {
	// this can be finicky when being called in rapid succession (especially with other SPI devices)

	// if not transmitting (alt: rf95.mode() != RHGenericDriver::RHModeTX)
	if (rf95.mode() == RHGenericDriver::RHModeIdle) {
		if (DEBUG) Serial.println("START telemetry transmission.");

		rf95.send(bytes_to_send, len_bytes_to_send);
	}
}

void pullSlavesHighAndInit() {

	pinMode(RFM95_CS, OUTPUT);
	digitalWrite(RFM95_CS, HIGH);

	pinMode(SS_BMP, OUTPUT);
	digitalWrite(SS_BMP, HIGH);

	pinMode(SS_SD, OUTPUT);
	digitalWrite(SS_SD, HIGH);

	delay(1);
}

/* For debug prints of sensors. */
void generalDebugPrint() {
	if (DEBUG) {
		Serial.print("Pressure (hP):");
		Serial.println(bme.readPressure() / 100);
		Serial.print("Calibrated Alt (m):");
		Serial.println(readSelfCalibratedAltitude());
		// GPSDebugPrint();
	}
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
