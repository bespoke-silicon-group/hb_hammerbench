HB_HAMMERBENCH_PATH := $(shell git rev-parse --show-toplevel)

from_cse += g7k1.mtx
from_cse += g10k4.mtx
from_cse += g10k8.mtx
from_cse += g10k16.mtx
from_cse += g11k4.mtx
from_cse += g11k8.mtx
from_cse += g11k16.mtx
from_cse += g12k4.mtx
from_cse += g12k8.mtx
from_cse += g12k16.mtx
from_cse += g13k4.mtx
from_cse += g13k8.mtx
from_cse += g13k16.mtx
from_cse += g14k4.mtx
from_cse += g14k8.mtx
from_cse += g14k16.mtx
from_cse += g15k4.mtx
from_cse += g15k8.mtx
from_cse += g15k16.mtx
from_cse += g16k4.mtx
from_cse += g16k8.mtx
from_cse += g16k16.mtx
from_cse += u7k1.mtx
from_cse += u10k4.mtx
from_cse += u10k8.mtx
from_cse += u10k16.mtx
from_cse += u11k4.mtx
from_cse += u11k8.mtx
from_cse += u11k16.mtx
from_cse += u12k4.mtx
from_cse += u12k8.mtx
from_cse += u12k16.mtx
from_cse += u13k4.mtx
from_cse += u13k8.mtx
from_cse += u13k16.mtx
from_cse += u14k4.mtx
from_cse += u14k8.mtx
from_cse += u14k16.mtx
from_cse += u15k4.mtx
from_cse += u15k8.mtx
from_cse += u15k16.mtx
from_cse += u16k4.mtx
from_cse += u16k8.mtx
from_cse += u16k16.mtx

from_cse_mtx := $(addprefix $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/,$(from_cse))
from_cse_tar := $(addsuffix .tar.gz,$(from_cse_mtx))

$(from_cse_tar):
	$(eval tarfile=$(notdir $@))
	$(eval url=https://homes.cs.washington.edu/~mrutt/sparse_inputs/$(tarfile))
	wget --inet4-only -O $@ $(url)

$(from_cse_mtx):  $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.mtx: $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.mtx.tar.gz
	tar zxf $< -C $(dir $@)


from_snap += wiki-Vote.mtx
from_snap += email-Enron.mtx
from_snap += web-NotreDame.mtx
from_snap += web-Stanford.mtx
from_snap += web-Google.mtx
from_snap += web-BerkStan.mtx
from_snap += roadNet-CA.mtx
from_snap += roadNet-PA.mtx
from_snap += roadNet-TX.mtx

from_snap_mtx := $(addprefix $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/,$(from_snap))
from_snap_tar := $(patsubst %.mtx,%.tar.gz,$(from_snap_mtx))

$(from_snap_tar):
	$(eval tarfile=$(notdir $@))
	$(eval url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/$(tarfile))
	wget --inet4-only -O $@ $(url)

$(from_snap_mtx):  $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.mtx: $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.tar.gz
	tar zxf $< -C $(dir $@)
	rm -f $@
	ln -s $(dir $@)/$*/$*.mtx $@

from_um += offshore.mtx

from_um_mtx := $(addprefix $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/,$(from_um))
from_um_tar := $(patsubst %.mtx,%.tar.gz,$(from_um_mtx))

$(from_um_tar):
	$(eval tarfile=$(notdir $@))
	$(eval url=https://suitesparse-collection-website.herokuapp.com/MM/Um/$(tarfile))
	wget --inet4-only -O $@ $(url)

$(from_um_mtx):  $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.mtx: $(HB_HAMMERBENCH_PATH)/inputs/sparse-inputs/%.tar.gz
	tar zxf $< -C $(dir $@)
	rm -f $@
	ln -s $(dir $@)/$*/$*.mtx $@

