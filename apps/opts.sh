apps="cello_aes cello_barnes_hut cello_blackscholes cello_fft256_2d cello_smithwaterman cello_spgemm"
for app in $apps; do
    python3 $app/opts/bigblade-tests.py --runs=10 > $app/opts/tests.mk
    make -C $app/opts HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done
