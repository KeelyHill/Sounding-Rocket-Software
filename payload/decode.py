
from collections import namedtuple
from struct import unpack

telem_packet_struct_format = "!xxIBLfBBBHffffBH"

telem_tuple_builder_raw = 'packet_num state_bits arduino_millis altimeter_alt gps_hour gps_min gps_sec gps_millis lat lon alt gps_speed num_sats tx_good'
telem_tuple_builder = 'rssi snr ' + telem_tuple_builder_raw

TelemPacket = namedtuple('TelemPacket', telem_tuple_builder) # forwared from reciver, includes rssi & snr
TelemPacketRaw = namedtuple('TelemPacket', telem_tuple_builder) # raw written to sd (what is actually transmitted)


"""Returns TelemPacket object (namedtuple). """
def unpack_telem_packet(data:bytes):
    return TelemPacket._make(unpack(telem_packet_struct_format, data))

"""Unpacks struct of raw telemetry packet written to flight SD card"""
def unpack_sd_log_row(data:bytes):
    return TelemPacketRaw._make(unpack('!' + telem_packet_struct_format[2:], data))


# test by running as main (i.e. not importing) TODO grab a real packet that works and put here for the test
if __name__ == '__main__':
    b = b'\x00\x00\x00\x00\x00\x00\x01\x07\x00\x00\x00\x2a\x0c\x17\x2d\x00\x2a\x3f\x8c\xcc\xcd\x40\x0c\xcc\xcd\x40\x53\x33\x33\x40\x8c\xcc\xcd\x03\x3f\x80\x00\x00'
    p = unpack_telem_packet(b)
    print(p)
