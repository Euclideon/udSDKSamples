using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Convert
  {
    public class udConvertContext
    {
      public void Create(udContext context)
      {
        udError error = udConvert_CreateContext(context.pContext, ref pConvertContext);
        udErrorUtils.ThrowOnUnsuccessful(error);
      }

      public void Destroy()
      {
        udError error = udConvert_DestroyContext(ref pConvertContext);
        udErrorUtils.ThrowOnUnsuccessful(error);
      }

      public void AddFile(string fileName)
      {
        udError error = udConvert_AddItem(pConvertContext, fileName);
        udErrorUtils.ThrowOnUnsuccessful(error);
      }
      public void SetFileName(string fileName)
      {
        udError error = udConvert_SetOutputFilename(pConvertContext, fileName);
        udErrorUtils.ThrowOnUnsuccessful(error);
      }

      public void DoConvert()
      {
        udError error = udConvert_DoConvert(pConvertContext);
        udErrorUtils.ThrowOnUnsuccessful(error);
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

