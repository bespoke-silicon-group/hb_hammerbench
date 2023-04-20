ifndef APPLICATION_PATH
$(error "APPLICATION_PATH not defined. Please define this variable")
endif
INPUTS_DIR = $(APPLICATION_PATH)/inputs


###################
# Empty 1024x1024 #
###################
empty1024		= $(INPUTS_DIR)/empty1024/empty1024.mtx
empty1024__directed	= yes
empty1024__weighted	= yes
empty1024__zero-indexed = no
empty1024__rows         = 1024
empty1024__cols         = 1024
empty1024__nnz          = 0
empty1024__solnnz       = 0
#$(empty1024):  url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/wiki-Vote.tar.gz
#$(empty1024):  tar=wiki-Vote.tar.gz


###################
# Wiki-Vote graph #
###################
INPUTS                 += wiki-vote
wiki-vote		= $(INPUTS_DIR)/wiki-Vote/wiki-Vote.mtx
wiki-vote__directed	= yes
wiki-vote__weighted	= no
wiki-vote__zero-indexed = no
wiki-vote__rows         = 8297
wiki-vote__cols         = 8297
wiki-vote__nnz          = 103689
wiki-vote__solnnz       = 1831112
$(wiki-vote):  url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/wiki-Vote.tar.gz
$(wiki-vote):  tar=wiki-Vote.tar.gz

##############
# poisson-3D #
##############
INPUTS              	       += poisson-3D
poisson-3D			= $(INPUTS_DIR)/poisson3Da/poisson3Da.mtx
poisson-3D__directed		= yes
poisson-3D__weighted		= yes
poisson-3D__zero-indexed	= no
poisson-3D__rows 		= 13514
poisson-3D__cols 		= 13514
poisson-3D__nnz 		= 352762
poisson-3D__solnnz 		= 2957530
$(poisson-3D): url=https://suitesparse-collection-website.herokuapp.com/MM/FEMLAB/poisson3Da.tar.gz
$(poisson-3D): tar=poisson3Da.tar.gz

##############
# ca-CondMat #
##############
INPUTS              	       += ca-CondMat
ca-CondMat			= $(INPUTS_DIR)/ca-CondMat/ca-CondMat.mtx
ca-CondMat__directed		= no
ca-CondMat__weighted		= no
ca-CondMat__zero-indexed	= no
ca-CondMat__rows 		= 23133
ca-CondMat__cols 		= 23133
ca-CondMat__nnz 		= 186936
ca-CondMat__solnnz 		= 2355437
$(ca-CondMat): url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/ca-CondMat.tar.gz
$(ca-CondMat): tar=ca-CondMat.tar.gz

##############
# offshore #
##############
INPUTS              	       += offshore
offshore			= $(INPUTS_DIR)/offshore/offshore.mtx
offshore__directed		= no
offshore__weighted		= yes
offshore__zero-indexed	= no
offshore__rows 		= 259789
offshore__cols 		= 259789
offshore__nnz 		= 4242673
offshore__solnnz	= 23356037
$(offshore): url=https://suitesparse-collection-website.herokuapp.com/MM/Um/offshore.tar.gz
$(offshore): tar=offshore.tar.gz

############
# 08blocks #
############
INPUTS += 08blocks
08blocks = $(INPUTS_DIR)/08blocks/08blocks.mtx
08blocks__directed = yes
08blocks__weighted = yes
08blocks__zero-indexed = no
08blocks__rows = 300
08blocks__cols = 300
08blocks__nnz  = 592
08blocks__solnnz  = 885
$(08blocks): url=https://suitesparse-collection-website.herokuapp.com/MM/JGD_SPG/08blocks.tar.gz
$(08blocks): tar=08blocks.tar.gz

############
# bcsstk20 #
############
INPUTS += bcsstk20
bcsstk20 = $(INPUTS_DIR)/bcsstk20/bcsstk20.mtx
bcsstk20__directed = no
bcsstk20__weighted = yes
bcsstk20__zero-indexed = no
bcsstk20__rows =  485
bcsstk20__cols =  485
bcsstk20__nnz  = 3135
bcsstk20__solnnz  = 5527
$(bcsstk20): url=https://suitesparse-collection-website.herokuapp.com/MM/HB/bcsstk20.tar.gz
$(bcsstk20): tar=bcsstk20.tar.gz

##############
# roadNet-CA #
##############
INPUTS += roadNet-CA
roadNet-CA = $(INPUTS_DIR)/roadNet-CA/roadNet-CA.mtx
roadNet-CA__directed = no
roadNet-CA__weighted = no
roadNet-CA__zero-indexed = no
roadNet-CA__rows = 1971281
roadNet-CA__cols = 1971281
roadNet-CA__nnz  = 5533214
roadNet-CA__solnnz  = 12908454
$(roadNet-CA): url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/roadNet-CA.tar.gz
$(roadNet-CA): tar=roadNet-CA.tar.gz

################
# road_central #
################
INPUTS += road-central
road-central = $(INPUTS_DIR)/road_central/road_central.mtx
road-central__directed = no
road-central__weighted = no
road-central__zero-indexed = no
road-central__rows = 14081816
road-central__cols = 14081816
road-central__nnz  = 33866826
road-central__solnnz  = 69990720
$(road-central): url=https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/road_central.tar.gz
$(road-central): tar=road_central.tar.gz

############
# road_usa #
############
INPUTS += road-usa
road-usa = $(INPUTS_DIR)/road_usa/road_usa.mtx
road-usa__directed = no
road-usa__weighted = no
road-usa__zero-indexed = no
road-usa__rows = 23947347
road-usa__cols = 23947347
road-usa__nnz  = 57708624
road-usa__solnnz  = 119546813
$(road-usa): url=https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/road_usa.tar.gz
$(road-usa): tar=road_usa.tar.gz

##############
# web-Google #
##############
INPUTS += web-Google
web-Google = $(INPUTS_DIR)/web-Google/web-Google.mtx
web-Google__directed = yes
web-Google__weighted = no
web-Google__zero-indexed = no
web-Google__rows =  916428
web-Google__cols =  916428
web-Google__nnz  = 5105039
web-Google__solnnz  = 29710165
$(web-Google): url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/web-Google.tar.gz
$(web-Google): tar=web-Google.tar.gz

##############
# amazon0312 #
##############
INPUTS += amazon0312
amazon0312 = $(INPUTS_DIR)/amazon0312/amazon0312.mtx
amazon0312__directed = yes
amazon0312__weighted = no
amazon0312__zero-indexed = no
amazon0312__rows =      400727
amazon0312__cols =      400727
amazon0312__nnz  =     3200440
amazon0312__solnnz  = 14390556
$(amazon0312): url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/amazon0312.tar.gz
$(amazon0312): tar=amazon0312.tar.gz

#########
# Pokec #
#########
INPUTS += soc-Pokec
soc-Pokec = $(INPUTS_DIR)/soc-Pokec/soc-Pokec.mtx
soc-Pokec__directed = yes
soc-Pokec__weighted = no
soc-Pokec__zero-indexed = no
soc-Pokec__rows =  1632803
soc-Pokec__cols =  1632803
soc-Pokec__nnz  = 30622564
soc-Pokec__solnnz  = 0
$(soc-Pokec): url=https://suitesparse-collection-website.herokuapp.com/MM/SNAP/soc-Pokec.tar.gz
$(soc-Pokec): tar=soc-Pokec.tar.gz

##############
# Hollywood  #
##############
INPUTS += hollywood
hollywood = $(INPUTS_DIR)/hollywood-2009/hollywood-2009.mtx
hollywood__directed = no
hollywood__weighted = no
hollywood__zero-indexed = no
hollywood__rows =   1139905
hollywood__cols =   1139905
hollywood__nnz  = 113891327
hollywood__solnnz  = 0
$(hollywood): url=https://suitesparse-collection-website.herokuapp.com/MM/LAW/hollywood-2009.tar.gz
$(hollywood): tar=hollywood-2009.tar.gz

###############
# LiveJournal #
###############
INPUTS += LiveJournal
LiveJournal = $(INPUTS_DIR)/ljournal-2008/ljournal-2008.mtx
LiveJournal__directed = yes
LiveJournal__weighted = no
LiveJournal__zero-indexed = no
LiveJournal__rows =  5363260
LiveJournal__cols =  5363260
LiveJournal__nnz  = 79023142
LiveJournal__solnnz  = 0
$(LiveJournal): url=https://suitesparse-collection-website.herokuapp.com/MM/LAW/ljournal-2008.tar.gz
$(LiveJournal): tar=ljournal-2008.tar.gz

#########
# U12K4 #
#########
INPUTS += u12k4
u12k4 = $(INPUTS_DIR)/u12k4.mtx
u12k4__directed = yes
u12k4__weighted = yes
u12k4__zero-indexed = no
u12k4__rows = 4096
u12k4__cols = 4096
u12k4__nnz  = 32730
u12k4__solnnz = 0
$(u12k4): url=https://drive.google.com/file/d/1wWd42ZjZEsbyknE7LrC375unaW28LeHk/view?usp=share_link
$(u12k4): tar=u12k4.mtx.tar.gz

########
# G7K1 #
########
INPUTS += g7k1
g7k1 = $(INPUTS_DIR)/g7k1.mtx
g7k1__directed = yes
g7k1__weighted = yes
g7k1__zero-indexed = no
g7k1__rows = 128
g7k1__cols = 128
g7k1__nnz  = 234
g7k1__solnnz = 0
$(g7k1): url=https://drive.google.com/file/d/1vFrohxOijg-HjckN0PTN22KGXlVJTtis/view?usp=share_link
$(g7k1): tar=g7k1.mtx.tar.gz

########
# U7K1 #
########
INPUTS += u7k1
u7k1 = $(INPUTS_DIR)/u7k1.mtx
u7k1__directed = yes
u7k1__weighted = yes
u7k1__zero-indexed = no
u7k1__rows = 128
u7k1__cols = 128
u7k1__nnz  = 254
u7k1__solnnz = 0
$(u7k1): url=https://drive.google.com/file/d/1hefg63DFUdCSs-dE8vOgX-0komVS_nXS/view?usp=share_link
$(u7k1): tar=u7k1.mtx.tar.gz

########
# U8K2 #
########
INPUTS += u8k2
u8k2 = $(INPUTS_DIR)/u8k2.mtx
u8k2__directed = yes
u8k2__weighted = yes
u8k2__zero-indexed = no
u8k2__rows = 256
u8k2__cols = 256
u8k2__nnz  = 1016
u8k2__solnnz = 0
$(u8k2): url=https://drive.google.com/file/d/1fWJSVA8-pQ64tPBklhpJhIkUe0t_1sKD/view?usp=share_link
$(u8k2): tar=u8k2.mtx.tar.gz

#########
# U12K2 #
#########
INPUTS += u12k2
u12k2 = $(INPUTS_DIR)/u12k2.mtx
u12k2__directed = yes
u12k2__weighted = yes
u12k2__zero-indexed = no
u12k2__rows = 4096
u12k2__cols = 4096
u12k2__nnz  = 16372
u12k2__solnnz = 0
$(u12k2): url=https://drive.google.com/file/d/1MYFVdQxPik4l-KHJlI6k4T5ym_M6DkN3/view?usp=share_link
$(u12k2): tar=u12k2.mtx.tar.gz

#########
# U16K8 #
#########
INPUTS += u16k8
u16k8 = $(INPUTS_DIR)/u16k8.mtx
u16k8__directed = yes
u16k8__weighted = yes
u16k8__zero-indexed = no
u16k8__rows = 65536
u16k8__cols = 65536
u16k8__nnz  = 1048404
u16k8__solnnz = 0
$(u16k8): url=https://drive.google.com/file/d/1BFoWWPy_HR8KuZdWi9SqaxqF7C7blyrq/view?usp=share_link
$(u16k8): tar=u16k8.mtx.tar.gz

# Download and unpack
$(foreach i,$(INPUTS),$($i)):
	@echo "Downloading and unpacking $@"
	@mkdir -p $(INPUTS_DIR)
	@cd $(INPUTS_DIR) && wget $(url)
	@cd $(INPUTS_DIR) && tar zxf $(tar)
	@touch $@ # updates the timecode of the untared file

$(empty1024):
	@echo "Generating $@"
	@mkdir -p $(INPUTS_DIR)
	@mkdir -p $(dirname $@)
	@echo "%%MatrixMarket matrix coordinate integer general" > $@
	@echo "1024 1024 0" >> $@

$(u16k8) $(u12k4) $(u12k2) $(g7k1) $(u7k1) $(u8k2):
	@echo "Generating $@"
	@mkdir -p $(INPUTS_DIR)
	@cd $(INPUTS_DIR) && gdown --fuzzy $(url)
	@cd $(INPUTS_DIR) && tar zxf $(tar)

inputs: $(foreach i,$(INPUTS),$($i))
inputs: $(empty1024)

clean.inputs:
	rm -rf $(INPUTS_DIR)
