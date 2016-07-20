# TODO: Currently only compiles the showobj example
# TODO: used models should be bundled in case of emscripten

# for gcc5.1+:
#CC=g++
# for clang3.4+:
CC=clang++
# for emscripten / webgl
#CC=em++

ifeq ($(CC),g++)
	# g++ (ver 5.1+ tested)
	CFLAGS=-c -std=c++14 #-Wall # sadly we are having a lot of unused functions for tests so this is undesirable...
	LDFLAGS=-lglut -lGLESv2 -lm
else 
ifeq ($(CC),em++)
	# em++ (ver 1.3+ tested)
	# TODO: added debug symbols
	# use 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent gcc toolchain than the 4.9 at my work!)
	CFLAGS=-c -stdlib=libc++ -std=c++1y -O2
	LDFLAGS=-lglut -stdlib=libc++ -O2
else
	# em++ / clang++ (ver 3.4+ tested)
	# em++ clang uses 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent gcc toolchain than the 4.9 at my work!)
	CFLAGS=-c -stdlib=libc++ -std=c++1y -g -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS=-lglut -lGLESv2 -lm -stdlib=libc++ -g -fsanitize=address -fsanitize-memory-track-origins
endif
endif

SOURCES=showobj.cpp objmaster/Obj.cpp objmaster/VertexElement.cpp objmaster/VertexNormalElement.cpp objmaster/VertexTextureElement.cpp objmaster/FaceElement.cpp objmaster/FacePoint.cpp objmaster/ObjMeshObject.cpp objmaster/Material.cpp objmaster/TextureDataHoldingMaterial.cpp objmaster/ObjectGroupElement.cpp objmaster/MtlLib.cpp objmaster/FileAssetLibrary.cpp objmaster/MaterializedObjModel.cpp objmaster/MaterializedObjMeshObject.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=showobj

all: $(SOURCES) $(EXECUTABLE)

# The LDFLAGS should be after the files
# because of some lazyness in newer toolchains!
$(EXECUTABLE): $(OBJECTS) 
ifeq ($(CC),em++)
	# In case of emscripten build, we make a html5/webgl output
	$(CC) $(OBJECTS) -o $@.html $(LDFLAGS)
else
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
endif

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o objmaster/*.o showobj
