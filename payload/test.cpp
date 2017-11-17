#include <stdio.h>

#include "Coder.cpp"


int test_coder_types();

int main() {
	printf("Testing payload coding.\n");

	test_coder_types();
}

int test_coder_types() {

	static Coder c;
	uint8_t * to_send;
	size_t len;

	c.payload_state_bits = 42;
	c.arduino_millis = 42;

	c.gps_hour = 12;
	c.gps_min = 23;
	c.gps_sec = 45;
	c.gps_millis = 42;

	c.latitude = 42.42;
	c.longitude = 1.23;
	c.altitude = 100.2;
	c.gps_speed = 42;

	c.num_sats = 3;
	c.altimeter_alt = 42;

	c.encode(&to_send, &len);

	for (int i = 0; i < len; i ++) {
		printf("\\x%02x", to_send[i]);
		// printf(" %02x", to_send[i]);
	} printf("\n");

	return 1;
}
