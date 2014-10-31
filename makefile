
CC=g++

FLAGS=-Wall -fPIC

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
LIB_DIR=lib

INCLUDE=-Iinclude/ -I/usr/include

_SRC=UrcData.cpp Classifier.cpp VectorMath.cpp Fading.cpp
_OBJ=UrcData.o Classifier.o VectorMath.o Fading.o
LIB=

ifeq ($(DEBUGMODE),1)
	FLAGS+=-g -DDEBUG=1
	LIBNAME=urcd
	LIB=$(LIB_DIR)/liburcd.a
	OBJ_DIR=obj/debug
	OMNETPP_MODE=debug
	OMNETPP_SO=urc_omnetpp_debug
	OMNETPP_OUTPUT_DIR=/gcc-debug
else
	FLAGS+=-O3
	LIBNAME=urc
	LIB=$(LIB_DIR)/liburc.a
	OMNETPP_MODE=release
	OMNETPP_SO=urc_omnetpp
	OMNETPP_OUTPUT_DIR=/gcc-release
endif

URCLIB_SRC=$(patsubst %,$(SRC_DIR)/UrcLib/%,$(_SRC))
URCLIB_OBJ=$(patsubst %,$(OBJ_DIR)/UrcLib/%,$(_OBJ))
URCLIB_SRC_DIR=$(SRC_DIR)/UrcLib
URCLIB_OBJ_DIR=$(OBJ_DIR)/UrcLib

RT_SRC=$(patsubst %,$(SRC_DIR)/Raytracer/%,Raytracer.cpp main.cpp)
RT_OBJ=$(patsubst %,$(OBJ_DIR)/Raytracer/%,Raytracer.o main.o)
RT_SRC_DIR=$(SRC_DIR)/Raytracer
RT_OBJ_DIR=$(OBJ_DIR)/Raytracer
RT_BIN=$(BIN_DIR)/Raytracer
RT_LIBS=-l$(LIBNAME) -lpthread


BS_SRC=$(patsubst %,$(SRC_DIR)/BuildingSolver/%, main.cpp)
BS_OBJ=$(patsubst %,$(OBJ_DIR)/BuildingSolver/%, main.o)
BS_SRC_DIR=$(SRC_DIR)/BuildingSolver
BS_OBJ_DIR=$(OBJ_DIR)/BuildingSolver
BS_BIN=$(BIN_DIR)/BuildingSolver
BS_LIBS=-l$(LIBNAME) -lpthread

RTVIS_SRC=$(patsubst %,$(SRC_DIR)/Raytracer/%,Raytracer.cpp visualiser.cpp)
RTVIS_OBJ=$(patsubst %,$(OBJ_DIR)/Raytracer/%,Raytracer.o visualiser.o)
RTVIS_SRC_DIR=$(SRC_DIR)/Raytracer
RTVIS_OBJ_DIR=$(OBJ_DIR)/Raytracer
RTVIS_BIN=$(BIN_DIR)/RaytraceVisualiser


ifeq ($(USE_VISUALISER),1)
	FLAGS+=-DUSE_VISUALISER
	RT_LIBS+=-lallegro -lallegro_primitives
endif


OMNETPP_SRC_DIR=$(SRC_DIR)/OMNeT++
OMNETPP_OBJ_DIR=$(OBJ_DIR)/OMNeT++
VEINS_DIRS=base/utils modules modules/obstacle modules/utility modules/mobility/traci base/connectionManager base/modules modules/mac modules/mac/ieee80211p modules/mobility modules/messages modules/analogueModel base/phyLayer modules/phy base/messages

LIBRARY=$(LIB_DIR)/$(LIBNAME)

.PHONY: check_veins create_dirs check_install_directory

all : create_dirs Library Raytracer BuildingSolver OMNETPP

create_dirs :
	mkdir -p $(OBJ_DIR)/UrcLib
	mkdir -p $(OBJ_DIR)/Raytracer
	mkdir -p $(OBJ_DIR)/BuildingSolver
	mkdir -p $(OMNETPP_OBJ_DIR)

Library : $(SRC) $(LIB)

$(LIB) : $(URCLIB_OBJ)
	rm -f $@
	ar cq $@ $(URCLIB_OBJ)

$(URCLIB_OBJ_DIR)/%.o : $(URCLIB_SRC_DIR)/%.cpp 
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

Raytracer : create_dirs Library $(RT_SRC) $(RT_BIN)

$(RT_BIN) : $(RT_OBJ)
	$(CC) $(RT_OBJ) -o $(RT_BIN) -L$(LIB_DIR) $(RT_LIBS)

$(RT_OBJ_DIR)/%.o : $(RT_SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

BuildingSolver : create_dirs Library $(BS_SRC) $(BS_BIN)

$(BS_BIN) : $(BS_OBJ)
	$(CC) $(BS_OBJ) -o $(BS_BIN) -L$(LIB_DIR) $(BS_LIBS) -lallegro -lallegro_primitives -lallegro_image -lallegro_font -lallegro_ttf

$(BS_OBJ_DIR)/%.o : $(BS_SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

RaytraceVisualiser : create_dirs Library $(RTVIS_SRC) $(RTVIS_BIN)

$(RTVIS_BIN) : $(RTVIS_OBJ)
	$(CC) $(RTVIS_OBJ) -o $(RTVIS_BIN) -L$(LIB_DIR) $(RT_LIBS) -lallegro -lallegro_primitives -lallegro_image

$(RTVIS_OBJ_DIR)/%.o : $(RTVIS_SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

OMNETPP : check_veins $(LIB)
	cd $(OMNETPP_SRC_DIR); opp_makemake -f -O ../../$(OMNETPP_OBJ_DIR) $(patsubst %,-I ../../$(VEINS_ROOT)/src/%,$(VEINS_DIRS)) -I ../../include -L ../../lib $(RT_LIBS) -s -o $(OMNETPP_SO); make MODE=$(OMNETPP_MODE)
	cp $(OMNETPP_OBJ_DIR)$(OMNETPP_OUTPUT_DIR)/lib$(OMNETPP_SO).so $(LIB_DIR)


check_veins :
	@if [ -z "$(VEINS_ROOT)" ]; then \
		echo "Error: Please specify a root directory for Veins if you're building for OMNeT++"; exit 2; \
	else true; fi

install_OMNETPP : create_dirs check_install_directory OMNETPP
	mkdir -p $(OMNETPP_INSTALL_DIR)/URC
	mkdir -p $(OMNETPP_INSTALL_DIR)/URC/include
	mkdir -p $(OMNETPP_INSTALL_DIR)/URC/lib
	mkdir -p $(OMNETPP_INSTALL_DIR)/URC/ned
	cp $(OMNETPP_SRC_DIR)/*.ned $(OMNETPP_INSTALL_DIR)/URC/ned/
	cp $(LIB_DIR)/*.so $(OMNETPP_INSTALL_DIR)/URC/lib/
	cp include/*.h $(OMNETPP_INSTALL_DIR)/URC/include/
	cp $(OMNETPP_SRC_DIR)/*.h $(OMNETPP_INSTALL_DIR)/URC/include/

check_install_directory :
	@if [ -z "$(OMNETPP_INSTALL_DIR)" ]; then \
		echo "Error: Please specify a simulation workspace directory for installation"; exit 2; \
	else true; fi


clean:
	cd $(OMNETPP_SRC_DIR); make clean; rm -f Makefile lib*.so
	rm -rf $(BIN_DIR)/*
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_DIR)/*
	rm -rf $(OMNETPP_INSTALL_DIR)/URC/lib/*
	rm -rf $(OMNETPP_INSTALL_DIR)/URC/include/*
	rm -rf $(OMNETPP_INSTALL_DIR)/URC/ned/*
	rm -rf $(OMNETPP_INSTALL_DIR)/URC/
