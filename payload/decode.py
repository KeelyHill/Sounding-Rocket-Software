
from collections import namedtuple
from struct import unpack

telem_packet_struct_format = "!hfH"
telem_packet_struct_format = "!xxIBLfBBBHffffB"

telem_tuple_builder = 'rssi somefloat status_flags'
telem_tuple_builder = 'rssi snr state_bits arduino_millis altimeter_alt gps_hour gps_min gps_sec gps_millis lat lon alt gps_speed num_sats'

TelemPacket = namedtuple('TelemPacket', telem_tuple_builder)


"""Returns TelemPacket object (namedtuple). """
def unpack_telem_packet(data:bytes):
    return TelemPacket._make(unpack(telem_packet_struct_format, data))


# test by running as main (i.e. not importing) TODO grab a real packet that works and put here for the test
if __name__ == '__main__':
    b = b'\x00\x00\x00\x00\x00\x00\x01\x07\x00\x00\x00\x2a\x0c\x17\x2d\x00\x2a\x3f\x8c\xcc\xcd\x40\x0c\xcc\xcd\x40\x53\x33\x33\x40\x8c\xcc\xcd\x03\x3f\x80\x00\x00'
    p = unpack_telem_packet(b)
    print(p)
