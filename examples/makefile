# TODO: Currently only compiles the showobj example with the gcc toolchain...
CC=g++
CFLAGS=-c -std=c++14 #-Wall # sadly we are having a lot of unused functions for tests so this is undesirable...
LDFLAGS=-lglut -lGLESv2 -lm
SOURCES=showobj.cpp objmaster/Obj.cpp objmaster/VertexElement.cpp objmaster/VertexNormalElement.cpp objmaster/VertexTextureElement.cpp objmaster/FaceElement.cpp objmaster/FacePoint.cpp objmaster/ObjMeshObject.cpp objmaster/Material.cpp objmaster/TextureDataHoldingMaterial.cpp objmaster/ObjectGroupElement.cpp objmaster/MtlLib.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=showobj

all: $(SOURCES) $(EXECUTABLE)

# The LDFLAGS should be after the files
# because of some lazyness in newer toolchains!
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o objmaster/*.o showobj
