apps="cello_aes cello_barnes_hut cello_blackcholes cello_fft256_2d cello_smithwaterman cello_spgemm"
for app in $apps; do
    python3 $app/scaling/bigblade-tests.py > $app/scaling/tests.mk
    make -C $app/scaling HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

