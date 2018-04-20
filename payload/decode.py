
from collections import namedtuple
from struct import unpack, Struct

# in bytes
TELEM_PACKET_SIZE_RAW = 37  # run file as main to auto-calculate
TELEM_PACKET_SIZE = TELEM_PACKET_SIZE_RAW + 4

telem_packet_struct_format = "!hhIBLfBBBffffBH"
telem_packet_struct_raw_format = '!' + telem_packet_struct_format[3:]

telem_tuple_builder_raw = 'packet_num state_bits arduino_millis altimeter_alt gps_hour gps_min gps_sec lat lon alt gps_speed num_sats tx_good'
telem_tuple_builder = 'rssi snr ' + telem_tuple_builder_raw # raw does not include rssi and snr

TelemPacket = namedtuple('TelemPacket', telem_tuple_builder) # forwared from reciver, includes rssi & snr
TelemPacketRaw = namedtuple('TelemPacketRaw', telem_tuple_builder_raw) # raw written to sd (what is actually transmitted)


"""Returns TelemPacket object (namedtuple). """
def unpack_telem_packet(data:bytes):
    return TelemPacket._make(unpack(telem_packet_struct_format, data))

"""Unpacks struct of raw telemetry packet written to flight SD card"""
def unpack_raw_log_packet(data:bytes):
    return TelemPacketRaw._make(unpack(telem_packet_struct_raw_format, data))

def state_bit_get(state_bits_int, num_bit):
    return (state_bits_int >> num_bit) & 0x01

# test by running as main (i.e. not importing) TODO grab a real packet that works and put here for the test
if __name__ == '__main__':
    print("> Raw's Byte Len: ", Struct(telem_packet_struct_raw_format).size)

    b = b'\x00\x00\x00Q\x00\x00\x00P[\xc6\xc2\x02:\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    p = unpack_raw_log_packet(b)
    print(p)

    print()

    print("> Normal's Byte Len: ", Struct(telem_packet_struct_format).size)

    b = b'\xff\xcc\x00\x03\x00\x00\x00Q\x00\x00\x00P[\xc6\xc2\x02:\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    p = unpack_telem_packet(b)
    print(p)
