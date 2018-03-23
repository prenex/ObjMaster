#define TEST // When enabled, the script tests for reaching to the DLL by using a simple array-sort test

using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Text;
using System.Collections.Generic;

/// <summary>
/// Unity facade for calling methods in the ObjMaster integration DLL library. Provided as mono behaviour so that we can test functionality in the editor.
/// Remarks for usage:
/// - You need to get a handle for a model first. There is a cached system from which you can get this.
/// - You provice this handle for various operations to get data out from models. They are provided in various formats.
/// - A model that the handle represents contains "meshes". For different materials or obj groups, you get multiple different meshes.
/// - You can query the material of a given mesh. This contains simple information like color, specular, etc. and an extra field telling you which texture filename queries are possible.
/// - You can query the given texture filenames and do whatever you want. The system is able to load the bitmaps, so that functionality might be added later. Typically it is enough to know the filename.
/// - You can access the vertex and index buffer data directly through IntPtr or through marshalled copies of these buffers. The latter way lets you unload the model from the system immediately if you want!
/// - After operating as you wish, you can unload the model using its handle. If others used the same because of caching and still want to access it, that will not work! This might upset you a bit, but caching is still feasible.
/// - At least you do not need to care about something being unloaded multiple times - we do not punish you for that. (without this, caching would be "pointless")
/// - There is an operation to unload anything that the underlying c++ code had aquired. This free up more memory than unloading everything you once have loaded! There should be no leaks (just bugs haha), but there is some constant administrative cost otherwise.
/// </summary>
public class ObjMasterUnityFacade : MonoBehaviour
{

    /// <summary>
    /// The separator character for "annotated" obj files
    /// </summary>
    public const char OBJ_ANNOTATION_SEP_CHAR = ':';

    ///  <summary>
    ///  The separator that serves as the name of the last annotation of the group name when getting an objMatFaceGroup. After this, the material name follows!
    ///  </summary>
    public const string OBJ_ANNOTATION_MTL_NAME = "mtl";
    #region Other subtypes
    /// <summary>
    /// Defines which directions the vertex-pos and vertex-normal data should be mirrored
    /// </summary>
    public enum MIRROR_MODE
    {
        NONE = 0,
        MIRROR_X = 1,
        MIRROR_Y = 2,
        MIRROR_Z = 4,
        MIRROR_XY = 3,
        MIRROR_XZ = 5,
        MIRROR_YZ = 6,
        MIRROR_XYZ = 7
    }
    #endregion
    #region ObjMaster structures
    /// <summary>
    /// Simplified material that also contains texturing information for making texture queries
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SimpleMaterial
    {
        /// <summary>
        /// Use the c++ side Material.F_* special values of ObjMaster when indexing! Also shows if we can ask for texture names or not!
        /// </summary>
		public uint enabledFields;
        /// <summary>
        /// Only relevant if enabledFields shows it is!
        /// </summary>
		public float kar, kag, kab, kaa;
        /// <summary>
        /// Only relevant if enabledFields shows it is!
        /// </summary>
		public float kdr, kdg, kdb, kda;
        /// <summary>
        /// Only relevant if enabledFields shows it is!
        /// </summary>
		public float ksr, ksg, ksb, ksa;

        /// <summary>
        /// Simple textual representation
        /// </summary>
        public override string ToString()
        {
            return "SimpleMaterial[fields:" + enabledFields
                + "; ambi:" + kar + ", " + kag + ", " + kab + ", " + kaa + ", "
                + "; diff:" + kdr + ", " + kdg + ", " + kdb + ", " + kda + ", "
                + "; spec:" + ksr + ", " + ksg + ", " + ksb + ", " + ksa + "]";
        }

        /// <summary>
        /// Determines if the material contains a description for the given field or not
        /// </summary>
        /// <returns>True in case we have it, false otherwise</returns>
        public bool hasAmbientColor()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_KA) != 0);
        }

        /// <summary>
        /// Determines if the material contains a description for the given field or not
        /// </summary>
        /// <returns>True in case we have it, false otherwise</returns>
        public bool hasDiffuseColor()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_KD) != 0);
        }

        /// <summary>
        /// Determines if the material contains a description for the given field or not
        /// </summary>
        /// <returns>True in case we have it, false otherwise</returns>
        public bool hasSpecularColor()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_KS) != 0);
        }

        /// <summary>
        /// Determines if the material has the named texture or not. If it has, one can access it by the specific facade methods!
        /// </summary>
        public bool hasAmbientTexture()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_MAP_KA) != 0);
        }

        /// <summary>
        /// Determines if the material has the named texture or not. If it has, one can access it by the specific facade methods!
        /// </summary>
        public bool hasDiffuseTexture()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_MAP_KD) != 0);
        }

        /// <summary>
        /// Determines if the material has the named texture or not. If it has, one can access it by the specific facade methods!
        /// </summary>
        public bool hasSpecularTexture()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_MAP_KS) != 0);
        }

        /// <summary>
        /// Determines if the material has the named texture or not. If it has, one can access it by the specific facade methods!
        /// </summary>
        public bool hasNormalTexture()
        {
            return ((enabledFields & ENABLED_FIELD_BITS.F_MAP_BUMP) != 0);
        }

        /// <summary>
        /// Can be used to determine which obj-mtl fields are enabled by AND-ing it with these values. Should be correspond as the order of bits are defined in c++ code for "Material.h" but here we we present them as power of two values for easier usage...
        /// </summary>
        public static class ENABLED_FIELD_BITS
        {
            public const uint F_KA = 1;         // Bit-index: 0
            public const uint F_KD = 2;         // Bit-index: 1
            public const uint F_KS = 4;         // Bit-index: 2
            public const uint F_MAP_KA = 8;     // Bit-index: 3
            public const uint F_MAP_KD = 16;    // Bit-index: 4
            public const uint F_MAP_KS = 32;    // Bit-index: 5
            public const uint F_MAP_BUMP = 64;  // Bit-index: 6
        }
    }

    /// <summary>
    /// Per-vertex data aquired from the objmaster mesh system. Uses striped representation where different values are compacted together in one buffer.
    /// This should be the same as it is in VertexStructure.h in the c/c++ code because we are blitting it against each other!
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VertexStructure
    {
        // position
        public float x, y, z;
        // normal
        public float i, j, k;
        // texture0 uv
        public float u, v;

        public override string ToString()
        {
            return "VertexPosNorUv(" + x + "," + y + "," + z + "; " + i + "," + j + "," + k + "; " + u + "," + v + ")";
        }
    }
    #endregion
    #region Imported DLL functions

    const string DLL_NAME = "ObjMasterHololensUnity";

    /// <summary>
    /// Try loading the given model with the objmaster system. If the model is already loaded, the earlier handle is returned from cache!
    /// </summary>
    /// <param name="path">Path for the model - also search path of mtl file and relative names</param>
    /// <param name="fileName">Filename of the obj</param>
    /// <returns>A handle to reference this model or -1 in case of errors!</returns>
    [DllImport(DLL_NAME, EntryPoint = "loadObjModel", CallingConvention = CallingConvention.Cdecl)]
    public static extern int loadObjModel(string path, string fileName);

    /// <summary>
    /// Try to unload the model, referenced by the handle. Beware when loading the same model multiple times and releasing without notifying the other subsystem!
    /// </summary>
    /// <param name="handle">The reference handle for the model to unload</param>
    /// <returns>True on success, false otherwise</returns>
    [DllImport(DLL_NAME, EntryPoint = "unloadObjModel", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool unloadObjModel(int handle);

    /// <summary>
    /// Try to unload all models - and free all memory. This operation is useful
    /// if you do not only need to release your model resources, but also release
    /// the small amount of administrative caches. After calling this, there should
    /// be no memory leaks and left-overs unless there is a bug in the underlying library.
    /// </summary>
    /// <returns>false indicates that something went wrong and there might be a leak or inconsistency!</returns>
    [DllImport(DLL_NAME, EntryPoint = "unloadEverything", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool unloadEverything();

    /// <summary>
    /// Returns the number of meshes a model is having.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <returns>-1 in case of errors or bad handle</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshNo", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshNo(int handle);

    /// <summary>
    /// Returns the SimpleMaterial of the designated model mesh. Also useful as it defines which texture names we can look for!
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The material.</returns>
    // Maybe needed ", CallingConvention = CallingConvention.Cdecl)]" ???
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshMaterial", CallingConvention = CallingConvention.Cdecl)]
    public static extern SimpleMaterial getModelMeshMaterial(int handle, int meshIndex);

    /// <summary>
    /// Tells the number of vertex data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all!
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>Number of vertex data for the given mesh</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshVertexDataCount", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshVertexDataCount(int handle, int meshIndex);

    /// <summary>
	/// Returns the base-offset of vertices in this mesh when using shared vertex buffers.
	/// When the vertex buffers are given per-mesh, this always return 0 for safe usage.
	/// This method is really useful for creating per-mesh buffers from the objmaster provided
	/// data as it is in the case of the unity integration and such. Indices of one mesh will
	/// have an offset of this value in the shared case so creating a copy of the real indices
	/// works by getting this value and substracting it from each shared index value!
	///
	/// Rem.: This method indicates errors by returning negative values (-1)
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh of the model</param>
    /// <returns>Negative on errors - otherwise the offset value described above</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshBaseVertexOffset", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshBaseVertexOffset(int handle, int meshIndex);

    /// <summary>
	/// Extracts the vertex data for the mesh of the given handle into output pointer
	/// 
	/// When marshalling with Marshar.PtrToStructure as VertexData, the target should be big-enough to hold
    /// the vertex data, so the user code should first ask for the number of the elements in this structure!
	/// 
	/// Returns false in case of errors or incomplete operation
    /// 
	/// See: getModelMeshVertexDataCount
    /// 
    /// Example usage is something like this:
    ///     IntPtr ptrNativeData;
    ///     int nativeDataLength = getModelMeshVertexData(0, 0, out ptrNativeData);
    ///     VertexStructure[] vertexArray = new VertexStructure[nativeDataLength];
    ///     for(int i = 0; i &lt; nativeDataLength; ++i) {
    ///         Marshal.PtrToStructure(ptrNativeData, vertexArray[i]);
    ///         p += Marshal.SizeOf(typeof(VertexStructure));
    ///     }
    /// 
    /// Also, the returned data could be used directly to pass to an other native operation or force to understand it as float values...
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <param name="pointer">Will hold pointer to the data as an IntPtr</param>
    /// <returns></returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshVertexData", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshVertexData(int handle, int meshIndex, out IntPtr pointer);

    /// <summary>
    /// Tells the number of index data for the given mesh of the handle.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns> Returns -1 in case of errors and zero when there is no data at all!</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshIndicesCount", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshIndicesCount(int handle, int meshIndex);

    /// <summary>
    /// Fills a pointer to point to the array of indices using the output parameter.
    /// The layout of the resulting data is one uint32_t for each element, so you can use the UIntPtr directly to access this memory easily!
    /// Also you can try to copy this memory area with marshalling or something else if you want your own copy here too.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <param name="output">The pointer which will contain the location of the result</param>
    /// <returns>The number of indices of -1 in case of errors</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshIndices", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshIndices(int handle, int meshIndex, out IntPtr output);

    /// <summary>
    /// Returns a pointer to the CSTR of the Ambient texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshAmbientTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshAmbientTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Diffuse texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshDiffuseTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshDiffuseTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Specular texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshSpecularTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshSpecularTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Normal texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshNormalTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshNormalTextureFileNamePtr(int handle, int meshIndex);

    /// <summary>
    /// Returns the name of the material for the given mesh. The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors. An empty string might be returned too!</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshMaterialName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshMaterialNamePtr(int handle, int meshIndex);

    /// <summary>
    /// Returns the name of the objMatFaceGroup - see objMaster - basically the 'g' and 'o' group name with added :mtl:<materialName>.
    /// The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors. An empty string should not be returned, but better expect that too as other side does not check!</returns>
    [DllImport(DLL_NAME, EntryPoint = "getModelMeshObjMatFaceGroupName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshObjMatFaceGroupNamePtr(int handle, int meshIndex);

    #endregion
    #region Imported DLL functions for *.obj output and creation
    // Obj creation functions
    // ======================

    // 1.) Creation / closure
    // ----------------------

    /// <summary>
    /// Creates an ObjCreator factory for runtime Obj generation and an empty *.obj model in it and return the handle for the factory.
    /// Rem.: Useful when creating / saving models from scratch.
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "createObjFactory", CallingConvention = CallingConvention.Cdecl)]
    public static extern int createObjFactory();

    /// <summary>
    /// Creates an ObjCreator factory for runtime Obj generation by loading the given *.obj model in it as a base and return the handle for the factory to extend this geometry.
    /// Rem.: The file designated by path+fileName is not affected or changed, unless there is a save operation as that designation as its target!
    /// Rem.: Useful when "appending" new data to an already existing obj file (output can be saved as a different obj however)
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "createObjFactoryWithBaseObj", CallingConvention = CallingConvention.Cdecl)]
    public static extern int createObjFactoryWithBaseObj(string path, string fileName);

    /// <summary>
    /// (!) CLOSE/reset THE FACTORY and save a *.obj file out created by the given factory handle to the given path.
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "saveObjFromFactoryToFileAndPossiblyCloseFactory", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool saveObjFromFactoryToFileAndPossiblyCloseFactory(int factoryHandle, string path, string fileName, bool closeFactory);

    /// <summary>
    /// Save a *.obj file out created by the given factory handle to the given path.
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "saveObjFromFactoryToFile", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool saveObjFromFactoryToFile(int factoryHandle, string path, string fileName);

    /// <summary>
    /// Resets the given factory. This saves a bit memory - but keeps the factory handle **reusable**.
    /// - Return value of false indicates that there was some error in this operation!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "resetFactory", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool resetFactory(int factoryHandle);

    /// <summary>
    /// The system tries its best to release all resources aquired by the factory: it is not defined if leaks are possible of not - so if you do not want leaks then use closeAllFactories()!
    /// This saves a bit of memory and makes the factory handle unusable! In good implementations it should be safe to rely on this method saving most resources that is possible.
    /// - Return value of false indicates that there was some error in this operation!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "hintCloseFactory", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool hintCloseFactory(int factoryHandle);

    /// <summary>
    /// A WAY TO SURELY RELEASE ALL RESOURCES for factories: Closes all ObjCreator factories created by the ObjMasterIntegrationFacade!
    /// - Return value of false indicates that there was some error in this operation!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "closeAllFactories", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool closeAllFactories();

    // 2.) Runtime generation of *.obj geometry and materials
    // ------------------------------------------------------

    /// <summary>
    /// Adds the given runtime-generated material to the given factory - beware of filling "enabledFields" properly!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "addRuntimeGeneratedMaterial", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool addRuntimeGeneratedMaterial(int factoryHandle, SimpleMaterial m, string materialName,
        string map_ka = null, string map_kd = null, string map_ks = null, string map_bump = null);

    /// <summary>
    /// Unsafe and fast: adds a vertex structure to the Obj
    /// - We expect that the Obj is build only using this method and nothing else.
    /// - The index of the added 'v' is returned - or in case of errors: (-1)
    /// - This index can be used as a global index for everything in the vData (like uvs and normals too)
    /// - However the latter only works if we do not use the objs support of "different indices per element" logic!
    ///   This means that we can easily bulk-generate output for data already in a renderer-friendly format, but
    ///   we better not mix generating data from that with "extending" already opened obj files for example!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "addHomogenousVertexStructure", CallingConvention = CallingConvention.Cdecl)]
    public static extern int addHomogenousVertexStructure(int factoryHandle, VertexStructure vData);

    /// <summary>
    /// Add a face with homogenous indices (v, vt, vn indices are the same).
    /// Three index is necessary for forming a triangle!
    /// Returns the face-Index
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "addFace", CallingConvention = CallingConvention.Cdecl)]
    public static extern int addFace(int factoryHandle, uint aIndex, uint bIndex, uint cIndex);

    /// <summary>
    /// Use the given group-name from now on - every face will be in the group.
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "useGroup", CallingConvention = CallingConvention.Cdecl)]
    public static extern int useGroup(int factoryHandle, string name);

    /// <summary>
    /// Use the given material-name from now on - every face will have the given material. If materialName not exists, an empty material gets generated!
    /// </summary>
    [DllImport(DLL_NAME, EntryPoint = "useMaterial", CallingConvention = CallingConvention.Cdecl)]
    public static extern int useMaterial(int factoryHandle, string materialName);
    #endregion
    #region TEST
#if TEST
#if UNITY_EDITOR
    // The imported test function
    [DllImport(DLL_NAME, EntryPoint = "testSort", CallingConvention = CallingConvention.Cdecl)]
    public static extern void testSort(int[] a, int length);

    // Test stuff
    [Tooltip("For testing DLL connectivity on startup!")]
    public int[] testArray;

    [Tooltip("When set as true, we test loading on Start() call using the specified test path and filename.")]
    public bool testLoadingOnStart = true;

    [Tooltip("The path of the file for testing obj model loading on startup!")]
    public string testPath = "C:\\";

    [Tooltip("The name of the file for testing obj model loading on startup!")]
    public string testFileName = "test.obj";

    [Tooltip("Used for 'triggerTestExport'. This is the output file path!")]
    public string testExportFilePath = "";

    [Tooltip("Used for 'triggerTestExport'. This is the output filename!")]
    public string testExportFileName = "test_out.obj";

    [Tooltip("Used for 'triggerTestExport'. This is the name of the texture.")]
    public string testExportTextureName = "";

    [Tooltip("Unit test trigger for exporting a simple example *.obj as testOutFileName")]
    public bool triggerTestExport = false;

    private void doTestExport()
    {
        // TODO: implement unit test
    }

    private void Update()
    {
        if (triggerTestExport)
        {
            doTestExport();
            triggerTestExport = false;
        }
    }

    private void Start()
    {
        Debug.Log("DLL test array before sort: " + testArray.ToString());
        testSort(testArray, testArray.Length);
        Debug.Log("DLL test array after sort: " + testArray.ToString());

        // Test loading of models
        if (testLoadingOnStart)
        {
            int modelHandle = loadObjModel(testPath, testFileName);

            // If this is a valid handle, we will test it and close!
            if (modelHandle >= 0)
            {
                Debug.Log("Loaded model with handle: " + modelHandle);

                try
                {
                    int meshNo = getModelMeshNo(modelHandle);
                    Debug.Log("Loaded model has " + meshNo + "meshes!");

                    if (meshNo > 0)
                    {
                        // Counts
                        int indicesCount = getModelMeshIndicesCount(modelHandle, 0);
                        Debug.Log("Number of indices in the first mesh: " + indicesCount);

                        int verticesCount = getModelMeshVertexDataCount(modelHandle, 0);
                        Debug.Log("Number of vertices in the first mesh: " + verticesCount);

                        // Simple material values
                        SimpleMaterial sm = getModelMeshMaterial(modelHandle, 0);
                        Debug.Log("The first mesh is having this material: " + sm);
                        string firstMatName = getModelMeshMaterialName(modelHandle, 0);
                        Debug.Log("The first mesh is having this material name: " + firstMatName);

                        // Texture filenames
                        string ambitex = getModelMeshAmbientTextureFileName(modelHandle, 0);
                        Debug.Log("Ambient texture is: " + ambitex);
                        string difftex = getModelMeshDiffuseTextureFileName(modelHandle, 0);
                        Debug.Log("Diffuse texture is: " + difftex);
                        string spectex = getModelMeshSpecularTextureFileName(modelHandle, 0);
                        Debug.Log("Specular texture is: " + spectex);
                        string bumptex = getModelMeshNormalTextureFileName(modelHandle, 0);
                        Debug.Log("Normal/bump texture is: " + bumptex);

                        // Vertex and index buffer data
                        uint[] indices = getModelMeshIndicesCopy(modelHandle, 0, MIRROR_MODE.MIRROR_X, true);
                        Debug.Log("Indices of the first mesh: " + prettyPrintIndices(indices));

                        VertexStructure[] vertices = getModelMeshVertexDataCopy(modelHandle, 0);
                        Debug.Log("Vertices of the first mesh: " + prettyPrintVertices(vertices));
                    }

                }
                finally
                {
                    // Try to unload this model immediately!
                    bool success = unloadObjModel(modelHandle);
                    Debug.Log("Model unload success: " + success);
                }
            }
            else
            {
                Debug.Log("Got bad model handle on return (there was an error): " + modelHandle);
            }
        }
    }

    // Pretty-print helper
    private string prettyPrintIndices(uint[] indices)
    {
        if (indices == null)
        {
            return "null";
        }
        else
        {
            StringBuilder sb = new StringBuilder("indices[");
            for (int i = 0; i < indices.Length; ++i)
            {
                sb.Append(indices[i]);
                if (i < indices.Length - 1)
                {
                    sb.Append("; ");
                }
            }
            sb.Append("]");
            return sb.ToString();
        }
    }

    // Pretty-print helper
    private string prettyPrintVertices(VertexStructure[] vertices)
    {
        if (vertices == null)
        {
            return "null";
        }
        else
        {
            StringBuilder sb = new StringBuilder("vertices[");
            for (int i = 0; i < vertices.Length; ++i)
            {
                sb.Append(vertices[i].ToString());
                if (i < vertices.Length - 1)
                {
                    sb.Append("; ");
                }
            }
            sb.Append("]");
            return sb.ToString();
        }
    }
#endif
#endif
    #endregion
    #region Helper methods
    /// <summary>
    /// Returns the count of all vertices in the whole model with its all meshes. Basically this method just iterates over all mesh indexes and summarizes.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <returns>The total number of vertices in the model. Tries to return negative values on errors.</returns>
    public static int getModelVertexDataCount(int handle)
    {
        if (handle >= 0)
        {
            int totalVertexNo = 0;
            for (int meshIndex = 0; meshIndex < getModelMeshNo(handle); ++meshIndex)
            {
                totalVertexNo += getModelMeshVertexDataCount(handle, meshIndex);
            }
            return totalVertexNo;
        }
        else
        {
            // Bad handle
            return -1;
        }
    }

    /// <summary>
    /// Ambient texture filename. Returns null in case of errors, and empty if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The string or nullptr in case of errors</returns>
    public static string getModelMeshAmbientTextureFileName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshAmbientTextureFileNamePtr(handle, meshIndex);
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    /// <summary>
    /// Diffuse texture filename. Returns null in case of errors, and empty if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The string or nullptr in case of errors</returns>
    public static string getModelMeshDiffuseTextureFileName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshDiffuseTextureFileNamePtr(handle, meshIndex);
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    /// <summary>
    /// Specular texture filename. Returns null in case of errors, and empty if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The string or nullptr in case of errors</returns>
    public static string getModelMeshSpecularTextureFileName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshSpecularTextureFileNamePtr(handle, meshIndex);
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    /// <summary>
    /// Returns the name of the group of the mesh - see objMaster - basically the 'g' and 'o' group name directly. Different meshes coming in any orders can share group names!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The group name string or nullptr in case of errors. An empty string should not be returned, but better expect that too as other side does not check against it!</returns>
    public static string getModelMeshGroupName(int handle, int meshIndex)
    {
        // This code here...
        // ...create: groupname:t:f
        // ...out of: groupname:t:f:mtl:materialname
        string objMatFaceGroupName = getModelMeshObjMatFaceGroupName(handle, meshIndex);
        if ((objMatFaceGroupName != null) && (objMatFaceGroupName != ""))
        {
            int lastSepI = objMatFaceGroupName.LastIndexOf(OBJ_ANNOTATION_SEP_CHAR + OBJ_ANNOTATION_MTL_NAME + OBJ_ANNOTATION_SEP_CHAR);
            string groupName = objMatFaceGroupName.Substring(0, lastSepI);
            return groupName;
        }
        else
        {
            if (objMatFaceGroupName != null)
            {
                return "";
            }
            else
            {
                return null;
            }
        }
    }

    /// <summary>
    /// Returns the name of the objMatFaceGroup - see objMaster - basically the 'g' and 'o' group name with added :mtl:<materialName> in the suffix of the string.
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The string or nullptr in case of errors. An empty string should not be returned, but better expect that too as other side does not check against it!</returns>
    public static string getModelMeshObjMatFaceGroupName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshObjMatFaceGroupNamePtr(handle, meshIndex);
        return stringFromNativeUtf8(ptr);
        /*
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            // This does a copy and the widening too
            return Marshal.PtrToStringAnsi(ptr);
        }
        */
    }

    /// <summary>
    /// Conversion of a native ptr pointing at an UTF8 cstring to C# "string". Also handles nullptr.
    /// </summary>
    /// <param name="nativeUtf8">Pointer to an UTF8 null-terminated string. Surely it can be also an ANSi string...</param>
    /// <returns>On nullptr we return null, otherwise a C# string object which contains copy of the data</returns>
    public static string stringFromNativeUtf8(IntPtr nativeUtf8)
    {
        try
        {
            // Get string
            if (IntPtr.Zero.Equals(nativeUtf8))
            {
                // nullptr to null conversion
                return null;
            }
            else
            {
                // This does a copy and the widening too
                // strlen
                int len = 0;
                while (Marshal.ReadByte(nativeUtf8, len) != 0) ++len;
                // byte-wise copy
                byte[] buffer = new byte[len];
                Marshal.Copy(nativeUtf8, buffer, 0, buffer.Length);
                // Creating an UTF8
                return Encoding.UTF8.GetString(buffer);
            }
        }
        catch (Exception)
        {
            // Some extra defensivity
            return null;
        }
    }

    /// <summary>
    /// Returns the name of the material for the given mesh. The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The string or nullptr in case of errors. An empty string might be returned too!</returns>
    public static string getModelMeshMaterialName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshMaterialNamePtr(handle, meshIndex);
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            // This does a copy and the widening too
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    /// <summary>
    /// Normal texture filename. Returns null in case of errors, and empty if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The string or nullptr in case of errors</returns>
    public static string getModelMeshNormalTextureFileName(int handle, int meshIndex)
    {
        // Get pointer
        IntPtr ptr = getModelMeshNormalTextureFileNamePtr(handle, meshIndex);
        // Get string
        if (IntPtr.Zero.Equals(ptr))
        {
            // nullptr to null conversion
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    /// <summary>
    /// Can be used to extract vertex positions from the returned vertex buffer. Better to use createModelMeshData directly to avoid multiple unnecessary calls if you can do that!
    /// </summary>
    /// <param name="vertexBuffer">The vertex data buffer which represents the values encoded in the obj file</param>
    /// <param name="mirrorMode">Mirroring mode</param>
    /// <returns>Copy of the positions as extracted from the buffer</returns>
    public static Vector3[] extractVertexPosDataFrom(VertexStructure[] vertexBuffer, MIRROR_MODE mirrorMode)
    {
        if (vertexBuffer == null)
        {
            return null;
        }
        else
        {
            Vector3[] vPosData = new Vector3[vertexBuffer.Length];
            for (int i = 0; i < vertexBuffer.Length; ++i)
            {
                VertexStructure vs = vertexBuffer[i];
                vPosData[i].x = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_X) != 0 ? vs.x : -vs.x;
                vPosData[i].y = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_Y) != 0 ? vs.y : -vs.y;
                vPosData[i].z = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_Z) != 0 ? vs.z : -vs.z;
            }
            return vPosData;
        }
    }

    /// <summary>
    /// Can be used to extract vertex normals from the returned vertex buffer. Better to use createModelMeshData directly to avoid multiple unnecessary calls if you can do that!
    /// </summary>
    /// <param name="vertexBuffer">The vertex data buffer which represents the values encoded in the obj file</param>
    /// <param name="mirrorMode">Mirroring mode</param>
    /// <returns>Copy of the normal-vectors as extracted from the buffer</returns>
    public static Vector3[] extractVertexNormalDataFrom(VertexStructure[] vertexBuffer, MIRROR_MODE mirrorMode)
    {
        if (vertexBuffer == null)
        {
            return null;
        }
        else
        {
            Vector3[] vnData = new Vector3[vertexBuffer.Length];
            for (int i = 0; i < vertexBuffer.Length; ++i)
            {
                VertexStructure vs = vertexBuffer[i];
                vnData[i].x = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_X) != 0 ? vs.i : -vs.i;
                vnData[i].y = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_Y) != 0 ? vs.j : -vs.j;
                vnData[i].z = ((uint)mirrorMode & (uint)MIRROR_MODE.MIRROR_Z) != 0 ? vs.k : -vs.k;
            }
            return vnData;
        }
    }

    /// <summary>
    /// Can be used to extract vertex uvs from the returned vertex buffer. Better to use createModelMeshData directly to avoid multiple unnecessary calls if you can do that!
    /// </summary>
    /// <param name="vertexBuffer"></param>
    /// <returns>Copy of the uv-values as extracted from the buffer</returns>
    public static Vector2[] extractVertexUvDataFrom(VertexStructure[] vertexBuffer)
    {
        if (vertexBuffer == null)
        {
            return null;
        }
        else
        {
            Vector2[] uvData = new Vector2[vertexBuffer.Length];
            for (int i = 0; i < vertexBuffer.Length; ++i)
            {
                VertexStructure vs = vertexBuffer[i];
                uvData[i].x = vs.u;
                uvData[i].y = vs.v;
            }
            return uvData;
        }
    }

    /// <summary>
    /// Useful to get a copy of the vertex-data for the given mesh in the given model. Because we are
    /// returning a copy, instead of the pointers to the native memory
    /// </summary>
    /// <param name="handle">The handle of the given model</param>
    /// <param name="meshIndex">The index of the mesh in that model</param>
    /// <returns>Copy of the vertex data</returns>
    public static VertexStructure[] getModelMeshVertexDataCopy(int handle, int meshIndex)
    {
        // Fetch pointer to the native data
        IntPtr ptrNativeData;
        int nativeDataLength = getModelMeshVertexData(handle, meshIndex, out ptrNativeData);

        // Check error (indicated by negative length)
        if (nativeDataLength < 0)
        {
            // An error has happened, maybe throw exception instead of this?
            return new VertexStructure[0];
        }

        // Copy data using struct-marshalling
        VertexStructure[] vertexArray = new VertexStructure[nativeDataLength];
        IntPtr p = ptrNativeData;
        for (int i = 0; i < nativeDataLength; ++i)
        {
            // Marshal data out as struct
            vertexArray[i] = (VertexStructure)Marshal.PtrToStructure(p, typeof(VertexStructure));
            // Increment pointer by the size of the vertex structure
            long size = (Marshal.SizeOf(typeof(VertexStructure)));
            long newAddr = p.ToInt64() + size; // this ensures both 32 and 64 bit support!
            p = new IntPtr(newAddr);
        }

        // return the array as we have found it
        return vertexArray;
    }

    /// <summary>
    /// Get the (possibly transformed) copy of the indices
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh</param>
    /// <param name="mirrorMode">The mirroring mode as the transformation - because mirroring vertex-data changes winding order of triangles, you should provide the same value here as with methods that extract vertex data!</param>
    /// <param name="resetToZeroOffset">When set, the returned copy of indices are being "reseted" so in case they are referring to the first vertex data of this mesh, they have the value of 0, the second the value of 1 and so on. In case no reset would happen, the results depend on how this is stored in the c++ side (we might share vertex buffers there for optimalization or something). The default value for this is true, just there is a small speedup when this is false and you can access the real values.</param>
    /// <returns>The (possibly transformed) copy of the indices</returns>
    public static UInt32[] getModelMeshIndicesCopy(int handle, int meshIndex, MIRROR_MODE mirrorMode, bool resetToZeroOffset = true)
    {
        // Fetch pointer to the native data
        IntPtr ptrNativeData;
        int nativeDataLength = getModelMeshIndices(handle, meshIndex, out ptrNativeData);

        // Check error (indicated by negative length)
        if (nativeDataLength < 0)
        {
            // An error has happened, maybe throw exception instead of this?
            return new UInt32[0];
        }

        UInt32[] indexArray = new UInt32[nativeDataLength];
        IntPtr p = ptrNativeData;
        // TODO: this works only if the indices are containing triangle data!
        int currentTrianglePointNo = 0; // 0, 1, 2, 0, 1, 2, ...
        int size = (Marshal.SizeOf(typeof(UInt32)));
        // See what is the offset - we spare native call if we can
        int baseVertexOffset = 0;
        if (resetToZeroOffset)
        {
            // TODO: Ensure that this thinking is right. For some moments I had doubts this is how I should do...
            //       This only works if the vertex buffer is only shared THAT way that there can be duplications 
            //       and the pointed areas for mesh contains all data for that mesh! Otherwise this will fail...
            // Get the base offset that we can use to substract from indices to get per-mesh indices.
            baseVertexOffset = getModelMeshBaseVertexOffset(handle, meshIndex);
            // Just to give a little more endurance to the code. Should never happen to get zeroed here!
            baseVertexOffset = baseVertexOffset < 0 ? 0 : baseVertexOffset;
        }
        for (int i = 0; i < nativeDataLength; ++i)
        {
            // Calculate index expander value
            // this is for changing the winding order of the triangles when mirroring is in effect
            int indexpander = 0;
            if (mirrorMode != MIRROR_MODE.NONE)
            {
                // Possibly exchange the B and C point indices for the A-B-C triangle - we leave A always as it is:
                if (currentTrianglePointNo > 0)
                {
                    //// A change should happen if we mirror odd times (so mirror over only X, only Y, only Z or all XYZ, etc)
                    //if(mirrorMode == MIRROR_MODE.MIRROR_X || mirrorMode == MIRROR_MODE.MIRROR_Y || mirrorMode == MIRROR_MODE.MIRROR_Z
                    //    || mirrorMode == MIRROR_MODE.MIRROR_XYZ)
                    //{
                    // It becomes 0 here when we are at point B
                    // and becomes 1 here when we are at point C
                    indexpander = currentTrianglePointNo - 1;
                    // It becomes 1 when we are at point B
                    // and becomes -1 when we are at point C
                    indexpander = -((indexpander * 2) - 1);
                    //}
                }
            }

            // Get the pointed uint32_t value into our copy-array
            // Cast works here without data loss, see discussion at:
            // https://social.msdn.microsoft.com/Forums/vstudio/en-US/012d583d-dd88-45ac-ac81-abb72b54d6c5/marshalreadint32-returning-uint32?forum=csharpgeneral
            indexArray[i + indexpander] = (UInt32)(Marshal.ReadInt32(p, i * size) - baseVertexOffset); // Rem.: The second ReadInt32 offset param is in bytes!

            // update the indicator that holds information about which triangle we use
            currentTrianglePointNo = (currentTrianglePointNo + 1) % 3;

            // Rem.: No need to increment pointer by the size of the uint32 because offset usage above!
        }

        // return the copy of the data
        return indexArray;
    }
    #endregion
    #region Unity-friendly mesh-data class
    /// <summary>
    /// More unity-friendly mesh data. See various get*MeshData(...) and other get*(...) methods for individual aquisition.
    /// </summary>
    public class MeshData
    {
        private string _objMatFaceGroupName;
        /// <summary>
        /// The full name of the objMatFaceGroup this mesh contains data for. This contains the group name, the many "group-encoded" (':<key>:<value>') things and the material name also encoded as ':mtl:<name>' as the last group-encoded thing.
        /// </summary>
        public string objMatFaceGroupName
        {
            get
            {
                return _objMatFaceGroupName;
            }
        }
        private string _objGroupName;
        /// <summary>
        /// The group name(*) this mesh belongs to. More than one mesh can belong to the same group, so one should create individual empty game objects for the meshes if groups to be handled.
        /// Basically this is the prefix of the objMatFaceGroupName until the very first ':' character (or as whole if there is no such character)
        /// (*) Rem.: This way the field here groups together all *.obj groups that only differ after the first ':' characted encountered! This might be unexpected by some actually but this way we can use group-hidden meta-data!
        /// </summary>
		public string objGroupName { get { return _objGroupName; } }
        private string _materialName;
        /// <summary>
        /// The name of the material - it can be empty for the default materials!
        /// </summary>
		public string materialName { get { return _materialName; } }
        private SimpleMaterial _simpleMaterial;
        /// <summary>
        /// The data of the material
        /// </summary>
		public SimpleMaterial simpleMaterial { get { return _simpleMaterial; } }
        protected string _ambientTexture;
        public string ambientTexture { get { return _ambientTexture; } }
        protected string _diffuseTexture;
        public string diffuseTexture { get { return _diffuseTexture; } }
        protected string _specularTexture;
        public string specularTexture { get { return _specularTexture; } }
        protected string _normalTexture;
        public string normalTexture { get { return _normalTexture; } }
        protected Vector3[] _vertices;
        public Vector3[] vertices { get { return _vertices; } }
        protected Vector3[] _normals;
        public Vector3[] normals { get { return _normals; } }
        protected Vector2[] _uv;
        public Vector2[] uv { get { return _uv; } }
        protected int[] _triangles;
        public int[] triangles { get { return _triangles; } }

        /// <summary>
        /// Returns true if the given triangle at beginIndex is three short index according to the maxShortIndex value - false otherwise.
        /// This method do not range-check!
        /// </summary>
        /// <param name="triangles">The array we search in</param>
        /// <param name="beginIndex">The index in the triangles array where the A index of the ABC triangle is at.</param>
        /// <param name="maxShortIndex">The criteria for checking against the indices</param>
        private static bool isAllShortIndexTriangle(int[] triangles, int beginIndex, uint maxShortIndex)
        {
            return (triangles[beginIndex] < maxShortIndex) &&
                (triangles[beginIndex + 1] < maxShortIndex) &&
                (triangles[beginIndex + 2] < maxShortIndex);
        }

        /// <summary>
        /// A helper class for calculating which vertex indices are used in a triangle data indices lists and which are not. Also counts how many are indeed in use.
        /// The helper class is very useful for manipulating according to short indices (see shortIndexHeadTailCut implementation)
        /// </summary>
        public class VertIndexUsageCalculatorHelper
        {
            /// <summary>
            /// Maximum vertex-data indexing value (for indices buffer)
            /// </summary>
            private readonly uint maxIndexNo;
            /// <summary>
            /// Full hash of all previously added indices - and mapping them to their new values after the transformation!
            /// </summary>
            private Dictionary<int, int> vertIndexConvertor = new Dictionary<int, int>();
            /// <summary>
            /// The currently different vertex-data indexing values that are in use.
            /// Also used to calculate the new indices (values for the keys) when adding to the dictionary.
            /// </summary>
            private uint usedVertIndNo = 0;

            /// <summary>
            /// Gets how many differend vertex-data indices are used - basically the number of vertex-data in the target list after convertion / computation.
            /// </summary>
            /// <returns></returns>
            public uint getUsedVertIndNo()
            {
                return usedVertIndNo;
            }

            /// <summary>
            /// Create the helper class for calculating collected constrained index usage of vertex-data
            /// </summary>
            /// <param name="maxIndexNo">The constrain of how many different indices we might have</param>
            public VertIndexUsageCalculatorHelper(uint maxIndexNo)
            {
                this.maxIndexNo = maxIndexNo;
            }

            /// <summary>
            /// Gets the set of vertex-data indices that appeared in the construction - and their mapping to the new indices after the reordering.
            /// Basically an oldIndex -> newIndex mapping that contains at most maxIndexNo elements as configured in the constructor.
            /// </summary>
            /// <returns>oldIndex -> newIndex mapping</returns>
            public Dictionary<int, int> getVertIndexConvertor()
            {
                return vertIndexConvertor;
            }

            /// <summary>
            /// Try to add vertices for an ABC triangle to the current set. Either adds all the vertices to the set or none of them.
            /// Rem.: Because here we can only add whole triangles integrity is ensured: no half triangles or dangling data can be filled!
            /// </summary>
            /// <param name="a">A-point index</param>
            /// <param name="b">B-point index</param>
            /// <param name="c">C-point index</param>
            /// <param name="maxShortIndex">The maximum vertexNo</param>
            /// <returns>true when successfully added and the already added vertices and this one can be handled smaller than maxIndexNo number of different indices</returns>
            public bool tryToAddTrivertIndex(int a, int b, int c)
            {
                // All index might be one that we already have, in which case the used vertIndNo should not get incremented at all.
                // We only need to increment for each addition that adds a non-exsisting vertex-data index!
                // In cases we already reached the limit
                uint inca = 0;
                uint incb = 0;
                uint incc = 0;
                // Rem.: We need to take care of the case of degenarate triangles!!!
                if (!vertIndexConvertor.ContainsKey(a)) ++inca;
                if (!vertIndexConvertor.ContainsKey(b) && (a != b)) ++incb;
                if (!vertIndexConvertor.ContainsKey(c) && (a != c) && (b != c)) ++incc;
                uint increment = inca + incb + incc;

                // Check if there is place for the ones we want to add now.
                if (usedVertIndNo + increment > maxIndexNo)
                {
                    // Indicate that we could not add this triangle!
                    return false;
                }
                else
                {
                    // Add indices to our hash - this will not duplicate if it already exists!
                    // The key is the original index and the value can be used to construct a new verex data with indices.

                    if (inca != 0)
                    {
                        vertIndexConvertor.Add(a, (int)usedVertIndNo);
                        ++usedVertIndNo;
                    }
                    if (incb != 0)
                    {
                        vertIndexConvertor.Add(b, (int)usedVertIndNo);
                        ++usedVertIndNo;
                    }
                    if (incc != 0)
                    {
                        vertIndexConvertor.Add(c, (int)usedVertIndNo);
                        ++usedVertIndNo;
                    }

                    // Indicate that we successfully added this!
                    // The user-code can rely on this 
                    return true;
                }
            }
        }

        /// <summary>
        /// Breaks the current mesh data into a head:tail pair where the head will contain at most "maxShortIndex" number of (vertex, normal, uv) elements.
        /// This ensures that small "short" indexing can be used in the targets - like for example 16bit indices.
        /// Rem.: The tail might contain non-short indices in which case further application of this very same method helps ;-)
        /// </summary>
        /// <param name="maxShortIndex">Only triangle indices smaller than this value will happen in the generatd head so that head can be indexed with short indices after the operation.</param>
        /// <returns>head:tail - where the head can be indexed with the short indices and the tail contains all the rest of the data. The tail can be null if there is no tail data.</returns>
        public KeyValuePair<MeshData, MeshData> shortIndexHeadTailCut(uint maxShortIndex)
        {
            // Rem.: KeyValuePair is used instead of Tuple because many Mono versions does not supply proper stuff for the latter!
            if (vertices.Length < maxShortIndex)
            {
                // We are already smaller than the target short index so the tail is just null!
                // RETURNS A REFERENCE - but still we don't change our object at least.
                return new KeyValuePair<MeshData, MeshData>(this, null);
            }
            else
            {
                // //////////////// //
                // TRIANGLE COLLECT //
                // //////////////// //
                // First collect which triangles can go into the "head" and which into the "tail"
                List<int> origTrianglesToHead = new List<int>(); // index triplets for the head - those among the original indices that will go to the head!
                List<int> initialOriginalTrianglesToTail = new List<int>(); // index triplets for the tail - not the final: data after the first special run will be here!
                // We also collect which vertex indices we are using and the total count  and such things with a helper class
                VertIndexUsageCalculatorHelper headCalculator = new VertIndexUsageCalculatorHelper(maxShortIndex);  // constrained index value for the head
                VertIndexUsageCalculatorHelper tailCalculator = new VertIndexUsageCalculatorHelper(int.MaxValue);   // unconstrained index value for the tail

                // CREATE INITIAL HEAD:TAIL
                // ------------------------
                // All triangles that only have smaller indices than maxShortIndex go into the head.
                // This way we collect a "starting-data" for the head in a way that they can be
                // surely represented by indices less than the target value.
                for (int i = 0; i < triangles.Length; i += 3)
                {
                    if (isAllShortIndexTriangle(triangles, i, maxShortIndex))
                    {
                        // Surely we can add this triangle with not using more varieties of indices than maxShortIndex later when indexing optimally.
                        // Just think about it!
                        origTrianglesToHead.Add(triangles[i]);
                        origTrianglesToHead.Add(triangles[i + 1]);
                        origTrianglesToHead.Add(triangles[i + 2]);
                        bool b = headCalculator.tryToAddTrivertIndex(triangles[i], triangles[i + 1], triangles[i + 2]);
                        if (!b) throw new NotSupportedException("Code is broken! Should never happen!"); // Defensive coding assert
                    }
                    else
                    {
                        // All other triangles are going to the tail initially
                        initialOriginalTrianglesToTail.Add(triangles[i]);
                        initialOriginalTrianglesToTail.Add(triangles[i + 1]);
                        initialOriginalTrianglesToTail.Add(triangles[i + 2]);
                    }
                }

                // CREATE FINAL HEAD:TAIL
                // ----------------------
                // After doing the above, we might still have a lot of "space" in the head.
                // So we will try to fill in a little more triangles to aid proper cutting.
                // For this latter operation we need to be more careful as we never know
                // which triangles we can add and which not...

                // Try finding more candidates from the tail that we can add
                // for those we add from the tail we create a skipping list.
                List<int> tailSkippingList3 = new List<int>(); // "3": When we have an index in here we need to also skip the two ones after that one!!!! (also is automagically sorted)
                for (int i = 0; i < initialOriginalTrianglesToTail.Count; i += 3)
                {
                    // Add more if we can
                    if (headCalculator.tryToAddTrivertIndex(initialOriginalTrianglesToTail[i], initialOriginalTrianglesToTail[i + 1], initialOriginalTrianglesToTail[i + 2]))
                    {
                        // Add these to the head list (with source index values that can serve as keys in the calculator!)
                        origTrianglesToHead.Add(initialOriginalTrianglesToTail[i]);
                        origTrianglesToHead.Add(initialOriginalTrianglesToTail[i + 1]);
                        origTrianglesToHead.Add(initialOriginalTrianglesToTail[i + 2]);
                        // Update the skipping-list of the tail. So that we see which indexes in the tail we skip over
                        tailSkippingList3.Add(i);
                    }
                    else
                    {
                        // Real tail values need calculator only - the above don't needed it so we only do this here!
                        // The tail calculator is unconstrained so this should always return true and we should always be able to add
                        // We only do this here so that the old->new mapping is also constructed in this case that belong to the tail!
                        bool b = tailCalculator.tryToAddTrivertIndex(initialOriginalTrianglesToTail[i], initialOriginalTrianglesToTail[i + 1], initialOriginalTrianglesToTail[i + 2]);
                        if (!b) throw new NotSupportedException("Code is broken! Should never happen!"); // Defensive coding assert
                    }
                }

                // ///////////////// //
                // MESH-DATA FILL-IN //
                // ///////////////// //

                // We need to create the head-tail cross-cutting copies of our object
                // Tricky algorithm to get the head vertices and triangles! This is not
                // so much better than just doing 20k triangles and copying vertices and such,
                // but just a little bit tries to be more heuristical in using up shared vertices...

                // HEAD
                // ----

                // Index of earlier data will go here to places with the mapped index in the calculator when filled!
                Vector3[] headVertices = new Vector3[headCalculator.getUsedVertIndNo()];
                Vector3[] headNormals = new Vector3[headCalculator.getUsedVertIndNo()];
                Vector2[] headUvs = new Vector2[headCalculator.getUsedVertIndNo()];
                // Into here we need converted index data
                int[] headTriangles = new int[origTrianglesToHead.Count];

                // With this map, we can "convert" where this or that old index's data will go in the new vertex datas.
                // So basically we can tell where our corresponding new index is.
                Dictionary<int, int> headConv = headCalculator.getVertIndexConvertor();
                for (int i = 0; i < origTrianglesToHead.Count; i += 3)
                {
                    // Grab those triangles data that gets transferred to the head
                    // Origindices
                    int origa = origTrianglesToHead[i];
                    int origb = origTrianglesToHead[i + 1];
                    int origc = origTrianglesToHead[i + 2];
                    // New indices
                    int newa = headConv[origa];
                    int newb = headConv[origb];
                    int newc = headConv[origc];

                    // Apply data conversion for vertex-datas so that the new indices to the new triangle will point to them
                    // The mapping is necessary because of data-sharing optimizations and stuff. This should fill the data near-completely!
                    // vertices:
                    headVertices[newa] = this.vertices[origa];
                    headVertices[newb] = this.vertices[origb];
                    headVertices[newc] = this.vertices[origc];
                    // normals:
                    headNormals[newa] = this.normals[origa];
                    headNormals[newb] = this.normals[origb];
                    headNormals[newc] = this.normals[origc];
                    // uvs:
                    headUvs[newa] = this.uv[origa];
                    headUvs[newb] = this.uv[origb];
                    headUvs[newc] = this.uv[origc];

                    // Generate triangle indices pointing to the data vector properly.
                    headTriangles[i] = newa;
                    headTriangles[i + 1] = newb;
                    headTriangles[i + 2] = newc;
                }

                // If we are here, the data for the head mesh should have been already prepared
                MeshData headMesh = new MeshData();
                // Set the newly generated data (so we are not referencing the source object here at all)
                headMesh._vertices = headVertices;
                headMesh._normals = headNormals;
                headMesh._uv = headUvs;
                headMesh._triangles = headTriangles;
                // Set references for unchanged data like names and textures which can be just referenced back to this
                headMesh._objMatFaceGroupName = this._objMatFaceGroupName;
                headMesh._objGroupName = this._objGroupName;
                headMesh._materialName = this._materialName;
                headMesh._simpleMaterial = this._simpleMaterial;
                headMesh._ambientTexture = this._ambientTexture;
                headMesh._diffuseTexture = this._diffuseTexture;
                headMesh._specularTexture = this._specularTexture;
                headMesh._normalTexture = this._normalTexture;

                // TAIL
                // ----

                // Create the tail hash data with that skipping list also applied
                // Index of earlier data will go here to places with the mapped index in the calculator when filled!
                Vector3[] tailVertices = new Vector3[tailCalculator.getUsedVertIndNo()];
                Vector3[] tailNormals = new Vector3[tailCalculator.getUsedVertIndNo()];
                Vector2[] tailUvs = new Vector2[tailCalculator.getUsedVertIndNo()];
                // Into here we need converted index data
                // Rem.: The size is like such becase we had an original list of tail triangles (that is what we still have) but we might need to skip elements in that.
                //       The skipping list however only contains index for each second-run head-put triangles (3 consequtive indbuf elems) to spare some space and ensure skip integrity!
                int[] tailTriangles = new int[initialOriginalTrianglesToTail.Count - 3 * tailSkippingList3.Count];

                // With this map, we can "convert" where this or that old index's data will go in the new vertex datas.
                // So basically we can tell where our corresponding new index is.
                Dictionary<int, int> tailConv = tailCalculator.getVertIndexConvertor();
                // We iterate over all triangles we considered originally, but might "skip" in this list those that the second run could still add to the head!
                // This way we have saved ourselves ffrom some maybe long memcpy at least... hopefully...
                // (!) The skipping-list contains list index values to skip in an incremental fashion (see above) so we can rely on this and only store the "current" one we wait for.
                //     When there is no current index to skip, this should contain (-1). The latter can happen if the skipping list is empty and after we are through the last skip.
                int currentSkipIndexIndex = 0;
                int currentSkipIndex = (tailSkippingList3.Count > currentSkipIndexIndex) ? tailSkippingList3[currentSkipIndexIndex] : (-1); // set first index to skip over
                for (int i = 0, j = 0; i < initialOriginalTrianglesToTail.Count; i += 3) // j is incremented only when there is no skip, so it cannot be here!
                {
                    // Skipping over:
                    if (i == currentSkipIndex)
                    {
                        // Move the skipping-list index to its next
                        ++currentSkipIndexIndex;
                        currentSkipIndex = (tailSkippingList3.Count > currentSkipIndexIndex) ? tailSkippingList3[currentSkipIndexIndex] : (-1); // set index to skip over
                        // and continue the loop by skipping this elem
                        continue;
                    }

                    // Tail data creation:
                    // Grab those triangles data that gets transferred to the tail
                    // Origindices
                    int origa = initialOriginalTrianglesToTail[i];
                    int origb = initialOriginalTrianglesToTail[i + 1];
                    int origc = initialOriginalTrianglesToTail[i + 2];
                    // New indices
                    int newa = tailConv[origa];
                    int newb = tailConv[origb];
                    int newc = tailConv[origc];

                    // Apply data conversion for vertex-datas so that the new indices to the new triangle will point to them
                    // The mapping is necessary because of data-sharing optimizations and stuff. This should fill the data near-completely!
                    // vertices:
                    tailVertices[newa] = this.vertices[origa];
                    tailVertices[newb] = this.vertices[origb];
                    tailVertices[newc] = this.vertices[origc];
                    // normals:
                    tailNormals[newa] = this.normals[origa];
                    tailNormals[newb] = this.normals[origb];
                    tailNormals[newc] = this.normals[origc];
                    // uvs:
                    tailUvs[newa] = this.uv[origa];
                    tailUvs[newb] = this.uv[origb];
                    tailUvs[newc] = this.uv[origc];

                    // Generate triangle indices pointing to the data vector properly.
                    tailTriangles[j] = newa;
                    tailTriangles[j + 1] = newb;
                    tailTriangles[j + 2] = newc;
                    j += 3; // increment j (here because we only do this when there was no skip)
                }

                // Fill-in the tail hash-data
                MeshData tailMesh = new MeshData();
                // Set the newly generated data (so we are not referencing the source object here at all)
                tailMesh._vertices = tailVertices;
                tailMesh._normals = tailNormals;
                tailMesh._uv = tailUvs;
                tailMesh._triangles = tailTriangles;
                // Set references for unchanged data like names and textures which can be just referenced back to this
                tailMesh._objMatFaceGroupName = this._objMatFaceGroupName;
                tailMesh._objGroupName = this._objGroupName;
                tailMesh._materialName = this._materialName;
                tailMesh._simpleMaterial = this._simpleMaterial;
                tailMesh._ambientTexture = this._ambientTexture;
                tailMesh._diffuseTexture = this._diffuseTexture;
                tailMesh._specularTexture = this._specularTexture;
                tailMesh._normalTexture = this._normalTexture;

                // Returns only copies to relevant data to keep our object unchanged...
                KeyValuePair<MeshData, MeshData> ret = new KeyValuePair<MeshData, MeshData>(headMesh, tailMesh);
                return ret;
            }
        }

        /// <summary>
        /// Returns a copy of the triangle indices, with every index incremented by the "baseIndexOffset" value. Useful for manipulating submeshes or breaking them into more than one submesh.
        /// </summary>
        /// <param name="baseIndexOffset">The offset for each triangle index to increment with</param>
        /// <returns>A copy of the "offseted" triangle array</returns>
        public int[] getBaseIndexOffsetedTrianglesCopy(int baseIndexOffset)
        {
            // Create the offseted copy (useful for re-creating of submeshes)
            int[] ret = new int[triangles.Length];
            for (int ek = 0; ek < triangles.Length; ++ek)
            {
                ret[ek] = triangles[ek] + baseIndexOffset;
            }

            // Return this copy
            return ret;
        }

        /// <summary>
        /// This is the method that one should use in most of the cases
        /// </summary>
        /// <param name="handle">The handle of the given model</param>
        /// <param name="meshIndex">The index of the mesh in that model</param>
        /// <param name="mirrorMode">The mirroring mode</param>
        /// <returns></returns>
        public static MeshData createFromModelMeshData(int handle, int meshIndex, MIRROR_MODE mirrorMode = MIRROR_MODE.MIRROR_XY)
        {
            // Create empty data first
            MeshData md = new MeshData();

            // Fill material
            md._simpleMaterial = getModelMeshMaterial(handle, meshIndex);
            // Fill material name - the easiest to do this seperately
            md._materialName = getModelMeshMaterialName(handle, meshIndex);

            // Fill in mesh names and identifications
            string matFaceGroupName = getModelMeshObjMatFaceGroupName(handle, meshIndex);
            md._objMatFaceGroupName = matFaceGroupName;
            md._objGroupName = matFaceGroupName.Split(OBJ_ANNOTATION_SEP_CHAR)[0];

            // Fill possible textures
            md._ambientTexture = getModelMeshAmbientTextureFileName(handle, meshIndex);
            md._diffuseTexture = getModelMeshDiffuseTextureFileName(handle, meshIndex);
            md._specularTexture = getModelMeshSpecularTextureFileName(handle, meshIndex);
            md._normalTexture = getModelMeshNormalTextureFileName(handle, meshIndex);

            // Fill indices
            uint[] indices = getModelMeshIndicesCopy(handle, meshIndex, mirrorMode, true);
            // Unity seem to support only signed so I need this conversion code...
            int[] unitindices = new int[indices.Length];
            for (int i = 0; i < indices.Length; ++i)
            {
                unitindices[i] = (int)indices[i];
            }
            md._triangles = unitindices;
            // Get vertex buffer data as seperate arrays
            VertexStructure[] vertexBuffer = getModelMeshVertexDataCopy(handle, meshIndex);
            md._vertices = extractVertexPosDataFrom(vertexBuffer, mirrorMode);
            md._normals = extractVertexNormalDataFrom(vertexBuffer, mirrorMode);
            md._uv = extractVertexUvDataFrom(vertexBuffer);

            return md;
        }
    }
    #endregion
}

