using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  public enum udError
  {
    udE_Success, //!< Indicates the operation was successful

    udE_Failure, //!< A catch-all value that is rarely used, internally the below values are favored
    udE_NothingToDo, //!< The operation didn't specifically fail but it also didn't do anything
    udE_InternalError, //!< There was an internal error that could not be handled

    udE_NotInitialized, //!< The request can't be processed because an object hasn't been configured yet
    udE_InvalidConfiguration, //!< Something in the request is not correctly configured or has conflicting settings
    udE_InvalidParameter, //!< One or more parameters is not of the expected format
    udE_OutstandingReferences, //!< The requested operation failed because there are still references to this object

    udE_MemoryAllocationFailure, //!< udSDK wasn't able to allocate enough memory for the requested feature
    udE_CountExceeded, //!< An internal count was exceeded by the request, generally going beyond the end of a buffer or internal limit

    udE_NotFound, //!< The requested item wasn't found or isn't currently available

    udE_BufferTooSmall, //!< Either the provided buffer or an internal one wasn't big enough to fufill the request
    udE_FormatVariationNotSupported, //!< The supplied item is an unsupported variant of a supported format

    udE_ObjectTypeMismatch, //!< There was a mismatch between what was expected and what was found

    udE_CorruptData, //!< The data/file was corrupt

    udE_InputExhausted, //!< The input buffer was exhausted so no more processing can occur
    udE_OutputExhausted, //!< The output buffer was exhausted so no more processing can occur

    udE_CompressionError, //!< There was an error in compression or decompression
    udE_Unsupported, //!< This functionality has not yet been implemented (usually some combination of inputs isn't compatible yet)

    udE_Timeout, //!< The requested operation timed out. Trying again later may be successful

    udE_AlignmentRequired, //!< Memory alignment was required for the operation

    udE_DecryptionKeyRequired, //!< A decryption key is required and wasn't provided
    udE_DecryptionKeyMismatch, //!< The provided decryption key wasn't the required one

    udE_SignatureMismatch, //!< The digital signature did not match the expected signature

    udE_ObjectExpired, //!< The supplied object has expired

    udE_ParseError, //!< A requested resource or input was unable to be parsed

    udE_InternalCryptoError, //!< There was a low level cryptography issue

    udE_OutOfOrder, //!< There were inputs that were provided out of order
    udE_OutOfRange, //!< The inputs were outside the expected range

    udE_CalledMoreThanOnce, //!< This function was already called

    udE_ImageLoadFailure, //!< An image was unable to be parsed. This is usually an indication of either a corrupt or unsupported image format

    udE_StreamerNotInitialised, //!<  The streamer needs to be initialised before this function can be called

    udE_OpenFailure, //!< The requested resource was unable to be opened
    udE_CloseFailure, //!< The resource was unable to be closed
    udE_ReadFailure, //!< A requested resource was unable to be read
    udE_WriteFailure, //!< A requested resource was unable to be written
    udE_SocketError, //!< There was an issue with a socket problem

    udE_DatabaseError, //!< A database error occurred
    udE_ServerError, //!< The server reported an error trying to complete the request
    udE_AuthError, //!< The provided credentials were declined (usually email or password issue)
    udE_NotAllowed, //!< The requested operation is not allowed (usually this is because the operation isn't allowed in the current state)
    udE_InvalidLicense, //!< The required license isn't available or has expired

    udE_Pending, //!< A requested operation is pending.
    udE_Cancelled, //!< The requested operation was cancelled (usually by the user)
    udE_OutOfSync, //!< There is an inconsistency between the internal udSDK state and something external. This is usually because of a time difference between the local machine and a remote server
    udE_SessionExpired, //!< The udServer has terminated your session

    udE_ProxyError, //!< There was some issue with the provided proxy information (either a proxy is in the way or the provided proxy info wasn't correct)
    udE_ProxyAuthRequired, //!< A proxy has requested authentication
    udE_ExceededAllowedLimit, //!< The requested operation failed because it would exceed the allowed limits (generally used for exceeding server limits like number of projects)

    udE_RateLimited, //!< This functionality is currently being rate limited or has exhausted a shared resource. Trying again later may be successful
    udE_PremiumOnly, //!< The requested operation failed because the current session is not for a premium user
    udE_InProgress, //!< The requested operation is currently in progress

    udE_Count //!< Internally used to verify return values
  };

  public class UDException : Exception
  {
    public udError code;

    public UDException(udError code): base(udErrorUtils.ToString(code))
    {
      this.code = code; 
    }
  };

  public class udErrorUtils
  {
    [DllImport("udSDK")]
    private static extern IntPtr udError_GetErrorString(udError code);

    public static string ToString(udError code)
    {
      return Marshal.PtrToStringUTF8(udError_GetErrorString(code));
    }
  };

  //!
  //! @struct udVoxelID
  //! Combines the traverse context and node index to uniquely identify a node
  //!
  [StructLayout(LayoutKind.Sequential)]
  public struct udVoxelID
  {
    UInt64 index; //!< Internal index value
    IntPtr pTrav; //!< Internal traverse info
    IntPtr pRenderInfo; //!< Internal render info
  };

  [StructLayout(LayoutKind.Sequential)]
  public struct udPointCloudHeader
  {
    public double scaledRange; //!< The point cloud's range multiplied by the voxel size
    public double unitMeterScale; //!< The scale to apply to convert to/from metres (after scaledRange is applied to the unitCube)
    public uint totalLODLayers; //!< The total number of LOD layers in this octree
    public double convertedResolution; //!< The resolution this model was converted at
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
    public double[] storedMatrix; //!< This matrix is the 'default' internal matrix to go from a unit cube to the full size

    public Attributes.udAttributeSetInternal attributes;   //!< The attributes contained in this pointcloud

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] baseOffset; //!< The offset to the root of the pointcloud in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] pivot; //!< The pivot point of the model, in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] boundingBoxCenter; //!< The center of the bounding volume, in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] boundingBoxExtents; //!< The extents of the bounding volume, in unit cube space  }
  }

  public class udPointCloud
  {
    public udPointCloud(udContext context, string path)
    {
      Load(context, path, ref this.header);
    }

    public udPointCloud()
    {
    }

    private void Load(udContext context, string modelLocation, ref udPointCloudHeader header)
    {
      udError error = udPointCloud_Load(context.pContext, ref pModel, modelLocation, ref header);
      if (error != udError.udE_Success)
        throw new UDException(error);

      this.context = context;
    }

    private void Unload()
    {
      udError error = udPointCloud_Unload(ref pModel);
      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    ~udPointCloud()
    {
      Unload();
    }

    public void GetMetadata(ref string ppJSONMetadata)
    {
      udError error = udPointCloud_GetMetadata(pModel, ref ppJSONMetadata);
      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    public IntPtr pModel = IntPtr.Zero;
    private udContext context;
    public udPointCloudHeader header;

    [DllImport("udSDK")]
    private static extern udError udPointCloud_Load(IntPtr pContext, ref IntPtr ppModel, string modelLocation, ref udPointCloudHeader header);

    [DllImport("udSDK")]
    private static extern udError udPointCloud_Unload(ref IntPtr ppModel);

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetMetadata(IntPtr pModel, ref string ppJSONMetadata);

    //[DllImport("udSDK")]
    //private static extern udError udPointCloud_LoadAdv(IntPtr pContext, ref IntPtr ppModel, string modelLocation, ref udPointCloudHeader pHeader, ref udPointCloudLoadOptions pOptions);

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetHeader(IntPtr pPointCloud, ref udPointCloudHeader pHeader);

    public void GetHeader()
    {
      udPointCloudHeader ret = new udPointCloudHeader();
      udError error = udPointCloud_GetHeader(pModel, ref ret);
      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_Export(IntPtr pModel, string pExportFilename, IntPtr /* udGeometry*/ pFilter, ref float pProgress);

    public void Export(string filename, Geometry.udGeometry geom, ref float pProgress)
    {
      udError error = udPointCloud_Export(pModel, filename, geom.pGeometry, ref pProgress);
      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetNodeColour(IntPtr pModel, ref udVoxelID pVoxelID, ref UInt32 pColour);

    public UInt32 GetNodeColour(udVoxelID voxelID)
    {
      UInt32 ret = 0;
      udError error = udPointCloud_GetNodeColour(pModel, ref voxelID, ref ret);
      if (error != udError.udE_Success)
        throw new UDException(error);
      return ret;
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetNodeColour64(IntPtr pModel, ref udVoxelID pVoxelID, ref UInt64 pColour);

    public UInt64 GetNodeColour64(udVoxelID voxelID)
    {
      UInt64 ret = 0;
      udError error = udPointCloud_GetNodeColour64(pModel, ref voxelID, ref ret);
      if (error != udError.udE_Success)
        throw new UDException(error);
      return ret;
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetAttributeAddress(IntPtr pModel, ref udVoxelID pVoxelID, UInt32 attributeOffset, ref IntPtr ppAttributeAddress);

    public IntPtr GetAttributeAddress(udVoxelID voxelID, UInt32 attributeOffset)
    {
      IntPtr ret = IntPtr.Zero;
      udError error = udPointCloud_GetAttributeAddress(pModel, ref voxelID, attributeOffset, ref ret);
      if (error != udError.udE_Success)
        throw new UDException(error);
      return ret;
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetStreamingStatus(IntPtr pModel);

    public udError StreamingStatus
    {
      get { return udPointCloud_GetStreamingStatus(pModel); }
    }

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetSourceAttributes(IntPtr pModel, IntPtr pAttributeSet);

    public Attributes.AttributeSet SourceAttributes
    {
      get
      {
        Attributes.AttributeSet ret = new Attributes.AttributeSet(Attributes.StandardAttributeContent.udSAC_None, 0);
        udError error = udPointCloud_GetSourceAttributes(pModel, ret.pAttributeSet);
        if (error != udError.udE_Success)
          throw new UDException(error);
        return ret;
      }
    }
  }

  public class udContext
  {
    ~udContext()
    {
      if (pContextPartial != IntPtr.Zero)
        ConnectCancel();

      if (pContext != IntPtr.Zero)
        Disconnect();
    }

    public void TryResume(string pURL, string pApplicationName, string email=null, bool useDongle = false)
    {
      udError error = udContext_TryResume(ref pContext, pURL, pApplicationName, email, System.Convert.ToInt32(useDongle)); //Set to 1 to try use the dongle
      if (error != udError.udE_Success)
        throw new UDException(error);
      
    }

    public void ConnectWithKey(string pURL, string pKey, string pApplicationName, string appVersion, bool useDongle = false)
    {
      udError error = udError.udE_Failure;

      error = udContext_TryResume(ref pContext, pURL, pApplicationName, null, System.Convert.ToInt32(useDongle)); //Set to 1 to try use the dongle

      if (error != udError.udE_Success)
        error = udContext_ConnectWithKey(ref pContext, pURL, pApplicationName, appVersion, pKey);

      if (error != udError.udE_Success)
        throw new UDException(error);

    }

    public void ConnectStart(string pURL, string pApplicationName, string pApplicationVersion, ref string ppApprovePath, ref string ppApproveCode)
    {
      udError error = udError.udE_Failure;


      if (error != udError.udE_Success)
      {
        IntPtr approvePath = IntPtr.Zero;
        IntPtr approveCode = IntPtr.Zero;
        error = udContext_ConnectStart(ref pContextPartial, pURL, pApplicationName, pApplicationVersion, ref approvePath, ref approveCode);
        ppApprovePath = Marshal.PtrToStringUTF8(approvePath);
        ppApproveCode = Marshal.PtrToStringUTF8(approveCode);
      }

      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    public void ConnectComplete()
    {
      udError error = udError.udE_Failure;

      error = udContext_ConnectComplete(ref pContext, ref pContextPartial);

      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    public void ConnectCancel()
    {
      udError error = udContext_ConnectCancel(ref pContextPartial);
      if (error != udError.udE_Success)
        throw new UDException(error);
    }

    public void Disconnect(bool endSession=false)
    {
      udError error = udContext_Disconnect(ref pContext, System.Convert.ToInt32(endSession));
      // ignore outstanding references as the garbage collector will destroy objects inits own time
      if (error != udError.udE_Success && error != udError.udE_OutstandingReferences)
        throw new UDException(error);
    }

    public IntPtr pContext = IntPtr.Zero;
    public IntPtr pContextPartial = IntPtr.Zero;

    [DllImport("udSDK")]
    private static extern udError udContext_TryResume(ref IntPtr ppContext, string pURL, string pApplicationName, string pEmail, int tryDongle);
    [DllImport("udSDK")]
    private static extern udError udContext_ConnectWithKey(ref IntPtr ppContext, string pURL, string pApplicationName, string pVersion, string pKey);
    [DllImport("udSDK")]
    private static extern udError udContext_ConnectStart(ref IntPtr ppPartialContext, string pURL, string pApplicationName, string pApplicationVersion, ref IntPtr ppApprovePath, ref IntPtr ppApproveCode);
    [DllImport("udSDK")]
    private static extern udError udContext_ConnectComplete(ref IntPtr ppContext, ref IntPtr ppPartialContext);
    [DllImport("udSDK")]
    private static extern udError udContext_ConnectCancel(ref IntPtr ppContext);
    [DllImport("udSDK")]
    private static extern udError udContext_Disconnect(ref IntPtr ppContext, int endSession);
  }

  public class udServerAPI
  {
    public static string Query(udContext context, string API, string JSON)
    {
      IntPtr response = IntPtr.Zero;
      udError error = udServerAPI_Query(context.pContext, API, JSON, ref response);
      if (error != udError.udE_Success)
        throw new UDException(error);

      string responseStr = Marshal.PtrToStringUTF8(response);
      udServerAPI_ReleaseResult(ref response);
      return responseStr;
    }

    [DllImport("udSDK")]
    private static extern udError udServerAPI_Query(IntPtr pContext, [MarshalAs(UnmanagedType.LPUTF8Str)] string pAPIAddress, [MarshalAs(UnmanagedType.LPUTF8Str)] string pJSON, ref IntPtr ppResult);

    [DllImport("udSDK")]
    private static extern udError udServerAPI_ReleaseResult(ref IntPtr ppResult);
  }
}

