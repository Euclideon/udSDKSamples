using System;
using System.Runtime.InteropServices;

namespace udSDK
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

  public enum RenderViewMatrix
  {
    Camera,     // The local to world-space transform of the camera (View is implicitly set as the inverse)
    View,       // The view-space transform for the model (does not need to be set explicitly)
    Projection, // The projection matrix (default is 60 degree LH)
    Viewport,   // Viewport scaling matrix (default width and height of viewport)

    Count,
  };


  public enum StandardAttribute
  {
    GPSTime,
    ARGB,
    Normal,
    Intensity,
    NIR,
    ScanAngle,
    PointSourceID,
    Classification,
    ReturnNumber,
    NumberOfReturns,
    ClassificationFlags,
    ScannerChannel,
    ScanDirection,
    EdgeOfFlightLine,
    ScanAngleRank,
    LASUserData,

    Count
  };

  public enum StandardAttributeContent
  {
    None = (0),
    GPSTime = (1 << StandardAttribute.GPSTime),
    ARGB = (1 << StandardAttribute.ARGB),
    Normal = (1 << StandardAttribute.Normal),
    Intensity = (1 << StandardAttribute.Intensity),
    NIR = (1 << StandardAttribute.NIR),
    ScanAngle = (1 << StandardAttribute.ScanAngle),
    PointSourceID = (1 << StandardAttribute.PointSourceID),
    Classification = (1 << StandardAttribute.Classification),
    ReturnNumber = (1 << StandardAttribute.ReturnNumber),
    NumberOfReturns = (1 << StandardAttribute.NumberOfReturns),
    ClassificationFlags = (1 << StandardAttribute.ClassificationFlags),
    ScannerChannel = (1 << StandardAttribute.ScannerChannel),
    ScanDirection = (1 << StandardAttribute.ScanDirection),
    EdgeOfFlightLine = (1 << StandardAttribute.EdgeOfFlightLine),
    ScanAngleRank = (1 << StandardAttribute.ScanAngleRank),
    LasUserData = (1 << StandardAttribute.LASUserData),
  };

  [StructLayout(LayoutKind.Sequential)]
  public struct udRenderInstance
  {
    public IntPtr pointCloud;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst=16)]
    public double[] worldMatrix;

    public IntPtr filter;
    public IntPtr voxelShader;
    public IntPtr voxelUserData;
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct udAttributeSet
  {
    public StandardAttributeContent standardContent; //!< Which standard attributes are available (used to optimize lookups internally), they still appear in the descriptors
    public uint count; //!< How many udAttributeDescriptor objects are used in pDescriptors
    public uint allocated; //!< How many udAttributeDescriptor objects are allocated to be used in pDescriptors
    public IntPtr pDescriptors; //!< this contains the actual information on the attributes
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

    public udAttributeSet attributes;   //!< The attributes contained in this pointcloud

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] baseOffset; //!< The offset to the root of the pointcloud in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] pivot; //!< The pivot point of the model, in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] boundingBoxCenter; //!< The center of the bounding volume, in unit cube space
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public double[] boundingBoxExtents; //!< The extents of the bounding volume, in unit cube space  }
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

    public void Connect(string pURL, string pApplicationName, string pKey)
    {
      udError error = udError.udE_Failure;

      error = udContext_TryResume(ref pContext, pURL, pApplicationName, null, 0); //Set to 1 to try use the dongle

      if (error != udError.udE_Success)
        error = udContext_ConnectWithKey(ref pContext, pURL, pApplicationName, "1.0", pKey);

      if (error == udError.udE_ConnectionFailure)
        throw new Exception("Could not connect to server.");
      else if (error == udError.udE_AuthFailure)
        throw new Exception("Username or Password incorrect.");
      else if (error == udError.udE_OutOfSync)
        throw new Exception("Your clock doesn't match the remote server clock.");
      else if (error == udError.udE_SecurityFailure)
        throw new Exception("Could not open a secure channel to the server.");
      else if (error == udError.udE_ServerFailure)
        throw new Exception("Unable to negotiate with server, please confirm the server address");
      else if (error != udError.udE_Success)
        throw new Exception("Unknown error occurred, please try again later.");
    }

    public void ConnectStart(string pURL, string pApplicationName, string pApplicationVersion, ref string ppApprovePath, ref string ppApproveCode)
    {
      udError error = udError.udE_Failure;

      error = udContext_TryResume(ref pContext, pURL, pApplicationName, null, 0); //Set to 1 to try use the dongle

      if (error != udError.udE_Success)
      {
        IntPtr approvePath = IntPtr.Zero;
        IntPtr approveCode = IntPtr.Zero;
        error = udContext_ConnectStart(ref pContextPartial, pURL, pApplicationName, pApplicationVersion, ref approvePath, ref approveCode);
        ppApprovePath = Marshal.PtrToStringUTF8(approvePath);
        ppApproveCode = Marshal.PtrToStringUTF8(approveCode);
      }

      if (error == udError.udE_ConnectionFailure)
        throw new Exception("Could not connect to server.");
      else if (error == udError.udE_AuthFailure)
        throw new Exception("Username or Password incorrect.");
      else if (error == udError.udE_OutOfSync)
        throw new Exception("Your clock doesn't match the remote server clock.");
      else if (error == udError.udE_SecurityFailure)
        throw new Exception("Could not open a secure channel to the server.");
      else if (error == udError.udE_ServerFailure)
        throw new Exception("Unable to negotiate with server, please confirm the server address");
      else if (error != udError.udE_Success)
        throw new Exception("Unknown error occurred, please try again later.");
    }

    public void ConnectComplete()
    {
      udError error = udError.udE_Failure;

      error = udContext_ConnectComplete(ref pContext, ref pContextPartial);

      if (error == udError.udE_ConnectionFailure)
        throw new Exception("Could not connect to server.");
      else if (error == udError.udE_AuthFailure)
        throw new Exception("Username or Password incorrect.");
      else if (error == udError.udE_OutOfSync)
        throw new Exception("Your clock doesn't match the remote server clock.");
      else if (error == udError.udE_SecurityFailure)
        throw new Exception("Could not open a secure channel to the server.");
      else if (error == udError.udE_ServerFailure)
        throw new Exception("Unable to negotiate with server, please confirm the server address");
      else if (error != udError.udE_Success)
        throw new Exception("Unknown error occurred, please try again later.");
    }

    public void ConnectCancel()
    {
      udError error = udContext_ConnectCancel(ref pContextPartial);
      if (error != udError.udE_Success)
        throw new Exception("udContext.Cancel failed.");
    }

    public void Disconnect()
    {
      udError error = udContext_Disconnect(ref pContext, 0);
      if (error != udError.udE_Success)
        throw new Exception("udContext.Disconnect failed.");
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

  public class udRenderContext
  {
    public void Create(udContext context)
    {
      udError error = udRenderContext_Create(context.pContext, ref pRenderer);
      if (error != udError.udE_Success)
        throw new Exception("udRenderContext.Create failed.");

      this.context = context;
    }

    public void Destroy()
    {
      udError error = udRenderContext_Destroy(ref pRenderer);
      if (error != udError.udE_Success)
        throw new Exception("udRenderContext.Destroy failed.");
    }

    public void Render(udRenderTarget renderView, udRenderInstance[] pModels, int modelCount)
    {
      udError error = udRenderContext_Render(pRenderer, renderView.pRenderTarget, pModels, modelCount, (IntPtr)0);
      if (error != udError.udE_Success)
        throw new Exception("udRenderContext.Render failed.");
    }

    public IntPtr pRenderer = IntPtr.Zero;
    private udContext context;

    [DllImport("udSDK")]
    private static extern udError udRenderContext_Create(IntPtr pContext, ref IntPtr ppRenderer);
    [DllImport("udSDK")]
    private static extern udError udRenderContext_Destroy(ref IntPtr ppRenderer);
    [DllImport("udSDK")]
    private static extern udError udRenderContext_Render(IntPtr pRenderer, IntPtr pRenderView, udRenderInstance[] pModels, int modelCount, IntPtr options);
  }

  public class udRenderTarget
  {
    public void Create(udContext context, udRenderContext renderer, UInt32 width, UInt32 height)
    {
      udError error = udRenderTarget_Create(context.pContext, ref pRenderTarget, renderer.pRenderer, width, height);
      if (error != udError.udE_Success)
        throw new Exception("udRenderTarget.Create failed.");

      this.context = context;
    }

    public void Destroy()
    {
      if (colorBufferHandle.IsAllocated)
        colorBufferHandle.Free();
      if (depthBufferHandle.IsAllocated)
        depthBufferHandle.Free();

      udError error = udRenderTarget_Destroy(ref pRenderTarget);
      if (error != udError.udE_Success)
        throw new Exception("udRenderTarget.Destroy failed.");
    }

    public void SetTargets(ref UInt32[] colorBuffer, UInt32 clearColor, ref float[] depthBuffer)
    {
      if (colorBufferHandle.IsAllocated)
        colorBufferHandle.Free();
      if (depthBufferHandle.IsAllocated)
        depthBufferHandle.Free();

      colorBufferHandle = GCHandle.Alloc(colorBuffer, GCHandleType.Pinned);
      depthBufferHandle = GCHandle.Alloc(depthBuffer, GCHandleType.Pinned);

      udError error = udRenderTarget_SetTargets(pRenderTarget, colorBufferHandle.AddrOfPinnedObject(), clearColor, depthBufferHandle.AddrOfPinnedObject());
      if (error != udError.udE_Success)
        throw new Exception("udRenderTarget.SetTargets failed.");
    }

    public void GetMatrix(RenderViewMatrix matrixType, double[] cameraMatrix)
    {
      udError error = udRenderTarget_GetMatrix(pRenderTarget, matrixType, cameraMatrix);
      if (error != udError.udE_Success)
        throw new Exception("udRenderTarget.GetMatrix failed.");
    }

    public void SetMatrix(RenderViewMatrix matrixType, double[] cameraMatrix)
    {
      udError error = udRenderTarget_SetMatrix(pRenderTarget, matrixType, cameraMatrix);
      if (error != udError.udE_Success)
        throw new Exception("udRenderTarget.SetMatrix failed.");
    }

    public IntPtr pRenderTarget = IntPtr.Zero;
    private udContext context;

    private GCHandle colorBufferHandle;
    private GCHandle depthBufferHandle;

    [DllImport("udSDK")]
    private static extern udError udRenderTarget_Create(IntPtr pContext, ref IntPtr ppRenderView, IntPtr pRenderer, UInt32 width, UInt32 height);

    [DllImport("udSDK")]
    private static extern udError udRenderTarget_Destroy(ref IntPtr ppRenderView);

    [DllImport("udSDK")]
    private static extern udError udRenderTarget_SetTargets(IntPtr pRenderView, IntPtr pColorBuffer, UInt32 colorClearValue, IntPtr pDepthBuffer);

    [DllImport("udSDK")]
    private static extern udError udRenderTarget_GetMatrix(IntPtr pRenderView, RenderViewMatrix matrixType, double[] cameraMatrix);

    [DllImport("udSDK")]
    private static extern udError udRenderTarget_SetMatrix(IntPtr pRenderView, RenderViewMatrix matrixType, double[] cameraMatrix);
  }

  public class udPointCloud
  {
    public void Load(udContext context, string modelLocation, ref udPointCloudHeader header)
    {
      udError error = udPointCloud_Load(context.pContext, ref pModel, modelLocation, ref header);
      if (error != udError.udE_Success)
        throw new Exception("udPointCloud.Load failed.");

      this.context = context;
    }

    public void Unload()
    {
      udError error = udPointCloud_Unload(ref pModel);
      if (error != udError.udE_Success)
        throw new Exception("udPointCloud.Unload failed.");
    }

    public void GetMetadata(ref string ppJSONMetadata)
    {
      udError error = udPointCloud_GetMetadata(pModel, ref ppJSONMetadata);
      if (error != udError.udE_Success)
        throw new Exception("udPointCloud.GetMetadata failed.");
    }

    public IntPtr pModel = IntPtr.Zero;
    private udContext context;

    [DllImport("udSDK")]
    private static extern udError udPointCloud_Load(IntPtr pContext, ref IntPtr ppModel, string modelLocation, ref udPointCloudHeader header);

    [DllImport("udSDK")]
    private static extern udError udPointCloud_Unload(ref IntPtr ppModel);

    [DllImport("udSDK")]
    private static extern udError udPointCloud_GetMetadata(IntPtr pModel, ref string ppJSONMetadata);
  }

  public class udConvertContext
  {
    public void Create(udContext context)
    {
      udError error = udConvert_CreateContext(context.pContext, ref pConvertContext);
      if (error != udError.udE_Success)
        throw new Exception("udConvertContext.Create failed.");
    }

    public void Destroy()
    {
      udError error = udConvert_DestroyContext(ref pConvertContext);
      if (error != udError.udE_Success)
        throw new Exception("udConvertContext.Destroy failed.");
    }


    public void AddFile(string fileName)
    {
      udError error = udConvert_AddItem(pConvertContext, fileName);
      if (error != udError.udE_Success)
        throw new Exception("udConvertContext.AddItem failed.");
    }
    public void SetFileName(string fileName)
    {
      udError error = udConvert_SetOutputFilename(pConvertContext, fileName);
      if (error != udError.udE_Success)
        throw new Exception("udConvertContext.SetOutputFilename failed.");
    }

    public void DoConvert()
    {
      udError error = udConvert_DoConvert(pConvertContext);
      if (error != udError.udE_Success)
        throw new Exception("udConvertContext.DoConvert failed.");
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

  public class udServerAPI
  {
    public static string Query(udContext context, string API, string JSON)
    {
      IntPtr response = IntPtr.Zero;
      udError error = udServerAPI_Query(context.pContext, API, JSON, ref response);
      if (error != udError.udE_Success)
        throw new Exception("udServerAPI.Query failed.");

      string responseStr = Marshal.PtrToStringUTF8(response);
      udServerAPI_ReleaseResult(ref response);
      return responseStr;
    }

    [DllImport("udSDK")]
    private static extern udError udServerAPI_Query(IntPtr pContext, string pAPIAddress, string pJSON, ref IntPtr ppResult);

    [DllImport("udSDK")]
    private static extern udError udServerAPI_ReleaseResult(ref IntPtr ppResult);
  }
}
