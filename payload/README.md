
# Payload and Telemetry Software

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
