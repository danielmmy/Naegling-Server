## Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = naegling
AM_LDFLAGS = -lvirt -lpthread -lssl -lcrypto -lsqlite3 $(BOOST_LIB_SUFFIX) 

naegling_SOURCES = naegling-main.c naegling-main.h vm-control.c vm-control.h naegling-com.c naegling-com.h db-control-sqlite.c db-control-sqlite.h
