set logscale x
set grid
set title "Xhtml Write Time."
set xlabel "Number of XML Elements."
set xtics
#set format x ""

set logscale y
set ylabel "Time (seconds)."
# set yrange [8:35]
# set ytics 8,35,3

# set logscale y2
# set y2label "Bytes"
# set y2range [1:1e9]
# set y2tics

set pointsize 1
set datafile separator whitespace#"\t"
set datafile missing "NaN"
# Curve fit
#cost(x) = a + (b / (x/1024))
#fit cost(x) "XhtmlWriteTime.dat" using 1:2 via a,b

set terminal svg size 750,550           # choose the file format
set output "XhtmlWriteTime.svg"   # choose the output device

#set key title "Window Length"

#  lw 2 pointsize 2
plot "XhtmlWriteTime.dat" using 1:2 t "C++, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:3 t "C++, with attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:4 t "Python, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:5 t "Python, with attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:6 t "Pybind, No attributes" with linespoints, \
    "XhtmlWriteTime.dat" using 1:7 t "Pybind, with attributes" with linespoints

reset
