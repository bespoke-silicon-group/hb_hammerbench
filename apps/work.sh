apps="cello_aes cello_barnes_hut cello_blackscholes cello_fft256_2d cello_smithwaterman cello_spgemm"
for app in $apps; do
    python3 $app/work/bigblade-tests.py > $app/work/tests.mk
    make -C $app/work HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

