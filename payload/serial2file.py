""" Serial monitor that also writes directly out to a file

Usage:
$ python3 serial2file.py list  # list ports
$ python3 serial2file.py /dev/[portx] (baud rate)

Copyleft Feb 2018 Keely Hill
"""

import serial
from serial.tools import list_ports
from datetime import datetime
import sys, os

# how many records need to be written, before commiting to disk
LOG_FILE_FLUSH_COUNT = 20

def start_loop(port='/dev/ttyS1', baud=57600):

    running = True
    with serial.Serial(port, baud) as ser:
        date_string = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        log_file = open('logs/serial-out-log-%s.txt' % date_string, 'w+')
        log_write_count = 0

        try:
            while running:
                line = ser.readline().decode('utf8')
                line.replace('\r', '')
                print(line, end='')
                log_file.write(line)

                log_write_count += 1
                if log_write_count > LOG_FILE_FLUSH_COUNT:
                    log_write_count = 0
                    log_file.flush()
                    os.fsync(log_file) # commit changes without closing

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
