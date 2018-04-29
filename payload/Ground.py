#!/usr/bin/env python3
""" Ground.py

Listens on a serial port for the raw bytes of telemetry packets,
converts them to readable values, logs to a CSV file, and displays on screen.

Usage:
$ python3 Ground.py list  # list ports
$ python3 Ground.py /dev/[portx] (baud rate)

Set `USE_CURSES` to false to turn off the "termial GUI".

By: Keely Hill
17 Nov 2017

Copyright (c) 2017, 2018 Keely Hill
"""

USE_CURSES = True  # curses is a terminal 'GUI'. False to just print line. Always writes to file.

import serial
from serial.tools import list_ports

import sys
import os

if USE_CURSES:
    import curses

from datetime import datetime

from decode import *

# how many records need to be written, before commiting to disk
LOG_FILE_FLUSH_COUNT = 10

def start_loop(port='/dev/ttyS1', baud=57600):

    port_list = list_ports.comports()
    if port not in [i[0] for i in port_list]:
        print("'%s' is not an avaliable port" % port)
        return

    if not os.path.isdir('logs'):
        os.makedirs('logs')

    running = True
    with serial.Serial(port, baud) as ser:
        print("Running and connected to ground station hardware.")
        print("Awaiting first signal...")

        window = None
        if USE_CURSES:
            window = curses.initscr()
            # right_window = curses.newwin(10, 50, 10, 50)
            curses.noecho()

        date_string = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        log_file = open('logs/telem-log-%s.csv' % date_string, 'w+')
        log_write_count = 0

        csv_header = telem_tuple_builder.replace(' ', ',') + '\n'
        log_file.write(csv_header)

        # ser.write(b'***') # tell device we are ready to start

        last_time = None

        try:
            while running:

                # Arduino restarts when a new serial connection occurs, so the following is not needed
                # USE IF preamble is needed to sync devices
                # buf = ser.read(1) # wait for a begin packet
                # while buf is not b'***':
                #     buf.append(ser.read(1))
                #     if len(buf) > 3:
                #         buf = buf[-3:]


                if last_time is None:
                    last_time = datetime.now()

                # first 4 bytes is signal strength indicator (rssi) and SNR, include in unpack
                packet_data = ser.read(TELEM_PACKET_SIZE) # blocks until all bytes collected
                # may want to save raw data to disk too

                packet = unpack_telem_packet(packet_data)  # decode and create object

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
                if not USE_CURSES:
                    telem_time_str = "%02im:%02is" % divmod((packet.arduino_millis/1000),60)
                    print("RSSI: %i| lat/lon: %.6f  %.6f | gps_alt %.0f m  altimeter: %.2f | tx_good %i dur: %s" % (packet.rssi, packet.lat, packet.lon, packet.alt, packet.altimeter_alt, packet.tx_good, telem_time_str))
                else:
                    update_curses_window(window, packet, datetime.now() - last_time)

                last_time = datetime.now()

        except KeyboardInterrupt:

            if USE_CURSES:
                curses.nocbreak(); window.keypad(0); curses.echo()
                curses.endwin()

            print("\nGoodbye.")
            running = False
            log_file.close()


def main(argv):
    num_args = len(argv)

    try:

        if num_args > 0:
            if argv[0] == 'list':
                port_list = list_ports.comports()
                print("Ports:")
                for i in port_list:
                    print(' ' + i[0])

            elif num_args >= 2:
                start_loop(argv[0], argv[1])
            else:
                start_loop(argv[0])
        else:
            print("No arguments supplied.\n /dev/[portx] (baud rate) \n list")

    # generic except to clean up curses window stuff
    except Exception as e:
        print(e)

        if USE_CURSES:
            curses.nocbreak(); curses.echo(); curses.endwin()


COL2 = 30
COL3 = 55

def b2Str(theBool):
    return 'OK' if theBool else 'NO'

def update_curses_window(window, telem_obj, delta_time_recv):
    t = telem_obj
    h, w = window.getmaxyx()

    window.clearok(True)
    window.addstr(3,2, "Telem Time: %02im:%02is" % divmod((t.arduino_millis/1000),60))
    window.addstr(4,2, "  GPS Time: %02i:%02i:%02i" % (t.gps_hour, t.gps_min, t.gps_sec))
    window.addstr(5,2, "   Seq Num: %i" % t.packet_num)
    window.addstr(6,2, "                             ")
    window.addstr(6,2, "   TX Good: %s" % (t.tx_good))
    window.addstr(7,2, "   dT Recv: %ss" % (delta_time_recv.seconds))
    window.addstr(8,2, "      RSSI: %i" % t.rssi)
    window.addstr(9,2, "       SNR: %i" % t.snr)

    ###
    window.addstr(3,COL2, "Lat: %.6f" % t.lat)
    window.addstr(4,COL2, "Lon: %.6f" % t.lon)
    window.addstr(5,COL2, "Alt: %.2f m" % t.alt)
    window.addstr(6,COL2, "BMP: %.2f" % t.altimeter_alt)
    window.addstr(7,COL2, "Vel: %.2f knots" % t.gps_speed)
    window.addstr(8,COL2, "Vel: %.2f m/s" % (t.gps_speed * 0.514444))


    ###
    window.addstr(3,COL3, "Status", curses.A_UNDERLINE)

    bmp_okay = state_bit_get(t.state_bits, 0)
    sd_okay = state_bit_get(t.state_bits, 1)
    gps_okay = state_bit_get(t.state_bits, 2)
    gps_lock = state_bit_get(t.state_bits, 3)

    window.addstr(4,COL3, "[%s] GPS Lock" % b2Str(gps_lock))
    window.addstr(5,COL3, "[%s] GPS" % b2Str(gps_okay))
    window.addstr(6,COL3, "[%s] BMP" % b2Str(bmp_okay))
    window.addstr(7,COL3, "[%s] SD Log" % b2Str(sd_okay))

    window.addstr(9,COL3, "%i GPS Sats" % t.num_sats)


    window.addstr(h-1,1, "^C to exit. Screen updates on new reception.")
    window.refresh()


if __name__ == "__main__":
    main(sys.argv[1:])
