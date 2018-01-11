set xlabel "Size (bytes)"
set ylabel "time (us)"
set title "Comm Perf for MPI (0) type blocking"
# Model complexity is (6.751236e-05 + n * 1.008978e-07)
# startup = 67.51 usec and transfer rate = 9.91 Mbytes/sec
# Variance in fit = 0.000105 (smaller is better)
plot 'pt2pt.mpl.gpl' using 4:5:7:8 with errorbars,\
67.512363+0.100898*x with dots
pause -1 "Press <return> to continue"
clear
