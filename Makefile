
all: firmware

firmware:
	particle compile electron
	mv *.bin firmwares/

clean:
	rm -f firmwares/*.bin
