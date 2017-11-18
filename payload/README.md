
# Payload and Telemetry Software

- `Coder.cpp`: Class for encoding a telemetry packet (see file for docs and packet structure).
- `decode.py`: Contains function that decodes the payload and stores in named tuple.
- `Ground.py`: View and logger app launched from command line that reads from the serial connection (radio receiver forwarder).
	- `$ python3 Ground.py /dev/portx [baud rate]`

- `flight-m0/`
- `ground-m0/`: Receiver and USB serial forwarder. contains project for board uploading

## Ground forwarder USB protocol
`[2 byte: signal strength][telem-packet-len: raw data]`

First 2 bytes is signal strength (RSSI), followed by fixed length raw telemetry data.
