
CFLAGS = -g -Wall -O3 -lrt

all: machid \
	unix_lat unix_thr \
	tcp_lat tcp_thr \
	tcp_local_lat tcp_remote_lat \
	udp_lat

bench: all
	@./bench.sh

trash:
	rm -f *.l
	rm -f *.t

clean:
	rm -f *~ core
	rm -f machid
	rm -f unix_lat unix_thr 
	rm -f tcp_lat tcp_thr 
	rm -f tcp_local_lat tcp_remote_lat
	rm -f udp_lat
