SDL2_CONFIG=`sdl2-config --cflags`
SDL2_STATIC=`sdl2-config --static-libs` -lSDL2_mixer 
SDL2_DYN=`sdl2-config --libs` -lSDL2_mixer

COMPILER=gcc
OUTPUT_NAME=cc2
SOURCE_FILES=celeste2.c p8.c

all: $(SOURCE_FILES) p8.h celeste2.h gamedata.h
	$(COMPILER) $(SOURCE_FILES) -O2 -g -o $(OUTPUT_NAME) $(SDL2_CONFIG) $(SDL2_DYN)

static: $(SOURCE_FILES) p8.h celeste2.h gamedata.h
	$(COMPILER) $(SOURCE_FILES) -O2 -g -o $(OUTPUT_NAME) $(SDL2_CONFIG) $(SDL2_STATIC)

release: $(SOURCE_FILES) p8.h celeste2.h gamedata.h
	$(COMPILER) $(SOURCE_FILES) -O2 -o $(OUTPUT_NAME) $(SDL2_CONFIG) $(SDL2_DYN)
	
release_static: $(SOURCE_FILES) p8.h celeste2.h gamedata.h
	$(COMPILER) $(SOURCE_FILES) -O2 -o $(OUTPUT_NAME) $(SDL2_CONFIG) $(SDL2_STATIC)

clean: $(OUTPUT_NAME)
	rm -rf $(OUTPUT_NAME)
