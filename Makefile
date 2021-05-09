# File: Makefile
# Author(s): Peter Urgoš (xurgos00)
# Date: 9.5.2021

SRC=src
BUILD=build
DOC=doc

.PHONY: all run doc pack pre_req clean

all: pre_req
	cd $(BUILD) && make

run: all
	cd $(BUILD) && ./ICP

doc:
	cd src/ && doxygen

pack:
	zip -r xurgos00.zip Makefile README.txt $(SRC) examples/

pre_req:
	mkdir -p $(BUILD) && cd $(BUILD) && qmake ../$(SRC)

clean:
	rm -rf $(DOC) $(BUILD)

