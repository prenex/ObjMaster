# for gcc5.1+:
CC=g++
# for clang3.4+:
#CC=clang++
# for emscripten / webgl
#CC=em++

ifeq ($(CC),g++)
	# g++ (ver 5.1+ tested)
	CFLAGS=-c -std=c++14 -g # -O0 #-Wall # sadly we are having a lot of unused functions for tests so this is undesirable...
	LDFLAGS=-lglut -lEGL -lX11 -lGLEW -lGLESv2 -lm -g # -O0 # TODO: added glew2 only for testing now!!!
else 
ifeq ($(CC),em++)
	# em++ (ver 1.3+ tested)
	# TODO: added debug symbols
	# use 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent gcc toolchain than the 4.9 at my work!)
	CFLAGS=-c -stdlib=libc++ -std=c++1y -O2
	LDFLAGS=--shell-file shell_minimal.html --preload-file models -lglut -stdlib=libc++ -O2 -s TOTAL_MEMORY=167772160
else
	# clang++ (ver 3.4+ tested)
	# clang uses 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent gcc toolchain than the 4.9 at my work!)
	CFLAGS=-c -stdlib=libc++ -std=c++1y -g #-O0 -fno-omit-frame-pointer # -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS=-lglut -lGLESv2 -lm -stdlib=libc++ -g -O2 # -fsanitize=address -fsanitize-memory-track-origins
endif
endif

SOURCES=showobj.cpp objmaster/Obj.cpp objmaster/VertexElement.cpp objmaster/VertexNormalElement.cpp objmaster/VertexTextureElement.cpp objmaster/FaceElement.cpp objmaster/FacePoint.cpp objmaster/ObjMeshObject.cpp objmaster/Material.cpp objmaster/TextureDataHoldingMaterial.cpp objmaster/ObjectGroupElement.cpp objmaster/MtlLib.cpp objmaster/FileAssetLibrary.cpp objmaster/MaterializedObjMeshObject.cpp objmaster/StbImgTexturePreparationLibrary.cpp objmaster/ext/GlGpuTexturePreparationLibrary.cpp objmaster/ext/integration/ObjMasterIntegrationFacade.cpp objmaster/LineElement.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=showobj

all: $(SOURCES) $(EXECUTABLE)

# The LDFLAGS should be after the files
# because of some lazyness in newer toolchains!
$(EXECUTABLE): $(OBJECTS) 
# In case of emscripten build, we make a html5/webgl output
ifeq ($(CC),em++)
	$(CC) $(OBJECTS) -o $@.html $(LDFLAGS)
else
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
endif

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o objmaster/*.o showobj objmaster/ext/*.o objmaster/ext/integration/*.o
