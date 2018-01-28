
test:
	g++ test.cpp -o test.exec && ./test.exec

ground-client:
	mkdir ground_build && cp Ground.py ground_build && cp decode.py ground_build

flight:
	cd flight-m0/ && source ~/.platformio/penv/bin/activate && pio run

flight-upload:
	cd flight-m0/ && source ~/.platformio/penv/bin/activate && pio run --target upload
