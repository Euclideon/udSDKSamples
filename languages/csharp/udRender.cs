
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

    [StructLayout(LayoutKind.Sequential)]
    public struct udRenderInstance
    {
      public IntPtr pointCloud;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst=16)]
      public double[] worldMatrix;

      public IntPtr filter;
      public IntPtr voxelShader;
      public IntPtr voxelUserData;
      public double opacity;
      public uint skipRender;
    }


    public class udRenderContext
    {
      public void Create(udContext context)
      {
        udError error = udRenderContext_Create(context.pContext, ref pRenderer);
        if (error != udError.udE_Success)
          throw new UDException(error);

        this.context = context;
      }

      public void Destroy()
      {
        udError error = udRenderContext_Destroy(ref pRenderer);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      public void Render(udRenderTarget renderView, udRenderInstance[] pModels, int modelCount)
      {
        udError error = udRenderContext_Render(pRenderer, renderView.pRenderTarget, pModels, modelCount, (IntPtr)0);
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
      private static extern udError udRenderContext_Render(IntPtr pRenderer, IntPtr pRenderView, udRenderInstance[] pModels, int modelCount, IntPtr options);
    }

    public class udRenderTarget
    {
      public void Create(udContext context, udRenderContext renderer, UInt32 width, UInt32 height)
      {
        udError error = udRenderTarget_Create(context.pContext, ref pRenderTarget, renderer.pRenderer, width, height);
        if (error != udError.udE_Success)
          throw new UDException(error);

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

      public void SetMatrix(RenderViewMatrix matrixType, double[] cameraMatrix)
      {
        udError error = udRenderTarget_SetMatrix(pRenderTarget, matrixType, cameraMatrix);
        if (error != udError.udE_Success)
          throw new UDException(error);
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
  }
}
