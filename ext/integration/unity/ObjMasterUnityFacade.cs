#define TEST // When enabled, the script tests for reaching to the DLL by using a simple array-sort test

using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Text;

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
public class ObjMasterUnityFacade : MonoBehaviour {

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

    /// <summary>
    /// Try loading the given model with the objmaster system. If the model is already loaded, the earlier handle is returned from cache!
    /// </summary>
    /// <param name="path">Path for the model - also search path of mtl file and relative names</param>
    /// <param name="fileName">Filename of the obj</param>
    /// <returns>A handle to reference this model or -1 in case of errors!</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "loadObjModel", CallingConvention = CallingConvention.Cdecl)]
    public static extern int loadObjModel(string path, string fileName);

    /// <summary>
    /// Try to unload the model, referenced by the handle. Beware when loading the same model multiple times and releasing without notifying the other subsystem!
    /// </summary>
    /// <param name="handle">The reference handle for the model to unload</param>
    /// <returns>True on success, false otherwise</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "unloadObjModel", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool unloadObjModel(int handle);

    /// <summary>
    /// Try to unload all models - and free all memory. This operation is useful
    /// if you do not only need to release your model resources, but also release
    /// the small amount of administrative caches. After calling this, there should
    /// be no memory leaks and left-overs unless there is a bug in the underlying library.
    /// </summary>
    /// <returns>false indicates that something went wrong and there might be a leak or inconsistency!</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "unloadEverything", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool unloadEverything();

    /// <summary>
    /// Returns the number of meshes a model is having.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <returns>-1 in case of errors or bad handle</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshNo", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshNo(int handle);

    /// <summary>
    /// Returns the SimpleMaterial of the designated model mesh. Also useful as it defines which texture names we can look for!
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The material.</returns>
    // Maybe needed ", CallingConvention = CallingConvention.Cdecl)]" ???
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshMaterial", CallingConvention = CallingConvention.Cdecl)]
    public static extern SimpleMaterial getModelMeshMaterial(int handle, int meshIndex);

    /// <summary>
    /// Tells the number of vertex data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all!
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>Number of vertex data for the given mesh</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshVertexDataCount", CallingConvention = CallingConvention.Cdecl)]
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
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshBaseVertexOffset", CallingConvention = CallingConvention.Cdecl)]
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
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshVertexData", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshVertexData(int handle, int meshIndex, out IntPtr pointer);

    /// <summary>
    /// Tells the number of index data for the given mesh of the handle.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns> Returns -1 in case of errors and zero when there is no data at all!</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshIndicesCount", CallingConvention = CallingConvention.Cdecl)]
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
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshIndices", CallingConvention = CallingConvention.Cdecl)]
    public static extern int getModelMeshIndices(int handle, int meshIndex, out IntPtr output);

    /// <summary>
    /// Returns a pointer to the CSTR of the Ambient texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshAmbientTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshAmbientTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Diffuse texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshDiffuseTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshDiffuseTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Specular texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshSpecularTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshSpecularTextureFileNamePtr(int handle, int meshIndex);
    /// <summary>
    /// Returns a pointer to the CSTR of the Normal texture filename. Returns nullptr in case of errors, and points to empty CSTR if there is no such texture.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <param name="meshIndex">The index of the mesh - should be smaller than getModelMeshNo</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshNormalTextureFileName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshNormalTextureFileNamePtr(int handle, int meshIndex);

    /// <summary>
    /// Returns the name of the material for the given mesh. The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors. An empty string might be returned too!</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshMaterialName", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshMaterialNamePtr(int handle, int meshIndex);

    /// <summary>
    /// Returns the name of the objMatFaceGroup - see objMaster - basically the 'g' and 'o' group name with added :mtl:<materialName>.
    /// The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
    /// </summary>
    /// <param name="handle">The handle of the *.obj file</param>
    /// <param name="meshIndex">The mesh index in that *.obj file</param>
    /// <returns>The pointer to the c-style string or nullptr in case of errors. An empty string should not be returned, but better expect that too as other side does not check!</returns>
    [DllImport("ObjMasterHololensUnity", EntryPoint = "getModelMeshObjMatFaceGroupNamePtr", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr getModelMeshObjMatFaceGroupNamePtr(int handle, int meshIndex);

    #endregion
    #region TEST
#if TEST
    // The imported test function
    [DllImport("ObjMasterHololensUnity", EntryPoint = "testSort")]
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

    void Start()
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
    #endregion
    #region Helper methods
    /// <summary>
    /// Returns the count of all vertices in the whole model with its all meshes. Basically this method just iterates over all mesh indexes and summarizes.
    /// </summary>
    /// <param name="handle">The handle of the model</param>
    /// <returns>The total number of vertices in the model. Tries to return negative values on errors.</returns>
    public static int getModelVertexDataCount(int handle)
    {
        if (handle >= 0) {
            int totalVertexNo = 0;
            for (int meshIndex = 0; meshIndex < getModelMeshNo(handle); ++meshIndex)
            {
                totalVertexNo += getModelMeshVertexDataCount(handle, meshIndex);
            }
            return totalVertexNo;
        } else
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
    /// Returns the name of the objMatFaceGroup - see objMaster - basically the 'g' and 'o' group name with added :mtl:<materialName>.
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
            if (IntPtr.Zero.Equals(ptr))
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
        catch(Exception)
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
        for (int i = 0; i < nativeDataLength; ++i) {
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
        for(int i = 0; i < nativeDataLength; ++i)
        {
            // Calculate index expander value
            // this is for changing the winding order of the triangles when mirroring is in effect
            int indexpander = 0;
            if(mirrorMode != MIRROR_MODE.NONE)
            {
                // Possibly exchange the B and C point indices for the A-B-C triangle - we leave A always as it is:
                if(currentTrianglePointNo > 0)
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
            indexArray[i + indexpander] =(UInt32) (Marshal.ReadInt32(p, i * size) - baseVertexOffset); // Rem.: The second ReadInt32 offset param is in bytes!

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
        /// <summary>
        /// The full name of the objMatFaceGroup this mesh contains data for. This contains the group name, the many "group-encoded" (':<key>:<value>') things and the material name also encoded as ':mtl:<name>' as the last group-encoded thing.
        /// </summary>
        public string objMatFaceGroupName;
        /// <summary>
        /// The group name(*) this mesh belongs to. More than one mesh can belong to the same group, so one should create individual empty game objects for the meshes if groups to be handled.
        /// Basically this is the prefix of the objMatFaceGroupName until the very first ':' character (or as whole if there is no such character)
        /// (*) Rem.: This way the field here groups together all *.obj groups that only differ after the first ':' characted encountered! This might be unexpected by some actually but this way we can use group-hidden meta-data!
        /// </summary>
        public string objGroupName;
        /// <summary>
        /// The name of the material - it can be empty for the default materials!
        /// </summary>
        public string materialName;
        /// <summary>
        /// The data of the material
        /// </summary>
        public SimpleMaterial simpleMaterial;
        public string ambientTexture;
        public string diffuseTexture;
        public string specularTexture;
        public string normalTexture;
        public Vector3[] vertices;
        public Vector3[] normals;
        public Vector2[] uv;
        public int[] triangles;

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
            md.simpleMaterial = getModelMeshMaterial(handle, meshIndex);
            // Fill material name - the easiest to do this seperately
            md.materialName = getModelMeshMaterialName(handle, meshIndex);

            // Fill possible textures
            md.ambientTexture = getModelMeshAmbientTextureFileName(handle, meshIndex);
            md.diffuseTexture = getModelMeshDiffuseTextureFileName(handle, meshIndex);
            md.specularTexture = getModelMeshSpecularTextureFileName(handle, meshIndex);
            md.normalTexture = getModelMeshNormalTextureFileName(handle, meshIndex);

            // Fill indices
            uint[] indices = getModelMeshIndicesCopy(handle, meshIndex, mirrorMode, true);
            // Unity seem to support only signed so I need this conversion code...
            int[] unitindices = new int[indices.Length];
            for(int i = 0; i < indices.Length; ++i)
            {
                unitindices[i] = (int)indices[i];
            }
            md.triangles = unitindices;

            // Get vertex buffer data as seperate arrays
            VertexStructure[] vertexBuffer = getModelMeshVertexDataCopy(handle, meshIndex);
            md.vertices = extractVertexPosDataFrom(vertexBuffer, mirrorMode);
            md.normals = extractVertexNormalDataFrom(vertexBuffer, mirrorMode);
            md.uv = extractVertexUvDataFrom(vertexBuffer);

            return md;
        }
    }
    #endregion
}
