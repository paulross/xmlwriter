set logscale x
set grid
set title "Write Time in Excess of Baseline (C++) (Microseconds/element)."
set xlabel "Number of XML Elements."
set xtics
#set format x ""

#set logscale y
set ylabel "Excess (Microseconds/Element)."
# set yrange [8:35]
# set ytics 8,35,3
# set yrange [0:]

# set logscale y2
# set y2label "Bytes"
# set y2range [1:1e9]
# set y2tics

set pointsize 1
set datafile separator whitespace#"\t"
set datafile missing "NaN"
# Curve fit
#cost(x) = a + (b / (x/1024))
#fit cost(x) "XhtmlWriteFriction.dat" using 1:2 via a,b

# Rate(x,y) = 1e6 * y / x

Friction(num_elements, datum, actual) = 1e6 * (actual - datum) / num_elements

set terminal svg size 750,550           # choose the file format
set output "XhtmlWriteFriction.svg"   # choose the output device

#set key title "Window Length"

#  lw 2 pointsize 2
#plot "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$4)) t "Pure Python" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$5)) t "Pure Python, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$6)) t "Pybind & C++" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$7)) t "Pybind & C++, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$8)) t "CPython & C++" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$9)) t "CPython & C++, with attributes" with linespoints lw 2
    
# No attributes
plot "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$4)) t "Pure Python" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$6)) t "Pybind & C++" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$2,$8)) t "CPython & C++" with linespoints lw 2 lt 7

# With Attributes only
#plot "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$5)) t "Python, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$7)) t "Pybind & C++, with attributes" with linespoints lw 2, \
    "XhtmlWriteTime.dat" using 1:(Friction($1,$3,$9)) t "CPython & C++, with attributes" with linespoints lw 2 lt 7

reset
