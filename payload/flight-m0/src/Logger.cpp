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
	bool sd_okay = false;

	void begin(int chipSelects, bool debug=false) {
		initOkay = SD.begin(10);
		sd_okay = initOkay;
		this->debug = debug;
	}

	void log(char *data, int len) {

		if (initOkay) {
			file = SD.open("data.txt", FILE_WRITE);

			if (file) {
				file.write(data, len);
				file.write("\n\n");
				file.flush();
				if (debug) Serial.println("Wrote log data.");
			} else {
				sd_okay = false;
				if (debug) Serial.println("Failed to open log file.");
			}
		}
	}
};
