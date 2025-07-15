apps="cello_aes cello_barnes_hut cello_blackscholes cello_smithwaterman cello_fft256_2d cello_spgemm"
for app in $apps; do
    python3 $app/multispawn/bigblade-tests.py > $app/multispawn/tests.mk
    make -C $app/multispawn HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

