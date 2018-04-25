#set logscale x
set grid
set title "Xhtml Write Time (Microseconds/Element). Typically 0.7 kb/element including attributes."
set xlabel "Small, large and very large documents."
unset xtics
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
set output "XhtmlWriteRateHistogramWithAttrs.svg"   # choose the output device

#set key title "Window Length"


set style fill pattern 3
set style histogram clustered

# No Attributes
#plot "XhtmlWriteTime.dat" using (Rate($1,$4)) t "Python" w histograms fill pattern 3 lt 13, \
    "XhtmlWriteTime.dat" using (Rate($1,$6)) t "Pybind" w histograms fill pattern 3 lt 7, \
    "XhtmlWriteTime.dat" using (Rate($1,$8)) t "CPython" w histograms fill pattern 3 lt 3, \
    "XhtmlWriteTime.dat" using (Rate($1,$2)) t "C++ Baseline" w histograms fill pattern 3 lt 1
    
# With Attributes
plot "XhtmlWriteTime.dat" using (Rate($1,$5)) t "Python, with attributes" w histograms fill pattern 3 lt 13, \
    "XhtmlWriteTime.dat" using (Rate($1,$7)) t "Pybind, with attributes" w histograms fill pattern 3 lt 7, \
    "XhtmlWriteTime.dat" using (Rate($1,$9)) t "CPython, with attributes" w histograms fill pattern 3 lt 3, \
    "XhtmlWriteTime.dat" using (Rate($1,$3)) t "C++ Baseline, with attributes" w histograms fill pattern 3 lt 1

# All
#plot "XhtmlWriteTime.dat" using (Rate($1,$4)) t "Python" w histograms fill pattern 3 lt 13, \
    "XhtmlWriteTime.dat" using (Rate($1,$5)) t "Python, with attributes" w histograms fill pattern 1 lt 13, \
    "XhtmlWriteTime.dat" using (Rate($1,$6)) t "Pybind" w histograms fill pattern 3 lt 7, \
    "XhtmlWriteTime.dat" using (Rate($1,$7)) t "Pybind, with attributes" w histograms fill pattern 1 lt 7, \
    "XhtmlWriteTime.dat" using (Rate($1,$8)) t "CPython" w histograms fill pattern 3 lt 3, \
    "XhtmlWriteTime.dat" using (Rate($1,$9)) t "CPython, with attributes" w histograms fill pattern 1 lt 3, \
    "XhtmlWriteTime.dat" using (Rate($1,$2)) t "C++ Baseline" w histograms fill pattern 3 lt 1,\
    "XhtmlWriteTime.dat" using (Rate($1,$3)) t "C++ Baseline, with attributes" w histograms fill pattern 1 lt 1
    
reset
