
test:
	g++ test.cpp -o test.exec && ./test.exec

ground-client:
	mkdir ground_build && cp Ground.py ground_build && cp decode.py ground_build && chmod +x ground_build/Ground.py

flight:
	cd flight-m0/ && source ~/.platformio/penv/bin/activate && pio run

flight-upload:
	cd flight-m0/ && source ~/.platformio/penv/bin/activate && pio run --target upload

ground:
	cd ground-m0/ && source ~/.platformio/penv/bin/activate && pio run

ground-upload:
	cd ground-m0/ && source ~/.platformio/penv/bin/activate && pio run --target upload
