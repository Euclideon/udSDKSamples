using System;
using System.Runtime.InteropServices;
using System.Text.Json;
namespace Euclideon.udSDK
{
  namespace Convert
  {
    [StructLayout(LayoutKind.Sequential)]
    public struct ConvertStatus
    {
      public UInt64 totalPointsRead; //!< How many points have been read in this model
      public UInt64 totalItems; //!< How many items are in the list

      // These are quick stats while converting
      public UInt64 currentInputItem; //!< The index of the item that is currently being read
      public UInt64 outputFileSize; //!< Size of the result UDS file
      public UInt64 sourcePointCount; //!< Number of points added (may include duplicates or out of range points)
      public UInt64 uniquePointCount; //!< Number of unique points in the final model
      public UInt64 discardedPointCount; //!< Number of duplicate or ignored out of range points
      public UInt64 outputPointCount; //!< Number of points written to UDS (can be used for progress)
      public UInt64 peakDiskUsage; //!< Peak amount of disk space used including both temp files and the actual output file
      public UInt64 peakTempFileUsage; //!< Peak amount of disk space that contained temp files
      public UInt32 peakTempFileCount; //!< Peak number of temporary files written
    };

    [StructLayout(LayoutKind.Sequential)]
    struct udConvertInfo
    {
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pOutputName; //!< The output filename
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pTempFilesPrefix; //!< The file prefix for temp files

      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pMetadata; //!< The metadata that will be added to this model (in JSON format)

      public Attributes.udAttributeSetInternal attributes; //!< The attributes in this model

      public Int32 ignoredAttributesLength; //!< The length of the ignored attributes list
      public IntPtr ppIgnoredAttributes; //!< The list of ignored attributes

      [MarshalAs(UnmanagedType.ByValArray, SizeConst =3)]
      public double[] globalOffset; //!< This amount is added to every point during conversion. Useful for moving the origin of the entire scene to geolocate

      public double minPointResolution; //!< The native resolution of the highest resolution file
      public double maxPointResolution; //!< The native resolution of the lowest resolution file
      public UInt32 skipErrorsWherePossible; //!< If not 0 it will continue processing other files if a file is detected as corrupt or incorrect

      public UInt32 everyNth; //!< If this value is >1, only every Nth point is included in the model. e.g. 4 means only every 4th point will be included, skipping 3/4 of the points
      public UInt32 polygonVerticesOnly; //!< If not 0 it will skip rasterization of polygons in favour of just processing the vertices
      public UInt32 retainPrimitives; //!< If not 0 rasterised primitives such as triangles/lines/etc are retained to be rendered at finer resolution if required at runtime
      public UInt32 bakeLighting; //!< if not 0 bake the normals into the colour channel in the output UDS file
      public UInt32 exportOtherEmbeddedAssets; //!< if not 0 export images contained in e57 files

      public UInt32 overrideResolution; //!< Set to not 0 to stop the resolution from being recalculated
      public double pointResolution; //!< The scale to be used in the conversion (either calculated or overriden)

      public UInt32 overrideSRID; //!< Set to not 0 to prevent the SRID being recalculated
      public int srid; //!< The geospatial reference ID (either calculated or overriden)
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pWKT; //!< The geospatial WKT string
      public ConvertStatus status;
    };

    //!
    //! @struct udConvertItemInfo
    //! Provides a copy of a subset of a convert item state
    //! @Warning this struct is larger than the C++ side struct, arrays of this struct passed to and from the native library will cause corruption
    [StructLayout(LayoutKind.Sequential)]
    public struct udConvertItemInfo
    {
      //[MarshalAs(UnmanagedType.LPUTF8Str)]
      //public string pFilename; //!< Name of the input file
      public IntPtr pFilename;
      public Int64 pointsCount; //!< This might be an estimate, -1 is no estimate is available
      public UInt64 pointsRead; //!< Once conversation begins, this will give an indication of progress
      public double estimatedResolution; //!< The estimated scale of the item
      private int _srid; //!< The calculated geospatial reference ID of the item

      // the following are not included in the c++ header:
      udConvertContext convertContext;
      UInt64 index;

      public udConvertItemInfo(udConvertContext context, UInt64 index)
      {
        this.convertContext = context;
        this.index = index;
        pFilename = IntPtr.Zero;
        pointsCount = 0;
        pointsRead = 0;
        estimatedResolution = 0;
        _srid = 0;
      }

      [DllImport("udSDK")]
      private static extern udError udConvert_SetInputSourceProjection(IntPtr pConvertContext, UInt64 index, int srid);
      public int srid
      {
        get { return _srid; }
        set
        {
          udError error = udConvert_SetInputSourceProjection(convertContext.pConvertContext, index, value);
          if (error != udError.udE_Success)
          {
            throw new UDException(error);
          }
        }
      }
    };

    public class udConvertContext
    {
      private IntPtr pInfo;
      public IntPtr pConvertContext;
      public ConversionMetadata metadata;
      public InputItems inputItems;

      public ConvertStatus Status
      {
        get { return info.status; }
      }

      public udConvertContext(udContext context)
      {
        udError error = udConvert_CreateContext(context.pContext, ref pConvertContext);
        udConvert_GetInfo(pConvertContext, ref pInfo);
        metadata = new ConversionMetadata(this);
        inputItems = new InputItems(this);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      ~udConvertContext()
      {
        udError error = udConvert_DestroyContext(ref pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void AddFile(string fileName)
      {
        udError error = udConvert_AddItem(pConvertContext, fileName);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void DoConvert()
      {
        udError error = udConvert_DoConvert(pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      udConvertInfo info
      {
        get { return Marshal.PtrToStructure<udConvertInfo>(pInfo); }
      }

      public string OutputFile
      {
        get { return info.pOutputName; }
        set
        {
          udError code = udConvert_SetOutputFilename(pConvertContext, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public string TempDirectory
      {
        get { return info.pTempFilesPrefix; }
        set
        {
          udError code = udConvert_SetTempDirectory(pConvertContext, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public double PointResolution
      {
        get { return info.pointResolution; }
        set
        {
          udError code = udConvert_SetPointResolution(pConvertContext, 1, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool OverridePointResolution
      {
        get { return System.Convert.ToBoolean(info.overrideResolution); }
        set
        {
          if (!value)
          {
            udError code = udConvert_SetPointResolution(pConvertContext, 0, 0);
            if (code != udError.udE_Success)
              throw new UDException(code); 
          }
          else
          {
            throw new ArgumentException("Override resolution must be set using the PointResolution Property");
          }
        }
      }

      public int SRID
      {
        get { return info.srid; }
        set
        {
          udError code = udConvert_SetSRID(pConvertContext, 1, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool OverrideSRID
      {
        get { return System.Convert.ToBoolean(info.overrideSRID); }
        set
        {
          if (!value)
          {
            udError code = udConvert_SetSRID(pConvertContext, 0, 0);
            if (code != udError.udE_Success)
              throw new UDException(code); 
          }
          else
          {
            throw new ArgumentException("Override SRID must be set using the SRID Property");
          }
        }
      }

      public string WellKnownText
      {
        get { return info.pWKT; }
        set
        {
          udError code = udConvert_SetWKT(pConvertContext, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public double[] GlobalOffset
      {
        get { return info.globalOffset; }
        set
        {
          udError code = udConvert_SetGlobalOffset(pConvertContext, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool SkipErrors
      {
        get { return System.Convert.ToBoolean(info.skipErrorsWherePossible); }
        set
        {
          udError code = udConvert_SetSkipErrorsWherePossible(pConvertContext, System.Convert.ToUInt32(value));
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public uint EveryNth
      {
        get { return info.everyNth; }
        set
        {
          udError code = udConvert_SetEveryNth(pConvertContext, value);
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool PolygonVerticesOnly
      {
        get { return System.Convert.ToBoolean(info.polygonVerticesOnly); }
        set
        {
          udError code = udConvert_SetPolygonVerticesOnly(pConvertContext, System.Convert.ToUInt32(value));
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool RetainPrimitives
      {
        get { return System.Convert.ToBoolean(info.retainPrimitives); }
        set
        {
          udError code = udConvert_SetRetainPrimitives(pConvertContext, System.Convert.ToUInt32(value));
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool BakeLighting
      {
        get { return System.Convert.ToBoolean(info.bakeLighting); }
        set
        {
          udError code = udConvert_SetBakeLighting(pConvertContext, System.Convert.ToUInt32(value));
          if (code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      public bool ExportOtherEmbeddedAssets
      {
        get { return System.Convert.ToBoolean(info.exportOtherEmbeddedAssets); }
        set
        {
          udError error = udConvert_SetExportOtherEmbeddedAssets(pConvertContext, System.Convert.ToUInt32(value));
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }

      public udConvertItemInfo CurrentItem
      {
        get
        {
          ulong curr = Status.currentInputItem;
          if (curr >= Status.totalItems)
            curr = Status.totalItems - 1;

          return inputItems[(int)curr];
        }
      }

      public class InputItems
      {
        udConvertContext convertContext;
        public InputItems(udConvertContext convertContext)
        {
          this.convertContext = convertContext;
        }
        public udConvertItemInfo this[int index]
        {
          get
          {
            udConvertItemInfo itemInfo = new udConvertItemInfo(convertContext, (ulong) index);
            udError error = udConvert_GetItemInfo(convertContext.pConvertContext, (UInt32) index, ref itemInfo);
            if (error != udError.udE_Success)
              throw new UDException(error);
            return itemInfo;
          }
        }
      }

      public class ConversionMetadata
      {
        udConvertContext convertContext;
        public ConversionMetadata(udConvertContext convertContext)
        {
          this.convertContext = convertContext;
        }

        public void Remove(int index)
        {
          udConvert_RemoveItem(convertContext.pConvertContext, (ulong) index);
        }

        public string JsonString
        {
          get { return convertContext.info.pMetadata; }
        }
        public string this[string key]
        {
          // unfortunately every framework has a different method of parsing json strings in a generic fashion.
          // a getter for individual elements needs to be therefore specific to the platform
          set
          {
            udError error = udConvert_SetMetadata(convertContext.pConvertContext, key, value);
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public void Cancel()
      {
        udError error = udConvert_Cancel(pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
        
      }

      public void Reset()
      {
        udError error = udConvert_Reset(pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
        
      }

      public udPointCloud GeneratePreview()
      {
        udPointCloud ret = new udPointCloud();
        udError error = udConvert_GeneratePreview(pConvertContext, ref ret.pModel);
        if (error != udError.udE_Success)
          throw new UDException(error);
        return ret;
      }

      public void IgnoreAttribute(string attributeName)
      {
        udError error = udConvert_IgnoreAttribute(pConvertContext, attributeName);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void RestoreAttribute(string attributeName)
      {
        udError error = udConvert_RestoreAttribute(pConvertContext, attributeName);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udConvert_CreateContext(IntPtr pContext, ref IntPtr ppConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_DestroyContext(ref IntPtr ppConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_AddItem(IntPtr pConvertContext, string fileName);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetOutputFilename(IntPtr pConvertContext, string fileName);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetTempDirectory(IntPtr pConvertContext, string pFolder);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetPointResolution(IntPtr pConvertContext, UInt32 newVal, double pointResolutionMeters);

      [DllImport("udSDK")]
      private static extern udError udConvert_IgnoreAttribute(IntPtr pConvertContext, string pAttributeName);

      [DllImport("udSDK")]
      private static extern udError udConvert_RestoreAttribute(IntPtr pConvertContext, string pAttributeName);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetAttributePrefix(IntPtr pConvertContext, string pAttributeName, string pPrefix);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetAttributeSuffix(IntPtr pConvertContext, string pAttributeName, string pSuffix);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetSRID(IntPtr pConvertContext, UInt32 newVal, int srid);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetWKT(IntPtr pConvertContext, string pWKT);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetGlobalOffset(IntPtr pConvertContext, double[] globalOffset);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetSkipErrorsWherePossible(IntPtr pConvertContext, UInt32 ignoreParseErrorsWherePossible);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetEveryNth(IntPtr pConvertContext, UInt32 everyNth);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetPolygonVerticesOnly(IntPtr pConvertContext, UInt32 polygonVerticesOnly);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetRetainPrimitives(IntPtr pConvertContext, UInt32 retainPrimitives);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetBakeLighting(IntPtr pConvertContext, UInt32 bakeLighting);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetExportOtherEmbeddedAssets(IntPtr pConvertContext, UInt32 exportImages);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetMetadata(IntPtr pConvertContext, string pMetadataKey, string pMetadataValue);

      [DllImport("udSDK")]
      private static extern udError udConvert_RemoveItem(IntPtr pConvertContext, UInt64 index);

      [DllImport("udSDK")]
      private static extern udError udConvert_Cancel(IntPtr pConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_Reset(IntPtr pConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_GeneratePreview(IntPtr pConvertContext, ref IntPtr ppCloud);

      //[DllImport("udSDK")]
      //private static extern udError udConvert_SetPostProcessCallback(IntPtr pContext, PostProcessCallback callback, void *pUserData, void(*pCleanUpUserData)(void*));

      //[DllImport("udSDK")]
      //private static extern udError udConvert_AddOutputAttribute(IntPtr pContext, struct udAttributeDescriptor *pAttribute);

      //[DllImport("udSDK")]
      //private static extern udError udConvert_RemoveOutputAttribute(IntPtr pContext, UInt32 index);

      [DllImport("udSDK")]
      private static extern udError udConvert_DoConvert(IntPtr pConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_GetInfo(IntPtr pConvertContext, ref IntPtr ppInfo);

      [DllImport("udSDK")]
      private static extern udError udConvert_GetItemInfo(IntPtr pConvertContext, UInt64 index, ref udConvertItemInfo pInfo);
    }
  }
}
