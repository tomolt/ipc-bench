#!/usr/bin/env gnuplot

set title "Measured throughput of multiple IPC methods (higher is better)"

set xlabel 'IPC method'
set ylabel 'throughput in megabits per second (Mbit/s)'

set boxwidth 0.5
set style fill solid

set yrange [0:*]

plot 'throughput.dat' using 0:2:xtic(1) title 'throughput' with boxes

