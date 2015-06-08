# Project: zRuijie4GZHU
# Makefile created by Dev-C++ 4.9.9.2

CPP  = i486-mingw32-g++
CC   = i486-mingw32-gcc
WINDRES = i486-mingw32-windres
RES  = window.res
OBJ  = win_main.o eap_protocol.o md5.o zdclient.o $(RES)
LINKOBJ  = $(OBJ)
LIBS =  -L"/usr/i486-mingw32/lib" -L"WpdPack/Lib" -lcomctl32 -lwpcap -lws2_32 -liphlpapi -mwindows 
INCS =  -I"/usr/i486-mingw32/include"  -I"WpdPack/Include"
CXXINCS = 
BIN  = ZDClient.exe
CXXFLAGS = $(CXXINCS)   -fexpensive-optimizations -O2
CFLAGS = $(INCS)  -Wall -Os
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN) *~

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

win_main.o: win_main.c
	$(CC) -c win_main.c -o win_main.o $(CFLAGS)

blog.o: blog.c
	$(CC) -c blog.c -o blog.o $(CFLAGS)

eap_protocol.o: eap_protocol.c
	$(CC) -c eap_protocol.c -o eap_protocol.o $(CFLAGS)

md5.o: md5.c
	$(CC) -c md5.c -o md5.o $(CFLAGS)

zdclient.o: zdclient.c
	$(CC) -c zdclient.c -o zdclient.o $(CFLAGS)

window.res: window.rc 
	$(WINDRES) -i window.rc --input-format=rc -o window.res -O coff 
