#!/usr/bin/env gnuplot

set title "Measured latencies of multiple IPC methods (lower is better)"

set xlabel 'IPC method'
set ylabel 'latency in nanoseconds (ns)'

set boxwidth 0.5
set style fill solid

set yrange [0:*]

plot 'latency.dat' using 0:2:xtic(1) title 'latency' with boxes

