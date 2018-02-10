/** Simple class for logging data to an SD card via a SPI SD card module */

#include "global.h"
#include <SD.h>

/*

Usage:
 Logger logger;
 logger.begin(SD_chip_select_pin);
 logger.log(data, len); // writes data followed by two newline chars
*/
class Logger {

	File file;

	bool initOkay = false;

	// int logsSinceFlush = 0;
public:
	bool sdOkay = false;

	void begin(int chipSelects) {
		initOkay = SD.begin(10);
		sdOkay = initOkay;
	}

	void log(uint8_t *data, size_t *len) {
		this->log((char *)data, len);
	}

	void log(char *data, size_t *len) {

		if (initOkay) {
			file = SD.open("flight.data.txt", FILE_WRITE);

			if (file) {
				file.write(data, *len);
				file.write("\n\n");
				file.flush(); // TODO maybe close instead need to test on board
				if (DEBUG) Serial.println("Wrote log data.");
			} else {
				sdOkay = false;
				if (DEBUG) Serial.println("Failed to open log file.");
			}
		}
	}
};
