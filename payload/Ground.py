""" Ground.py

Usage:
$ python3 Ground.py /dev/[portx] (baud rate)

By: Keely Hill
17 Nov 2017

Copyright (c) 2017 Keely Hill
"""

import serial
from serial.tools import list_ports

import sys
import os

from datetime import datetime

from decode import *

# how many records need to be written, before commiting to disk
LOG_FILE_FLUSH_COUNT = 20

TELEM_PACKET_SIZE = 35 + 1
TELEM_PACKET_SIZE = 6 # temp for small testing

def start_loop(port='/dev/ttyS1', baud=57600):

    running = True
    with serial.Serial(port, baud) as ser:
        date_string = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        log_file = open('logs/telem-log-%s.csv' % date_string, 'w+')
        log_write_count = 0

        csv_header = telem_tuple_builder.replace(' ', ',') + '\n'
        log_file.write(csv_header)

        # ser.write(b'***') # tell device we are ready to start

        try:
            while running:

                # Arduino restarts when a new serial connection occurs, so the following is not needed
                # USE IF preamble is needed to sync devices
                # buf = ser.read(1) # wait for a begin packet
                # while buf is not b'***':
                #     buf.append(ser.read(1))
                #     if len(buf) > 3:
                #         buf = buf[-3:]

                # first 2 bytes is signal strength indicator (rssi), include in unpack
                packet_data = ser.read(2 + TELEM_PACKET_SIZE) # blocks until all bytes collected
                # may want to save raw data to disk too

                packet = unpack_telem_packet(packet_data)

                print(packet) # temp dev for testing

                # save to disk into csv
                as_csv_row = ','.join([str(v) for v in list(packet)]) + '\n'
                # as_csv_row = ','.join([str(v) for v in list(packet)+[rssi]]) + '\n'
                log_file.write(as_csv_row)

                log_write_count += 1
                if log_write_count > LOG_FILE_FLUSH_COUNT:
                    log_write_count = 0
                    log_file.flush()
                    os.fsync(log_file) # commit changes without closing


                # print / update screen

        except KeyboardInterrupt:
            running = False
            log_file.close()


def main(argv):
    num_args = len(argv)

    if num_args > 0:
        if argv[0] == 'list':
            port_list = list_ports.comports()
            print("Ports:")
            [print(' ' + i[0]) for i in port_list]
        elif num_args >= 2:
            start_loop(argv[0], argv[1])
        else:
            start_loop(argv[0])
    else:
        print("No arguments supplied.")


if __name__ == "__main__":
    main(sys.argv[1:])
