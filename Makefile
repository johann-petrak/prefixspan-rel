CC = gcc
CXX = c++
EXECPREFIX =
VERSION = 1.3

CFLAGS = -O3 -Wall
CXXFLAGS = -O3 -Wall -Wno-deprecated
TARGETS = prefixspan${EXEC_PREFIX}
OBJ = prefixspan.o

all: ${OBJ}
	${CXX} ${CFLAGS} ${LDFLAGS} -o ${TARGETS} ${OBJ} ${LDFLAGS}

clean:
	rm -f *.o ${TARGETS} core *~ *.tar.gz

dist:	
	rm -fr prefixspan-rel-${VERSION}	
	mkdir prefixspan-rel-${VERSION}
	cp -R *.cpp Makefile README.md AUTHORS doc-orig data bin prefixspan-rel-${VERSION}
	tar zcfv prefixspan-rel-${VERSION}.tar.gz prefixspan-rel-${VERSION}
	rm -fr prefixspan-rel-${VERSION}

test:	
	./prefixspan -m 4 -c token -S 1 -0 < data/data2 > data/testout/0m4ctokenS1
	./prefixspan -m 4 -c type -S 1 -0 < data/data2 > data/testout/0m4ctypeS1
	./prefixspan -m 4 -c token -S 1  < data/data2 > data/testout/m4ctokenS1
	./prefixspan -m 4 -c type -S 1  < data/data2 > data/testout/m4ctypeS1
	./prefixspan -m 4 -s 1 -S 1 < data/data2 > data/testout/m4s1S1
	./prefixspan -m 4  < data/data2 > data/testout/m4
	./prefixspan -m 4 -S 0 < data/data2 > data/testout/m4S0 
	./prefixspan -m 4 -g 1 -G 1 -s 1 -S 1 < data/data2 > data/testout/m4g1G1s1S1
	./prefixspan -m 7 -c token < data/data2 > data/testout/m7ctoken
	./prefixspan -m 7 -c token -0 < data/data2 > data/testout/m7ctoken0
	./prefixspan -m 5 < data/data2 > data/testout/m5
	./prefixspan -a -m 5 < data/data2 > data/testout/am5
	./prefixspan -w -m 3 -S 1 -G 1 < data/data2 > data/testout/wm3S1G1
	./prefixspan -a -w -m 4 -S 1 -G 1 < data/data2 > data/testout/awm4S1G1
	./prefixspan -m 5 -x < data/data2 > data/testout/m5x
	echo 0m4ctokenS1
	diff -b  data/testout/0m4ctokenS1 data/00test/0m4ctokenS1
	rm -f data/testout/0m4ctokenS1
	echo 0m4ctypeS1
	diff -b  data/testout/0m4ctypeS1 data/00test/0m4ctypeS1
	rm -f data/testout/0m4ctypeS1
	echo m4ctokenS1
	diff -b  data/testout/m4ctokenS1 data/00test/m4ctokenS1
	rm -f data/testout/m4ctokenS1
	echo data/testout/m4ctypeS1
	diff -b  data/testout/m4ctypeS1 data/00test/m4ctypeS1
	rm -f data/testout/m4ctypeS1
	echo m4s1S1
	diff -b  data/testout/m4s1S1 data/00test/m4s1S1
	rm -f data/testout/m4s1S1
	echo m4 
	diff -b  data/testout/m4 data/00test/m4
	rm -f data/testout/m4 
	echo m4S0
	diff -b  data/testout/m4S0 data/00test/m4S0
	rm -f data/testout/m4S0
	echo m4g1G1s1S1
	diff -b  data/testout/m4g1G1s1S1 data/00test/m4g1G1s1S1
	rm -f data/testout/m4g1G1s1S1
	echo m7ctoken
	diff -b  data/testout/m7ctoken data/00test/m7ctoken
	rm -f data/testout/m7ctoken
	echo m7ctoken0
	diff -b  data/testout/m7ctoken0 data/00test/m7ctoken0
	rm -f data/testout/m7ctoken0
	echo m5
	diff -b  data/testout/m5 data/00test/m5
	rm -f data/testout/m5
	echo am5
	diff -b  data/testout/am5 data/00test/am5
	rm -f data/testout/am5
	echo wm3S1G1
	diff -b  data/testout/wm3S1G1 data/00test/wm3S1G1
	rm -f data/testout/wm3S1G1
	echo awm4S1G1
	diff -b  data/testout/awm4S1G1 data/00test/awm4S1G1
	rm -f data/testout/awm4S1G1
	echo m5x
	diff -b data/testout/m5x data/00test/m5x
	rm -f data/testout/m5x
