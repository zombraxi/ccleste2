SDL2_CONFIG=`sdl2-config --cflags`
SDL2_STATIC=`sdl2-config --static-libs` -lSDL2_mixer 

COMPILER=gcc
OUTPUT_NAME=cc2
SOURCE_FILES=celeste2.c p8.c

all: $(SOURCE_FILES) p8.h
	$(COMPILER) $(SOURCE_FILES) -O2 -g -o $(OUTPUT_NAME) $(SDL2_CONFIG) $(SDL2_STATIC)

clean: $(OUTPUT_NAME)
	rm -rf $(OUTPUT_NAME)