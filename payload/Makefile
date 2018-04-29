##
# Makefile for all things payload!
#
# Uses PlatformIO for compilation and upload.
# Ensure PIO_DIR constant is correct.
##

# PlatformIO 'dot' directory. Contains `penv`.
PIO_DIR = ~/.platformio

flightCMD = cd flight-m0/ && source $(PIO_DIR)/penv/bin/activate && pio run
groundCMD = cd ground-m0/ && source $(PIO_DIR)/penv/bin/activate && pio run


clean: flight-clean ground-clean ground-client-clean

test:
	g++ test.cpp -o test.exec && ./test.exec

ground-client:
	mkdir ground_build && cp Ground.py ground_build && cp decode.py ground_build && chmod +x ground_build/Ground.py

ground-client-clean:
	rm -rf ground_build/

##


flight:
	$(flightCMD)

flight-upload:
	$(flightCMD) --target upload

flight-clean:
	$(flightCMD) --target clean

##

ground:
	$(groundCMD)

ground-upload:
	$(groundCMD) upload

ground-clean:
	$(groundCMD) clean
