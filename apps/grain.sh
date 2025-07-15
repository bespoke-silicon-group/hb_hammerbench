apps="cello_aes cello_barnes_hut cello_blackscholes cello_smithwaterman cello_spgemm"
for app in $apps; do
    python3 $app/grain/bigblade-tests.py > $app/grain/tests.mk
    make -C $app/grain HB_MC_DEVICE_ID=$MY_UNIT_ID report.csv
done

