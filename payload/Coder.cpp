/** Coder.cpp
v 0.1

By: Keely Hill
16 Nov 2017

Copyright (c) 2017 Keely Hill


Usage:
	#include "Coder.cpp"

	static Coder c;
	uint8_t * to_send;
	size_t len;

	c.setStateFlags(...)
	c.arduino_millis = 42;
	c.latitude = 42.42;
	c.longitude = 1.23;
	c.altitude = 100.2;
	... etc

	c.encode(&to_send, &len);


Type | name

~~c finger print?~~ IDEA TODO
I packet number
B payload state (bit flags)
L Arduino millis
f altimeter altitude
B gps_hour
B gps_min
B gps_sec
H gps_millis
f latitude
f longitude
f altitude (meters)
f gps_speed (knots)
B num_sats
H tx_good  # successful tx
-----
f roll
f pitch
f yaw

State Flag Bits

bit(from lsb) | name
0 altimeter okay
1 sd card sd okay
2 gps okay
3 gps lock

*/

// #include <cinttypes>
#include <stdint.h>
#include <stddef.h>

#ifndef CODER_CPP
#define CODER_CPP

#define TELEM_PACKET_SIZE 37 // bytes TODO recalc to check add 1 if doing checksum 63

#define BIT(x) (0x01 << (x))
#define bit_write(cond,var,b) (cond ? (var) |= (b) : (var) &= ~(b)) // bit_write(bool, variable, BIT(b))

class Coder {

	uint8_t packet[TELEM_PACKET_SIZE];

	uint32_t packet_number;

	union floatUnion {
	    float f;
	    uint32_t i;
	};

	union doubleUnion {
	    float d;
	    uint64_t i;
	};

	int encode_to(uint8_t num, uint8_t * packet, size_t start) {
		packet[start] = num;
		return start+1;
	}

	int encode_to(uint16_t num, uint8_t * packet, size_t start) {
		packet[start] = num >> 8;
		packet[start + 1] = num;

		return start+2;
	}

	int encode_to(uint32_t num, uint8_t * packet, size_t start) {
		packet[start] = num >> 24;
		packet[start + 1] = num >> 16;
		packet[start + 2] = num >> 8;
		packet[start + 3] = num >> 0;

		return start+4;
	}

	int encode_to(uint64_t num, uint8_t * packet, size_t start) {
		packet[start] = num >> 56;
		packet[start + 1] = num >> 48;
		packet[start + 2] = num >> 40;
		packet[start + 3] = num >> 32;
		packet[start + 4] = num >> 24;
		packet[start + 5] = num >> 16;
		packet[start + 6] = num >> 8;
		packet[start + 7] = num;

		return start+8;
	}

	int encode_to(float num, uint8_t * packet, size_t start) {
		// uint32_t asInt = *( (uint32_t *)&num ); // alias warning
		floatUnion fu;
		fu.f = num;

		return encode_to(fu.i, packet, start);
	}

	int encode_to(double num, uint8_t * packet, size_t start) {
		doubleUnion du;
		du.d = num;

		return encode_to(du.i, packet, start);
	}


public:

	/** telemetry sent variables */
	uint8_t payload_state_bits;
	uint32_t arduino_millis;
	float altimeter_alt;
	uint8_t gps_hour, gps_min, gps_sec;
	uint16_t gps_millis;
	float latitude, longitude, altitude;
	float gps_speed;
	uint8_t num_sats;
	uint16_t tx_good;

	Coder() {
		packet_number = 0;
	}

	/** Sets bit flags in state uint */
	void setStateFlags(bool &alt_okay, bool &sd_okay, bool &gps_okay, bool &gps_lock) {
		// can add up to 8 flags with current uint8
		bit_write(alt_okay, payload_state_bits, 0);
		bit_write(sd_okay, payload_state_bits, 1);
		bit_write(gps_okay, payload_state_bits, 2);
		bit_write(gps_lock, payload_state_bits, 3);
	}

	/** Creates a telemetry packet based on instance data.

	access data and lengnt in uint8_t* and size_t* pointers

	Usage:
		uint8_t * to_send;
		size_t len;
		myCoder.encode(&to_send, &len);
	*/
	void encode_telem(uint8_t **ret, size_t* len) {
		packet_number += 1;

		for (size_t i=0; i<TELEM_PACKET_SIZE; i++) // nullify
			packet[i]=0;

		size_t start = 0;
		uint8_t * pkt_ptr = &packet[0];

		// build structure
		start = encode_to(packet_number, pkt_ptr, start);
		start = encode_to(payload_state_bits, pkt_ptr, start);
		start = encode_to(arduino_millis, pkt_ptr, start);

		start = encode_to(altimeter_alt, pkt_ptr, start);

		start = encode_to(gps_hour, pkt_ptr, start);
		start = encode_to(gps_min, pkt_ptr, start);
		start = encode_to(gps_sec, pkt_ptr, start);
		start = encode_to(gps_millis, pkt_ptr, start);

		start = encode_to(latitude, pkt_ptr, start);
		start = encode_to(longitude, pkt_ptr, start);
		start = encode_to(altitude, pkt_ptr, start);
		start = encode_to(gps_speed, pkt_ptr, start);

		start = encode_to(num_sats, pkt_ptr, start);

		start = encode_to(tx_good, pkt_ptr, start);

		// xor sum test, does NOT prepend at the moment
		// uint8_t xorsum = 0;
		// for (size_t i=0; i<TELEM_PACKET_SIZE; i++) {
		// 	xorsum ^= packet[i];
		// }

		// returns
		*len = TELEM_PACKET_SIZE;
		*ret = &packet[0];
	}


};


#endif
