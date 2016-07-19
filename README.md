# ObjMaster
Modern LGPLv3* C++14 library for handling *.obj/mtl 3D model files - originally written by me to aid model loading in the multi-platform AR toolkit projects.

Basic architecture:
-------------------

1.) MaterializedObjModel + ObjMeshObject, MaterializedObjMeshObject

2.) Obj

3.) MtlLib, VertexElement, FaceElement, UseMtl,

- The classes on the first level are the end-user classes and are prepared to aid rendering of the obj files with a 3D rendering API of choice. They are built upon the second level of classes and use them as a source, but they adhere to a logical structure that is optimized for rendering. The obj file format supports varoius things that are not supported by APIs and 3D cards directly and these are being translated to adhere this representation while construction. One example of this is that the obj file format handle faces with points having different index values for vertex positions, normals, texture coordinates. This is hardly optimally handled in current 3D hardware so an algorithm translates the representation to use one index value for all data and we regenerate the data like this etc. Any further changes that are in the representation for being render-friendly should go to this logical layer.
- The second layer is basically tries to be 1-1 corresponding to how the data are in the *.obj and *.mtl files. This is basically a parser where every line corresponds to various descriptor classes. Reading in a file results in a complete parse of the logical structure and loading of the lightweights of the material library. In case we are writing a converter application or want to do batched changes to *.obj or *.mtl files and such things, we can decide to stay on this level. The design tries to make it easy to use data in this representation when translating to the 3D API-friendly one by some c++ tricks like making it available to copy whole vectors and such, but not more than that and it is just a representation of the file structure logic as is. Changes in the parser logic and fixes to it should go to this logical level.
- The third layer is basically having a lot of "Element" objects. These are describing the possible syntax of the varios line descriptors of the obj file. For example VertexElement corresponds to the "v <parameters>" descriptor, while VertexNormalElement corresponds to "vn <params>" of the file. These elements are there to make the line-based parsing possible and all have an "isParsable(...)" method along with their constructors that create the object out of the line. In case one wants to contribute for the loader to handle more variations of these little details (like prefix white spaces, alternative keywords) this level should be approached.

Try to adhere to the architecture when forking the code or providing fixes!
 
Design considerations:
----------------------
- Use c++14 features extensively. Prefer unique pointers and non-shared ownership.
- Use the feature that vectors are by design are expected to be on continous memory areas (fast array access).
- In case of threads, APIs and libraries use the standard library as much as possible.
- Prefer one file per class or logical unit
- Prefer using embeddable libraries like one-header libraries instead of depencencies (like stb_image.h)
- Prefer copy semantics with small objects, try not using pointers and references at all if not necessary.
- Try to avoid allocating a lot of small objects on the heap - use the stack in this case even when it needs 10-20byte copies
- Try to be as multiplatform as it can be: we used this first for android, but all I/O code is generic through asset library
- Try to stay usable from different 3D APIs: Texture loading handling is provided by user, no tight coupling!- 

Try to adhere to these considerations when forking the code or providing fixes!

Why it is not LGPLv3, how it is modified?
-----------------------------------------
*: The licence is modified to make general unrestricted use available for Grepton Zrt. so that they do not need to adhere to the LGPLv3 parts of the licence for being fair. Other than this, basically the licence is LGPLv3 with an added emphasis on a mandatory and visible reference to the repository and Grepton Zrt. in any non-free application. These are not necessary in case of open sourced and free projects and applications or closed sourced but still free projects and applications. Refer to the licence for a better understanding.
