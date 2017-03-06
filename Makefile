CC = gcc
CXX = c++
EXECPREFIX =
VERSION = 1.2

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
	cp -R *.cpp Makefile README AUTHORS COPYING data data2 data3 stopwords index.htm masayu-a.css 00test prefixspan.exe prefixspan-rel-${VERSION}
	tar zcfv prefixspan-rel-${VERSION}.tar.gz prefixspan-rel-${VERSION}
	rm -fr prefixspan-rel-${VERSION}

test:	
	./prefixspan -m 4 -c token -S 1 -0 < data2 > 0m4ctokenS1
	./prefixspan -m 4 -c type -S 1 -0 < data2 > 0m4ctypeS1
	./prefixspan -m 4 -c token -S 1  < data2 > m4ctokenS1
	./prefixspan -m 4 -c type -S 1  < data2 > m4ctypeS1
	./prefixspan -m 4 -s 1 -S 1 < data2 > m4s1S1
	./prefixspan -m 4  < data2 > m4
	./prefixspan -m 4 -S 0 < data2 > m4S0 
	./prefixspan -m 4 -g 1 -G 1 -s 1 -S 1 < data2 > m4g1G1s1S1
	./prefixspan -m 7 -c token < data2 > m7ctoken
	./prefixspan -m 7 -c token -0 < data2 > m7ctoken0
	./prefixspan -m 5 < data2 > m5
	./prefixspan -a -m 5 < data2 > am5
	./prefixspan -w -m 3 -S 1 -G 1 < data2 > wm3S1G1
	./prefixspan -a -w -m 4 -S 1 -G 1 < data2 > awm4S1G1
	./prefixspan -m 5 -x < data2 > m5x
	echo 0m4ctokenS1
	diff -b  0m4ctokenS1 00test/0m4ctokenS1
	rm -f 0m4ctokenS1
	echo 0m4ctypeS1
	diff -b  0m4ctypeS1 00test/0m4ctypeS1
	rm -f 0m4ctypeS1
	echo m4ctokenS1
	diff -b  m4ctokenS1 00test/m4ctokenS1
	rm -f m4ctokenS1
	echo m4ctypeS1
	diff -b  m4ctypeS1 00test/m4ctypeS1
	rm -f m4ctypeS1
	echo m4s1S1
	diff -b  m4s1S1 00test/m4s1S1
	rm -f m4s1S1
	echo m4 
	diff -b  m4 00test/m4
	rm -f m4 
	echo m4S0
	diff -b  m4S0 00test/m4S0
	rm -f m4S0
	echo m4g1G1s1S1
	diff -b  m4g1G1s1S1 00test/m4g1G1s1S1
	rm -f m4g1G1s1S1
	echo m7ctoken
	diff -b  m7ctoken 00test/m7ctoken
	rm -f m7ctoken
	echo m7ctoken0
	diff -b  m7ctoken0 00test/m7ctoken0
	rm -f m7ctoken0
	echo m5
	diff -b  m5 00test/m5
	rm -f m5
	echo am5
	diff -b  am5 00test/am5
	rm -f am5
	echo wm3S1G1
	diff -b  wm3S1G1 00test/wm3S1G1
	rm -f wm3S1G1
	echo awm4S1G1
	diff -b  awm4S1G1 00test/awm4S1G1
	rm -f awm4S1G1
