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
	bool debug = false;

	// int logsSinceFlush = 0;
public:
	bool sdOkay = false;

	void begin(int chipSelects, bool debug=false) {
		initOkay = SD.begin(10);
		sdOkay = initOkay;
		this->debug = debug;
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
				if (debug) Serial.println("Wrote log data.");
			} else {
				sdOkay = false;
				if (debug) Serial.println("Failed to open log file.");
			}
		}
	}
};
