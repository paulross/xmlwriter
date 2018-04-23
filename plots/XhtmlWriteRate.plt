set logscale x
set grid
set title "Xhtml Write Time (Microseconds/Element). Typically 0.5 kb/element."
set xlabel "Number of XML Elements."
set xtics
#set format x ""

#set logscale y
set ylabel "Microseconds/Element."
# set yrange [8:35]
# set ytics 8,35,3
set yrange [0:]

# set logscale y2
# set y2label "Bytes"
# set y2range [1:1e9]
# set y2tics

set pointsize 1
set datafile separator whitespace#"\t"
set datafile missing "NaN"
# Curve fit
#cost(x) = a + (b / (x/1024))
#fit cost(x) "XhtmlWriteRate.dat" using 1:2 via a,b

Rate(x,y) = 1e6 * y / x

set terminal svg size 750,550           # choose the file format
set output "XhtmlWriteRate.svg"   # choose the output device

#set key title "Window Length"

#  lw 2 pointsize 2
#plot "XhtmlWriteTime.dat" using 1:(Rate($1,$2)) t "C++ Baseline, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$3)) t "C++ Baseline, with attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$4)) t "Python, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$5)) t "Python, with attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$6)) t "Pybind, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$7)) t "Pybind, with attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$8)) t "CPython, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$9)) t "CPython, with attributes" with linespoints

# No attributes
plot "XhtmlWriteTime.dat" using 1:(Rate($1,$2)) t "C++ Baseline" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$4)) t "Python" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$6)) t "Pybind" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$8)) t "CPython" with linespoints lw 2 lt 7

# With Attributes only
#plot "XhtmlWriteTime.dat" using 1:(Rate($1,$3)) t "C++ Baseline, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$5)) t "Python, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$7)) t "Pybind, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Rate($1,$9)) t "CPython, with attributes" with linespoints lw 2 lt 7


reset
