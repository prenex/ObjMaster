# ####### #
# SOURCES #
# ####### #

# ifeq ($(CC),g++)
# 	# g++ (ver 5.1+ tested)
# 	CFLAGS=-c -std=c++14 -g # -O0 #-Wall # sadly we are having a lot of unused functions for tests so this is undesirable...
# 	LDFLAGS=-lglut -lEGL -lX11 -lGLEW -lGLESv2 -lm -g # -O0 # TODO: added glew2 only for testing now!!!
# else 
# ifeq ($(CC),em++)
# 	# em++ (ver 1.3+ tested)
# 	# use 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent clang toolchain than the 4.9 at my work!)
# 	CFLAGS=-c -stdlib=libc++ -std=c++1y -O2
# 	LDFLAGS=--shell-file shell_minimal.html --preload-file models -lglut -stdlib=libc++ -O2 -s TOTAL_MEMORY=167772160
# else
# 	# clang++ (ver 3.4+ tested)
# 	# clang uses 1y instead of 14 here, we need the LLVM libc so that streams are copyable (or a more recent gcc toolchain than the 4.9 at my work!)
# 	CFLAGS=-c -stdlib=libc++ -std=c++1y -g #-O0 -fno-omit-frame-pointer # -fsanitize=address -fno-omit-frame-pointer
# 	LDFLAGS=-lglut -lGLESv2 -lm -stdlib=libc++ -g -O2 # -fsanitize=address -fsanitize-memory-track-origins
# endif
# endif

SOURCES=showobj.cpp objmaster/Obj.cpp objmaster/VertexElement.cpp objmaster/VertexNormalElement.cpp objmaster/VertexTextureElement.cpp objmaster/FaceElement.cpp objmaster/FacePoint.cpp objmaster/ObjMeshObject.cpp objmaster/Material.cpp objmaster/TextureDataHoldingMaterial.cpp objmaster/ObjectGroupElement.cpp objmaster/MtlLib.cpp objmaster/FileAssetLibrary.cpp objmaster/MaterializedObjMeshObject.cpp objmaster/StbImgTexturePreparationLibrary.cpp objmaster/ext/GlGpuTexturePreparationLibrary.cpp objmaster/ext/integration/ObjMasterIntegrationFacade.cpp objmaster/LineElement.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=showobj

# TARGETS #
# ####### #
#
# We are using target specific variable values below to support multiple compilers and environment.
# They are non-global variables, but apply to all rules recursively starting from the definition point.
#
# Rem.: Much more architectures are supported (including windows, dx, Unity3D, etc.) but the example app
#       is made to build for these targets. Feel free to add further examples, I cannot share company code!

# A probably good default target for most
default: gnu_glut

# g++ (ver 5.1+ tested)
gnu_glut: CC=g++
gnu_glut: CFLAGS=-c -std=c++14 -DUSE_FULL_GL=1 -DGLES2_HELPER_USE_GLUT -g
gnu_glut: LDFLAGS=-lGLESv2 -lGLEW -lglut -lm -g -Wall -Wconversion -Wextra
gnu_glut: build_exec

# clang++ (ver 4.9+)
clang_glut: CC=clang++
clang_glut: CFLAGS=-c -std=c++1y -DUSE_FULL_GL=1 -DGLES2_HELPER_USE_GLUT -g
clang_glut: LDFLAGS=-lglut -lGLEW -lGLESv2 -lm -g -O2 
clang_glut: build_exec

# clang++ (ver 3.4+)
clang_old_glut: CC=clang++
clang_old_glut: CFLAGS=-c -stdlib=libc++ -std=c++1y -DUSE_FULL_GL=1 -DGLES2_HELPER_USE_GLUT -g
clang_old_glut: LDFLAGS=-lglut -lGLEW -lGLESv2 -lm -stdlib=libc++ -g -O2 
clang_old_glut: build_exec

# g++ (ver 5.1+ tested)
gnu_egl: CC=g++
gnu_egl: CFLAGS=-c -std=c++14 -DUSE_GLES2=1 -DGLES2_HELPER_USE_EGL -g
gnu_egl: LDFLAGS=-lGLESv2 -lEGL -lX11 -lm -g
gnu_egl: build_exec

# !!! 16bin INDICES !!! #
# Orange and Raspberry Pi: They work only with 16bit indices. Used g++ (ver 5.1+ tested)
pi: CC=g++
pi: CFLAGS=-c -std=c++14 -DUSE_GLES2=1 -DGLES2_HELPER_USE_EGL -DUSE_16BIT_INDICES=1 -g
pi: LDFLAGS=-lGLESv2 -lEGL -lX11 -lm -g
pi: build_exec

# clang++ (ver 4.9+)
clang_egl: CC=clang++
clang_egl: CFLAGS=-c -std=c++1y -DUSE_GLES2=1 -DGLES2_HELPER_USE_EGL -g
clang_egl: LDFLAGS=-lGLESv2 -lEGL -lX11 -lm -g -O2
clang_egl: build_exec

# clang++ (ver 3.4+)
clang_old_egl: CC=clang++
clang_old_egl: CFLAGS=-c -stdlib=libc++ -std=c++1y -DUSE_GLES2=1 -DGLES2_HELPER_USE_EGL -g
clang_old_egl: LDFLAGS=-lGLESv2 -lEGL -lX11 -lm -stdlib=libc++ -g -O2
clang_old_egl: build_exec

# em++ (later toolchains)
# Rem: Change TOTAL_MEMORY for the value it is best for you (faster load time)
html: CC=em++
html: CFLAGS=-c -std=c++1y -DGLES2_HELPER_USE_GLUT -DGLES2_HELPER_EMSCRIPTEN_QUIRKS -O2
html: LDFLAGS=--shell-file shell_minimal.html --preload-file models -lglut -O2 -s TOTAL_MEMORY=167772160
html: EXTENSION=.html
html: build_exec

# em++ (ver 1.3+ tested)
# Rem: Change TOTAL_MEMORY for the value it is best for you (faster load time)
html_old: CC=em++
html_old: CFLAGS=-c -stdlib=libc++ -std=c++1y -DUSE_FULL_GL=1 -DGLES2_HELPER_USE_GLUT -DGLES2_HELPER_EMSCRIPTEN_QUIRKS -O2
html_old: LDFLAGS=--shell-file shell_minimal.html --preload-file models -lglut -stdlib=libc++ -O2 -s TOTAL_MEMORY=167772160
html_old: EXTENSION=.html
html_old: build_exec

# ########### #
# BUILD PHASE #
# ########### #

build_exec: $(SOURCES) $(EXECUTABLE)

# The LDFLAGS should be after the files
# because of some lazyness in newer toolchains!
$(EXECUTABLE): $(OBJECTS) 
# In case of emscripten build, we make a html5/webgl output
ifeq ($(CC),em++)
	$(CC) $(OBJECTS) -o $@$(EXTENSION) $(LDFLAGS)
else
	$(CC) $(OBJECTS) -o $@$(EXTENSION) $(LDFLAGS)
endif

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o objmaster/*.o showobj objmaster/ext/*.o objmaster/ext/integration/*.o
