
CFLAGS := -g -Wall -O3 -lrt

.PHONY: all subjects tools bench trash clean

all: subjects tools

subjects:
	@cd subjects && make

tools:
	@cd tools && make

bench: subjects tools
	@tools/bench.sh

trash:
	rm -f *.dl
	rm -f *.dt

clean:
	rm -f *~ core
	@cd subjects && make clean
	@cd tools && make clean

