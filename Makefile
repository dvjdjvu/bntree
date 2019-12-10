CC = gcc
PP = g++

SRC = ./src
BUILD = ./objs

CFLAGS      = -g -O0 #-W -Wall
PFLAGS      = -std=c++11

INCS = -I$(SRC) 

LDLIBS = -O0 -W -Wall

		
DEPSPP = $(SRC)/storage.hpp 

OBJS = $(BUILD)/storage.pp.o \
	$(BUILD)/test.pp.o 
	
OBJSPP = $(BUILD)/storage.pp.o \
	  $(BUILD)/test.pp.o 

BINS = $(BUILD)/test

BINS2 = test 

all: prebuild \
	$(BINS)

$(BUILD)/test: \
	$(BUILD)/test.pp.o \
	$(OBJS)
	$(PP) $(CFLAGS) $(PFLAGS) -o $(BUILD)/test $(LIBS) $(OBJS) $(LDLIBS)

$(BUILD)/storage.pp.o: $(DEPSPP) \
	$(SRC)/storage.cpp
	$(PP) -c $(CFLAGS) $(PFLAGS) $(INCS) -o $(BUILD)/storage.pp.o $(SRC)/storage.cpp

$(BUILD)/test.pp.o: $(DEPSPP) \
	$(SRC)/test.cpp
	$(PP) -c $(CFLAGS) $(PFLAGS) $(INCS) -o $(BUILD)/test.pp.o $(SRC)/test.cpp

clean:
	rm -rf $(BUILD)

prebuild:
	test -d $(BUILD) || mkdir -p $(BUILD)

	