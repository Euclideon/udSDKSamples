
using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Render
  {
    public enum RenderViewMatrix
    {
      Camera,     // The local to world-space transform of the camera (View is implicitly set as the inverse)
      View,       // The view-space transform for the model (does not need to be set explicitly)
      Projection, // The projection matrix (default is 60 degree LH)
      Viewport,   // Viewport scaling matrix (default width and height of viewport)

      Count,
    };

    //!
    //! These are the various point modes available in udSDK
    //!
    public enum udRenderContextPointMode
    {
      udRCPM_Rectangles, //!< This is the default, renders the voxels expanded as screen space rectangles
      udRCPM_Cubes, //!< Renders the voxels as cubes
      udRCPM_Points, //!< Renders voxels as a single point (Note: does not accurately reflect the 'size' of voxels)

      udRCPM_Count //!< Total number of point modes. Used internally but can be used as an iterator max when displaying different point modes.
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct udRenderInstance
    {
      public IntPtr pointCloud;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
      public double[] worldMatrix;

      public IntPtr filter;
      public IntPtr voxelShader;
      public IntPtr voxelUserData;
      public double opacity;
      public uint skipRender;
      public uint modelID;

      public udRenderInstance(udPointCloud pointCloud, double[] worldMatrix=null)
      {
        if (worldMatrix == null)
          this.worldMatrix = pointCloud.header.storedMatrix;
        else
          this.worldMatrix = worldMatrix;

        filter = new IntPtr(0);
        voxelShader = new IntPtr(0);
        voxelUserData = new IntPtr(0);
        this.pointCloud = pointCloud.pModel;
        opacity = 1;
        skipRender = 0;
        modelID = 0;
      }
    }

    public class udRenderContext
    {
      public udRenderContext(udContext context)
      {
        udError error = udRenderContext_Create(context.pContext, ref pRenderer);
        if (error != udError.udE_Success)
          throw new UDException(error);

        this.context = context;
      }

      ~udRenderContext()
      {
        udError error = udRenderContext_Destroy(ref pRenderer);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void Render(udRenderTarget renderView, udRenderInstance[] pModels, int modelCount, udRenderSettings? settings = null)
      {

        udRenderSettings defaultSettings = settings ?? new udRenderSettings();

        udError error = udRenderContext_Render(pRenderer, renderView.pRenderTarget, pModels, modelCount, ref defaultSettings.internalStruct);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }

      public IntPtr pRenderer = IntPtr.Zero;
      private udContext context;

      [DllImport("udSDK")]
      private static extern udError udRenderContext_Create(IntPtr pContext, ref IntPtr ppRenderer);
      [DllImport("udSDK")]
      private static extern udError udRenderContext_Destroy(ref IntPtr ppRenderer);
      [DllImport("udSDK")]
      private static extern udError udRenderContext_Render(IntPtr pRenderer, IntPtr pRenderView, udRenderInstance[] pModels, int modelCount, ref udRenderSettings_Internal options);
    }

    public enum udRenderContextFlags
    {
      udRCF_None = 0, //!< Render the points using the default configuration.

      udRCF_PreserveBuffers = 1 << 0, //!< The colour and depth buffers won't be cleared before drawing and existing depth will be respected
      udRCF_ComplexIntersections = 1 << 1, //!< This flag is required in some scenes where there is a very large amount of intersecting point clouds
                                           //!< It will internally batch rendering with the udRCF_PreserveBuffers flag after the first render.
      udRCF_BlockingStreaming = 1 << 2, //!< This forces the streamer to load as much of the pointcloud as required to give an accurate representation in the current view. A small amount of further refinement may still occur.
      udRCF_LogarithmicDepth = 1 << 3, //!< Calculate the depth as a logarithmic distribution.
      udRCF_ManualStreamerUpdate = 1 << 4, //!< The streamer won't be updated internally but a render call without this flag or a manual streamer update will be required
      udRCF_ZeroAlphaSkip = 1 << 5, //!< If the voxel has 0 alpha (upper 8 bits) after the voxel shader then the voxel will not be drawn to the buffers (effectively skipped)

      udRCF_2PixelOpt = 1 << 6, //!< Optimisation that allows the renderer to resolve the last 2 pixels simulataneously, this can result in slight inaccuracies (generally a few pixels) in the final image for a huge performance improvement.
      udRCF_DisableOrthographic = 1 << 7, //!< Disables the renderer entering high-performance orthographic mode
    };

    
    [StructLayout(LayoutKind.Sequential)]
    public struct udPickResult
    {
      // Output variables
      public uint hit; //!< Not 0 if a voxel was hit by this pick
      public uint isHighestLOD; //!< Not 0 if this voxel that was hit is as precise as possible
      public uint modelIndex; //!< Index of the model in the ppModels list
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
      public double[] pointCenter; //!< The center of the hit voxel in world space
      public udVoxelID voxelID; //!< The ID of the voxel that was hit by the pick; this ID is only valid for a very short period of time- Do any additional work using this ID this frame.
    }

    //!
    //! @struct udRenderPicking
    //! Stores both the input and output of the udSDK picking system
    //!
    [StructLayout(LayoutKind.Sequential)]
    public struct udRenderPickingInternal
    {
      // Input variables
      public uint x; //!< Mouse X position in udRenderTarget space
      public uint y; //!< Mouse Y position in udRenderTarget space

      public udPickResult result;
    };

    public struct udRenderPicking
    {
      /*
       * This class updates the pointer pPick when modified - this 'hack' is necessary to maintin a safe context as the pointer pPick cannot be modified directly
       * instead when the input properties x and y are modified we copy the entire struct to pPick. This greatly simplifies the method for setting picks
       */
      udRenderPickingInternal internalStruct;
      public IntPtr pPick;

      public uint x
      {
        get { return internalStruct.x; }
        set
        {
          internalStruct.x = value;
          Marshal.StructureToPtr<udRenderPickingInternal>(internalStruct, pPick, true);
        }
      }

      public uint y
      {
        get { return internalStruct.y; }
        set
        {
          internalStruct.y = value;
          Marshal.StructureToPtr<udRenderPickingInternal>(internalStruct, pPick, true);
        }
      }

      public udPickResult Result
      {
        get { return Marshal.PtrToStructure<udRenderPickingInternal>(pPick).result; }
      }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct udRenderSettings_Internal
    {
      public udRenderContextFlags flags; //!< Optional flags providing information about how to perform this render
      public IntPtr pPick; //!< Optional This provides information about the voxel under the mouse
      public udRenderContextPointMode pointMode; //!< The point mode for this render
      IntPtr pFilter; //!< Optional This filter is applied to all models in the scene

      UInt32 pointCount; //!< Optional (GPU Renderer) A hint to the renderer at the upper limit of voxels that are to be rendered.
      float pointThreshold; //!< Optional (GPU Renderer) A hint of the minimum size (in screen space) of a voxel that the renderer will produce.
    }

    public class udRenderSettings
    {
      /*
       * This structure exists as a proxy that Marshals C# side versions of structs to the C++ side.
       * It also ensures that Marshal.SizeOf() returns the correct size for the C++ side of the struct by separating it into an internal member struct.
       */
      public udRenderSettings_Internal internalStruct;

      public udRenderPicking pick; // every time this is modified it will copy its contents to pPick

      public udRenderSettings()
      {
        internalStruct = new udRenderSettings_Internal();
        pick = new udRenderPicking();
        internalStruct.pPick = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(udRenderPicking)));
        pick.pPick = internalStruct.pPick;
      }

      ~udRenderSettings()
      {
        Marshal.FreeHGlobal(internalStruct.pPick);
      }
      public udRenderContextFlags flags
      {
        get{ return internalStruct.flags; }
        set { internalStruct.flags = value; }
      }
      public udRenderContextPointMode pointMode
      {
        get{ return internalStruct.pointMode; }
        set { internalStruct.pointMode = value; }
      }
    }

    public class udRenderTarget
    {
      public udRenderTarget(udContext context, udRenderContext renderer, UInt32 width, UInt32 height)
      {
        udError error = udRenderTarget_Create(context.pContext, ref pRenderTarget, renderer.pRenderer, width, height);
        if (error != udError.udE_Success)
          throw new UDException(error);

        this.context = context;
      }

      ~udRenderTarget()
      {
        if (colorBufferHandle.IsAllocated)
          colorBufferHandle.Free();
        if (depthBufferHandle.IsAllocated)
          depthBufferHandle.Free();

        udError error = udRenderTarget_Destroy(ref pRenderTarget);
        if (error != udError.udE_Success)
          throw new UDException(error);
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
          throw new UDException(error);
      }

      public void GetMatrix(RenderViewMatrix matrixType, double[] cameraMatrix)
      {
        udError error = udRenderTarget_GetMatrix(pRenderTarget, matrixType, cameraMatrix);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      private void SetMatrix(RenderViewMatrix matrixType, double[] cameraMatrix)
      {
        udError error = udRenderTarget_SetMatrix(pRenderTarget, matrixType, cameraMatrix);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public double[] cameraMatrix
      {
        get
        {
          double[] mat = new double[16];
          GetMatrix(RenderViewMatrix.Camera, mat);
          return mat;
        }
        set
        {
          SetMatrix(RenderViewMatrix.Camera, value);
        }
      }

      public double[] projectionMatrix
      {
        get
        {
          double[] mat = new double[16];
          GetMatrix(RenderViewMatrix.Projection, mat);
          return mat;
        }
        set
        {
          SetMatrix(RenderViewMatrix.Projection, value);
        }
      }

      public double[] viewMatrix
      {
        get
        {
          double[] mat = new double[16];
          GetMatrix(RenderViewMatrix.View, mat);
          return mat;
        }
        set
        {
          SetMatrix(RenderViewMatrix.View, value);
        }
      }

      public void SetTargetsWithPitch(ref UInt32[] colorBuffer, UInt32 clearColor, ref float[] depthBuffer, uint colourPitchBytes, uint depthPitchInBytes)
      {
        if (colorBufferHandle.IsAllocated)
          colorBufferHandle.Free();
        if (depthBufferHandle.IsAllocated)
          depthBufferHandle.Free();

        colorBufferHandle = GCHandle.Alloc(colorBuffer, GCHandleType.Pinned);
        depthBufferHandle = GCHandle.Alloc(depthBuffer, GCHandleType.Pinned);

        udError error = udRenderTarget_SetTargetsWithPitch(pRenderTarget, colorBufferHandle.AddrOfPinnedObject(), clearColor, depthBufferHandle.AddrOfPinnedObject(), colourPitchBytes, depthPitchInBytes);
        if(error != udError.udE_Success)
          throw new UDException(error);
      }

      public void SetLogarithmicDepthPlanes(double nearPlane, double farPlane)
      {
        udRenderTarget_SetLogarithmicDepthPlanes(pRenderTarget, nearPlane, farPlane);
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

      [DllImport("udSDK")]
      private static extern udError udRenderTarget_SetTargetsWithPitch(IntPtr pRenderTarget, IntPtr pColorBuffer, UInt32 colorClearValue, IntPtr pDepthBuffer, UInt32 colorPitchInBytes, UInt32 depthPitchInBytes);

      [DllImport("udSDK")]
      private static extern udError udRenderTarget_SetLogarithmicDepthPlanes(IntPtr pRenderTarget, double nearPlane, double farPlane);
    }
  }
}
