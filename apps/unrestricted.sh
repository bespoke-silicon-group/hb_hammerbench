apps="cello_aes cello_barnes_hut cello_blackscholes cello_smithwaterman cello_fft256_2d"
for app in $apps; do
    python3 $app/unrestricted/bigblade-tests.py > $app/unrestricted/tests.mk
    make -C $app/unrestricted HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

