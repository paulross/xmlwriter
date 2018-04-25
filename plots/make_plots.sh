echo "Creating plots..."
gnuplot -p XhtmlWriteTime.plt
gnuplot -p XhtmlWriteRate.plt
gnuplot -p XhtmlWriteRateWithAttrs.plt
gnuplot -p XhtmlWriteRateHistogram.plt
gnuplot -p XhtmlWriteRateHistogramWithAttrs.plt
gnuplot -p XhtmlWriteFriction.plt
gnuplot -p XhtmlWriteFrictionHistogram.plt
echo "All plots done"
