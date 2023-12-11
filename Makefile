CPPOPT=-g -D_DEBUG
# -O2 -Os -Ofast
# -fprofile-generate -fprofile-use
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -std=c++11
# -Wparentheses -Wno-unused-parameter -Wformat-security
# -fno-rtti -std=c++11 -std=c++98

# documents and scripts
DOCS=Tasks.txt
SCRS=

# headers and code sources
HDRS=	defs.h \
		Iterator.h Scan.h Filter.h Sort.h \
		Dram.h Table.h tol.h Ssd.h verification.h
SRCS=	defs.cpp Assert.cpp \
		Iterator.cpp Scan.cpp Filter.cpp Sort.cpp \
		Dram.cpp Table.cpp tol.cpp Ssd.cpp Main.cpp verification.cpp

# compilation targets
OBJS=	defs.o Assert.o \
		Iterator.o Scan.o Filter.o Sort.o \
		Dram.o Table.o tol.o Ssd.o Main.o verification.o

# RCS assists
REV=-q -f
MSG=no message

# default target
#
Sort.exe : Makefile $(OBJS)
	g++ $(CPPFLAGS) -o Sort.exe $(OBJS)

trace : Sort.exe Makefile
	@date > trace
	./Sort.exe >> trace
	@size -t Sort.exe $(OBJS) | sort -r >> trace

$(OBJS) : Makefile defs.h
Test.o : Iterator.h Scan.h Filter.h Sort.h
Iterator.o Scan.o Filter.o Sort.o : Iterator.h
Scan.o : Scan.h
Filter.o : Filter.h
Sort.o : Sort.h

list : Makefile
	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
count : list
	@wc `cat list`

ci :
	ci $(REV) -m"$(MSG)" $(HDRS) $(SRCS) $(DOCS) $(SCRS)
	ci -l $(REV) -m"$(MSG)" Makefile
co :
	co $(REV) -l $(HDRS) $(SRCS) $(DOCS) $(SCRS)

clean :
	@rm -rf $(OBJS) Sort.exe Sort.exe.stackdump trace
