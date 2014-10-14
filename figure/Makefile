P2ROOT = ../
CPIMG = ${P2ROOT}_scripts/cp-img.sh

IMG_FILES := $(patsubst %.png, %.png_, $(wildcard *.png))

all : $(IMG_FILES)

%.png_ : %.png
	$(CPIMG) $< ${P2ROOT}

test :
	echo ${IMG_FILES}
