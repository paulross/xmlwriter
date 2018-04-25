#set logscale x
set grid
set title "Write Time in Excess of Baseline (C++) (Microseconds/element)."
set xlabel "Small, large and very large documents."
unset xtics
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
#set terminal svg size 600,400           # choose the file format
set output "XhtmlWriteFrictionHistogram.svg"   # choose the output device

#set key title "Window Length"

#set style fill pattern 7
set style histogram clustered

# No Attributes
#plot "XhtmlWriteTime.dat" using (Friction($1,$2,$4)) t "Python" w histograms fill solid lt 7, \
    "XhtmlWriteTime.dat" using (Friction($1,$2,$6)) t "Pybind" w histograms fill solid lt 2, \
    "XhtmlWriteTime.dat" using (Friction($1,$2,$8)) t "CPython" w histograms fill solid lt 1
    
# With Attributes
#plot "XhtmlWriteTime.dat" using (Friction($1,$3,$5)) t "Python, with attributes" w histograms fill solid lt 7, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$7)) t "Pybind, with attributes" w histograms fill solid lt 2, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$9)) t "CPython, with attributes" w histograms fill solid lt 1

# With and without attributes
#plot "XhtmlWriteTime.dat" using (Friction($1,$2,$4)) t "Python" w histograms fill solid lt 7, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$5)) t "Python, with attributes" w histograms fill solid lt 8, \
    "XhtmlWriteTime.dat" using (Friction($1,$2,$6)) t "Pybind" w histograms fill solid lt 2, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$7)) t "Pybind, with attributes" w histograms fill solid lt 3, \
    "XhtmlWriteTime.dat" using (Friction($1,$2,$8)) t "CPython" w histograms fill solid lt 1, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$9)) t "CPython, with attributes" w histograms fill solid lt 4

# With and without attributes, pybind and CPython only
plot "XhtmlWriteTime.dat" using (Friction($1,$2,$6)) t "Pybind" w histograms fill pattern 3 lt 7, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$7)) t "Pybind+attributes"  w histograms fill pattern 1 lt 7, \
    "XhtmlWriteTime.dat" using (Friction($1,$2,$8)) t "CPython"  w histograms fill solid 3, \
    "XhtmlWriteTime.dat" using (Friction($1,$3,$9)) t "CPython+attributes" w histograms fill pattern 1 lt 3

reset
