
# Payload and Telemetry Software

The flight and ground computer is a M0 Feather with LoRa transiver radio module. TODO more about the chip. The ground computer's only task is as a reciving interface to a laptop. It listens for data on the transmition frequency and forwards it over USB for final decoding, additional logging on disk, and live display.

The flight computer is responsible for reading raw data from all on board sensors, encoding telemetry packets  into a byte array, and transmitting this packet via the LoRa radio.

The Radiohead library is used to communicate over SPI with the radio module. A class ('Coder') was written to encode the many data variables to a 'uint8_t' array.

## Telemetry packet structure 


## Directory structure

- `Coder.cpp`: Class for encoding a telemetry packet in flight (see file for docs and packet structure).
- `decode.py`: Contains function that decodes the payload and stores in named tuple.
- `Ground.py`: View and logger app launched from command line that reads from the serial connection (radio receiver forwarder).
	- `$ python3 Ground.py /dev/portx [baud rate]`

- `flight-m0/`: Flight telemetry computer. Handles telemetry encoding and its logging to the on-board SD card and radio transmission. May also handle experimental data, depending on the physical payload experiment.
- `ground-m0/`: Code for receiver and USB serial forwarder chip. Folder contains project for board uploading.
- `M0 LoRa Send Recive test/`: Contains code for testing the sender and receiver (notably range).

## Ground forwarder USB protocol
The ground reciver forwards recived packets over USB-serial. Because our telemetry packet size is fixed length, no extra length header is needed. By default, the microcontroller reboots (TODO test this for the FeatherM0) so no synchronizing is needed. The first 2 bytes is signal strength (RSSI), followed by fixed length raw telemetry data (`serial_read_length = fixed_telemetry_length + 2`).

`[2 bytes RSSI][telem-packet-len: raw data]`
