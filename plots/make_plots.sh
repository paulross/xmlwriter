echo "Creating plots..."
gnuplot -p XhtmlWriteTime.plt
gnuplot -p XhtmlWriteRate.plt
gnuplot -p XhtmlWriteRateHistogram.plt
gnuplot -p XhtmlWriteFriction.plt
gnuplot -p XhtmlWriteFrictionHistogram.plt
echo "All plots done"
