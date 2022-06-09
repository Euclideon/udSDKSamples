using System;
using System.Runtime.InteropServices;

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

      public Attributes.udAttributeSet attributes; //!< The attributes in this model

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
    //!
    [StructLayout(LayoutKind.Sequential)]
    public struct udConvertItemInfo
    {
      //[MarshalAs(UnmanagedType.LPUTF8Str)]
      //public string pFilename; //!< Name of the input file
      public IntPtr pFilename;
      public Int64 pointsCount; //!< This might be an estimate, -1 is no estimate is available
      public UInt64 pointsRead; //!< Once conversation begins, this will give an indication of progress
      public double estimatedResolution; //!< The estimated scale of the item
      public int srid; //!< The calculated geospatial reference ID of the item
    };

    public class udConvertContext
    {
      public void Create(udContext context)
      {
        udError error = udConvert_CreateContext(context.pContext, ref pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void Destroy()
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
      public void SetFileName(string fileName)
      {
        udError error = udConvert_SetOutputFilename(pConvertContext, fileName);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void DoConvert()
      {
        udError error = udConvert_DoConvert(pConvertContext);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public IntPtr pConvertContext;

      [DllImport("udSDK")]
      private static extern udError udConvert_CreateContext(IntPtr pContext, ref IntPtr ppConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_DestroyContext(ref IntPtr ppConvertContext);

      [DllImport("udSDK")]
      private static extern udError udConvert_AddItem(IntPtr pConvertContext, string fileName);

      [DllImport("udSDK")]
      private static extern udError udConvert_SetOutputFilename(IntPtr pConvertContext, string fileName);

      [DllImport("udSDK")]
      private static extern udError udConvert_DoConvert(IntPtr pConvertContext);

    }
  }
}
