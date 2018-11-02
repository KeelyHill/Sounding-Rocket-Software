
# Payload and Telemetry Software

The flight and ground computer is a ARM Cortex M0 as an Adafruit Feather with LoRa transceiver radio module. The ground computer's only task is as a receiving interface to a laptop. It listens for data on the transmission frequency and forwards it over USB for final decoding, additional logging on disk, and live display.

The flight computer is responsible for reading raw data from all on board sensors, encoding telemetry packets  into a byte array, and transmitting this packet via the LoRa radio.

The Radiohead library is used to communicate over SPI with the radio module. A class ('Coder') was written to encode the many data variables to a 'uint8_t' array.

## Telemetry packet structure
A telemetry packet is just a concatenated byte string of the telemetry values in their binary (big-endian) format.
A list is located `Coder.cpp` and `decode.py`.

## Directory structure

- `Coder.cpp`: Class for encoding a telemetry packet in flight (see file for docs and packet structure).
- `decode.py`: Contains function that decodes the payload and stores in named tuple.
- `Ground.py`: View and logger app launched from command line that reads from the serial connection (radio receiver forwarder).
	- `$ python3 Ground.py /dev/portx [baud rate]`

- `flight-m0/`: Flight telemetry computer. Handles telemetry encoding and its logging to the on-board SD card and radio transmission. May also handle experimental data, depending on the physical payload experiment.
- `ground-m0/`: Code for receiver and USB serial forwarder chip. Folder contains project for board uploading.
- `M0 LoRa Send Recive test/`: Contains code for testing the sender and receiver (notably range).

## Ground forwarder USB protocol
The ground receiver forwards received packets over USB-serial. Because our telemetry packet size is fixed length, no extra length header is needed. By default, the microcontroller reboots (TODO test this for the FeatherM0) so no synchronizing is needed. The first 4 bytes are signal strength (RSSI) and signal-to-noise ratio (SNR), followed by fixed length raw telemetry data (`serial_read_length = raw_telemetry_length + 4`).

`[2 bytes RSSI][2 bytes SNR][telem-packet-len: raw data]`

## Preflight checklist
- Plug in ground station.
- Discover serial port: `$ ./Ground.py list`
- Start ground station program
	- `$ ./Ground.py /dev/xxx`
- Turn on flight computer (_may_ be done before ground).
- Ensure the status led is not rapidly blinking (several times a second).
- Check for good reception and statuses in ground program.

## Building
(Assumes PlatformIO is installed on the system. See Makefile for details.)

**Flight M0:**  
`$ make flight` or  
`$ make flight-upload`

**Ground M0:**  
`$ make ground` or  
`$ make ground-upload`

**Ground Client (Python)**  
`$ make ground-client`

## Peripheral devices
- Barometric Pressure Sensor (BPM)  [SPI]
- 9 DOF IMU  [I2C]
- GPS  [Serial]
- SD Reader [SPI]
