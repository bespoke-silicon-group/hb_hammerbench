apps="cello_aes cello_barnes_hut cello_blackscholes cello_fft256_2d cello_smithwaterman"
for app in $apps; do
    python3 $app/eval/bigblade-tests.py > $app/eval/tests.mk
    make -B -C $app/eval HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done
