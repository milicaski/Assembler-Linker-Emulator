all: emulator

clean:
	rm -f emulator

emulator:
	g++ -o emulator src/emulator.cpp src/ImplEmulator.cpp

test: emulator
	./emulator ../program.hex