apps="aes barnes_hut blackscholes fft/256 smithwaterman spgemm"
for app in $apps; do
    python3 $app/bigblade-tests.py > $app/tests.mk
    make -C $app HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

apps="cello_aes cello_barnes_hut cello_blackcholes cello_fft256_2d cello_smithwaterman cello_spgemm"
for app in $apps; do
    python3 $app/eval/bigblade-tests.py > $app/eval/tests.mk
    make -C $app/eval HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done
