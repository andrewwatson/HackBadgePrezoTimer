DEVICE=2c001c000d51353432393339

all: firmware

firmware:
	particle compile electron
	cp *.bin firmwares/latest.bin
	mv *.bin firmwares/

clean:
	rm -f firmwares/*.bin

flash: firmwares/latest.bin
	particle flash ${DEVICE} firmwares/latest.bin

start:
	particle call ${DEVICE} start test

stop:
	particle call ${DEVICE} stop test

mention:
	particle call ${DEVICE} mention

ack:
	particle call ${DEVICE} acknowledge

rain:
	particle call ${DEVICE} weather rain