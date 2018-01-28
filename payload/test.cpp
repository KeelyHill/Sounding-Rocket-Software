/** Tests the Coder class. */

#include <stdio.h>

#include "Coder.cpp"

int test_coder_types();

int main() {
	printf("Testing payload coding.\n");

	bool success = test_coder_types();

	if (success) {
		printf("--\nTests Succeeded\n");
	} else {
		printf("--\nTests FAILED\n");
	}
}

int test_coder_types() {

	static Coder c;
	uint8_t * to_send;
	size_t len;

	// c.setStateFlags(true,true,true,false);
	c.payload_state_bits = 0b00000111;
	c.arduino_millis = 42;

	c.gps_hour = 12;
	c.gps_min = 23;
	c.gps_sec = 45;
	c.gps_millis = 42;

	c.latitude = 1.1;
	c.longitude = 2.2;
	c.altitude = 3.3;
	c.gps_speed = 4.4;

	c.num_sats = 3;
	c.altimeter_alt = 1;

	c.encode_telem(&to_send, &len);

	// printf("%i\n", len);

	for (int i = 0; i < len; i ++) {
		printf("\\x%02x", to_send[i]);
		// printf(" %02x", to_send[i]);
	} printf("\n");

	return 1;
}
