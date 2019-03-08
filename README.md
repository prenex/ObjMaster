# ObjMaster
Modern LGPLv3(ext) C++14 library for handling .obj/mtl 3D model files - originally written by me to aid model loading in the multi-platform AR toolkit projects.

Remark: Both **import and export is supported** now!

Works quite well for obj files exported from Blender with triangulize-faces and generate-normals turned on in the exporter. Also contains a simple .obj thresholded-diff tool which is useful for creating animations (obj2obd.jar). Code is tested on android, emscripten/js/webgl, linux/opengl, hololens/directx11 and **this even works on a raspberry or orange pi**. The library should be easy to set up, use and found out its inner workings when something goes wrong - it is not fool-safe at all though.

Please keep attention to the licence as that is why I can push backport updates to this codebase when I change something that I need for my work and not only for my home purposes!

Try out the emscripten webgl example
------------------------------------

A (not latest) build of the emscripten example build can be found and tested here:

http://ballmerpeak.web.elte.hu/graphics/showobj.html

Basic architecture
------------------

1.) MaterializedObjModel + ObjMeshObject, MaterializedObjMeshObject

2.) Obj

2.5) ObjCreator

3.) MtlLib, VertexElement, FaceElement, UseMtl, LineElement, ...

4.) ObjMasterIntegrationFacade

- The classes on **the first layer** are the end-user classes and are prepared to aid rendering of the obj files with a 3D rendering API of choice. They are built upon the second level of classes and use them as a source, but they adhere to a logical structure that is optimized for rendering. The obj file format supports varoius things that are not supported by APIs and 3D cards directly and these are being translated to adhere this representation while construction. One example of this is that the obj file format handle faces with points having different index values for vertex positions, normals, texture coordinates. This is hardly optimally handled in current 3D hardware so an algorithm translates the representation to use one index value for all data and we regenerate the data like this etc. Any further changes that are in the representation for being render-friendly should go to this logical layer.
- **The second layer** is basically tries to be 1-1 corresponding to how the data are in the .obj and .mtl files. This is basically a parser where every line corresponds to various descriptor classes. Reading in a file results in a complete parse of the logical structure and loading of the lightweights of the material library. In case we are writing a converter application or want to do batched changes to .obj or .mtl files and such things, we can decide to stay on this level. The design tries to make it easy to use data in this representation when translating to the 3D API-friendly one by some c++ tricks like making it available to copy whole vectors and such, but not more than that and it is just a representation of the file structure logic as is. Changes in the parser logic and fixes to it should go to this logical level.
- **EXPORTING OBJs:** Now the ObjMaster library also supports generating and saving obj files as output files. A saveAs() method is added to the Obj and Mtl classes while most "Element"-level objects can be asked for the .obj-side representation with the asText() method. Generating new .obj files or appending data to an already existing one is best supported with the new class: **ObjCreator**. This class is trying to give you a bullet-proof way to generate the Obj and Mtl in a way that can be exported - everything is is not ensured to work, so please use this facade for exporting! Remark: convenience methods are added to generate .obj output from data that is in the ObjMeshObject format (that is having homogenous indices for vertices, normals and texcoords), but direct exporting from the third layer is not possible yet.
- **The third layer** is basically having a lot of "Element" objects. These are describing the possible syntax of the varios line descriptors of the obj file. For example VertexElement corresponds to the "v <parameters>" descriptor, while VertexNormalElement corresponds to "vn <params>" of the file. These elements are there to make the line-based parsing possible and all have an "isParsable(...)" method along with their constructors that create the object out of the line. In case one wants to contribute for the loader to handle more variations of these little details (like prefix white spaces, alternative keywords) this level should be approached.
- **The forth layer** is an external **interfacing layer**: It can be used to interface from C or managed languages through JNI or p/invoke. This facade provides a limited functionality - considered to be the most widely useful operations. Tested mostly as a C# DLL in UWP apps and with the automatic testing operations.

So Element... is the lowest level, Obj is how it is stored in the .obj files and ...Model, ...Object is for rendering purposes...

Try to adhere to the architecture when forking the code or providing fixes!
 
Design considerations
---------------------
- Use c++14 features extensively. Prefer unique pointers and non-shared ownership.
- Use the feature that vectors are by design are expected to be on continous memory areas (fast array access).
- In case of threads, APIs and libraries use the standard library as much as possible.
- Prefer one file per class or logical unit
- Prefer using embeddable libraries like one-header libraries instead of depencencies (like stb_image.h)
- Prefer copy semantics with small objects, try not using pointers and references at all if not necessary.
- Try to avoid allocating a lot of small objects on the heap - use the stack in this case even when it needs 10-20byte copies
- Try to be as multiplatform as it can be: we used this first for android, but all I/O code is generic through asset library
- Try to stay usable from different 3D APIs: Texture loading handling is provided by user, no tight coupling!
- Try not to become a "one and true magic library" - that is I am not planning to make this fool-proof to handle everything!
- Try to keep things as lightweight as possible and to the point. Designed for controlled situations, generate vertex normals, do clean up the files prior to use with vim or whatever you want, but it is better to keep the code small than to handle these!

Try to adhere to these considerations when forking the code or providing fixes!

Why it is not LGPLv3, how it is modified?
-----------------------------------------
(ext): The licence is modified to make general unrestricted use available for Grepton Zrt. so that they do not need to adhere to the LGPLv3 parts of the licence for being fair. Other than this, basically the licence is LGPLv3 with an added emphasis on a mandatory and visible reference to the repository and Grepton Zrt. in any non-free application. These are not necessary in case of open sourced and free projects and applications or closed sourced but still free projects and applications. Refer to the licence for a better understanding.

Examples and unit tests
-----------------------

An application called **showobj** is provided as a simple example. The whole library is provided as-is and you are not supposed to build it in any complicated ways (just add the sources to your project) however this example contains a makefile for emscripten, g++ and cland build possibilities with different parameters. These can be changed by calling make with different parameters (there is a default though):

* make default: uses gnu_glut
* make gnu_glut: Uses g++ for compilation and GLUT for window and I/O.
* make gnu_egl: Uses g++ for compilation and EGL with X11 only!
* make clang_glut: Uses clang 4.9+ and GLUT
* make clang_egl: Uses clang 4.9+ and EGL
* make clang_old_glut: Uses older clang 3.9+ and GLUT
* make clang_old_egl: Uses older clang 3.9+ and EGL
* make html: Uses a later amscripten
* make html_old: Uses an older emsripten
* make pi: Works on armbian and raspbian (Pi). Uses g++, EGL, X11 only for GLES2 and sets to use 16 bit indices only!

The example is based on the header-only gles2helper library from me. It is used as a git submodule:

	git submodule update --init --recursive

or for first cloning

	git clone --recursive [url]

This application also contains some ad-hoc **"unit tests":** you can run these with **showobj --test**. These are only semi-automatic testing and run various codes that are in the tests/test.h file. This is a headless run, while the showobj application as a whole is an opengl application that uses the common subset of opengl and opengl ES2 and webgl so it runs on every tested GL-related platforms.

From time-to-time the library is also tested as a C#/Unity3D external plugin as it is part of a much more complex infrastructure with a lot of code that is not visible here. Backports are usually coming when the win32 and win64 and uwp(32/64) builds as unity plugin DLLs seem to have issues. The design of the **"extern C"**-binding is also heavily influenced so that interfacing with managed languages are as easy as possible with the least amount of nasty marshalling issues.
