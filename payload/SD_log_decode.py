"""Takes in put raw SD card flight log file, and converts to a CSV.

The SD flight log file is all raw packets (as struct bytes) concatinated together,
this converts them to a CSV with more usable values.

Usage:
python3 ~/.../flight.data
# exports to flight.data.csv

"""

import sys
from struct import error as struct_error

from decode import *

"""TODO this needs to be tested!"""
def decode_raw_file_to_csv(file_name):
    in_file = open(file_name, 'rb')
    out_file = open(file_name + '.csv', 'w+')

    csv_header = telem_tuple_builder_raw.replace(' ', ',') + '\n'
    out_file.write(csv_header)

    while True: # read in chunks of raw packet length
        raw_packet = in_file.read(TELEM_PACKET_SIZE_RAW)
        if not raw_packet:
            break

        try:
            packet = unpack_raw_log_packet(raw_packet)
            as_csv_row = ','.join([str(v) for v in list(packet)]) + '\n'
            out_file.write(as_csv_row)
        except struct_error as e:
            print('Error unpacking:', e)
            break



    in_file.close()
    out_file.close()

def main(argv):
    num_args = len(argv)

    if num_args > 0:
        decode_raw_file_to_csv(argv[0])
    else:
        print("No raw data file path supplied.")


if __name__ == "__main__":
    main(sys.argv[1:])
