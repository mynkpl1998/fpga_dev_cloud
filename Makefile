COMPILER=g++
AOCL_COMPILE_CONFIG=$(shell aocl compile-config)
AOCL_LINK_CONFIG=$(shell aocl link-config)

all: main.cpp
	${COMPILER} main.cpp ${AOCL_COMPILE_CONFIG} ${AOCL_LINK_CONFIG}