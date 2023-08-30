#CC = clang
CXX = clang++
#LINK = clang++

DEFS = -DDIC_VERSION=102 -DVERSION="\"0.996\"" -DPACKAGE="\"mecab\""
INC = -I.
#CFLAGS = -std=c11 -O2 -Wall $(DEFS)
#CXXFLAGS = -std=c++20 -O2 -Wall $(INC) $(DEFS)
CXXFLAGS = -std=c++20 -Os -Wall $(INC) $(DEFS)
#CXXFLAGS = -std=c++20 -O2 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic $(INC) $(DEFS)
RM   := rm -f
TAR  := tar cofJ
FILE := /media/Box/TinyMecab$(shell date +%Y%m%d).tar.xz

SRC = mecab.cpp
HDR += CharProperty.hpp
HDR += Dictionary.hpp
HDR += Lattice.hpp
HDR += Mmap.hpp
HDR += Param.hpp
HDR += Stream.hpp
HDR += Writer.hpp
HDR += mecab.hpp

.PHONY: all
all: mecab test

#.PHONY: mecab
mecab: $(SRC) $(HDR) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

.PHONY: clean
clean:
	$(RM) mecab *.o

.PHONY: tar
tar:
	@$(RM) $(FILE)
	$(TAR) $(FILE) Makefile $(SRC) $(HDR) compile_flags.txt memo.md

DICDIR := /mnt/c/home/mecab/unidic-csj-202302
TXTFILE := ../_.md
GODFILE := /mnt/c/home/tmp/nya.god
CHKFILE := /mnt/c/home/tmp/nya.chk

TXT := '裏道を通って図書館に通ってジョジョの奇妙な冒険を読破したッ!'
OPT := -d $(DICDIR) -r dicrc -b 163840

.PHONY: test
test: mecab $(GODFILE) $(CHKFILE)
	diff $(GODFILE) $(CHKFILE) && echo OK

$(GODFILE): Makefile $(TXTFILE)
	mecab $(OPT) < $(TXTFILE) > $(GODFILE)
	echo "■wakati" >> $(GODFILE)
	mecab $(OPT) -Owakati $(TXTFILE) >> $(GODFILE)
	mecab $(OPT) -Owakach $(TXTFILE) >> $(GODFILE)
	echo "■rby" >> $(GODFILE)
	mecab $(OPT) -Orby $(TXTFILE) >> $(GODFILE)
	echo "■rbx" >> $(GODFILE)
	mecab $(OPT) -Orbx $(TXTFILE) >> $(GODFILE)
	echo "■STDIN" >> $(GODFILE)
	@echo $(TXT) | mecab $(OPT) -Osimple >> $(GODFILE)
	@echo $(TXT) | mecab $(OPT) -Orby >> $(GODFILE)
	@echo $(TXT) | mecab $(OPT) -Orbx >> $(GODFILE)

$(CHKFILE): Makefile ./mecab $(TXTFILE)
	./mecab $(OPT) < $(TXTFILE) > $(CHKFILE)
	echo "■wakati" >> $(CHKFILE)
	./mecab $(OPT) -Owakati $(TXTFILE) >> $(CHKFILE)
	./mecab $(OPT) -Owakach $(TXTFILE) >> $(CHKFILE)
	echo "■rby" >> $(CHKFILE)
	./mecab $(OPT) -Orby $(TXTFILE) >> $(CHKFILE)
	echo "■rbx" >> $(CHKFILE)
	./mecab $(OPT) -Orbx $(TXTFILE) >> $(CHKFILE)
	echo "■STDIN" >> $(CHKFILE)
	@echo $(TXT) | ./mecab $(OPT) -Osimple >> $(CHKFILE)
	@echo $(TXT) | ./mecab $(OPT) -Orby >> $(CHKFILE)
	@echo $(TXT) | ./mecab $(OPT) -Orbx >> $(CHKFILE)
