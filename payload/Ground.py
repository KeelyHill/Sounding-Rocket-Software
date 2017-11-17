# Python 3

import serial
from serial.tools import list_ports

import sys
import os

from datetime import datetime

from decode import *


TELEM_PACKET_SIZE = 63

def start_loop(port='/dev/ttyS1', baud=19200):

    running = True
    with serial.Serial(port, baud) as ser:
        date_string = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        log_file = open('logfile-%s.csv' % date_string, 'w+')
        log_write_count = 0

        ser.write(b'***') # tell device we are ready to start

        try:
            while running:
                packet_data = ser.read(TELEM_PACKET_SIZE) # blocks until all bytes collected
                # may want to save raw data to disk too

                packet = unpack_telem_packet(packet_data)

                # do stuff with packet

                # save to disk into csv
                as_csv_row = ','.join([str(v) for v in list(packet)]) + '\n'
                log_file.write(as_csv_row)

                if log_write_count > 20:
                    log_write_count = 0
                    log_file.flush()
                    os.fsync() # commit changes without closing

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
