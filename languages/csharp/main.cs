// A Vault Client Hello World! program in C#.
using System;
using System.Runtime.InteropServices;
using System.Drawing;

namespace udSDKSample
{
  class Program
  {
    static void Main(string[] args)
    {
      const int width = 1280;
      const int height = 720;

      const string applicationName = "CSSample";
      const string appversion = "1.0";

      udSDK.udContext context = new udSDK.udContext();
      udSDK.udRenderContext renderer = new udSDK.udRenderContext();
      udSDK.udRenderTarget renderView = new udSDK.udRenderTarget();
      udSDK.udPointCloud udModel = new udSDK.udPointCloud();
      udSDK.udPointCloudHeader header = new udSDK.udPointCloudHeader();
      uint[] colorBuffer = new uint[width * height];
      float[] depthBuffer = new float[width * height];

      string server = "https://udcloud.euclideon.com";
      string key = null;
      string modelName = "https://models.euclideon.com/DirCube.uds"; // Can be local or remote
      bool pause = false;

      for (int i = 0; i < args.Length; ++i)
      {
        if (args[i] == "-k" && i + 1 < args.Length)
          key = args[++i];
        else if (args[i] == "-s" && i + 1 < args.Length)
          server = args[++i];
        else if (args[i] == "-m" && i + 1 < args.Length)
          modelName = args[++i];
        else if (args[i] == "-pause")
          pause = true;
      }

      try
      {
        if (key == null)
        {
          context.ConnectInteractive(server, applicationName, appversion);
        }
        else
        {
          context.ConnectWithKey(server, key, applicationName, appversion);
        }

        renderer.Create(context);
        renderView.Create(context, renderer, width, height);
        udModel.Load(context, modelName, ref header);
        renderView.SetTargets(ref colorBuffer, 0, ref depthBuffer);

        double[] cameraMatrix = {
          1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0,-5,0,1
        };

        renderView.SetMatrix(udSDK.RenderViewMatrix.Camera, cameraMatrix);

        udSDK.udRenderInstance item = new udSDK.udRenderInstance();
        item.pointCloud = udModel.pModel;
        item.worldMatrix = header.storedMatrix;

        udSDK.udRenderInstance itemFlipped = new udSDK.udRenderInstance();
        itemFlipped.pointCloud = udModel.pModel;
        itemFlipped.worldMatrix = header.storedMatrix;
        itemFlipped.worldMatrix[0] = -itemFlipped.worldMatrix[0];
        itemFlipped.worldMatrix[5] = -itemFlipped.worldMatrix[5];
        itemFlipped.worldMatrix[10] = -itemFlipped.worldMatrix[10];

        udSDK.udRenderInstance[] modelArray = new udSDK.udRenderInstance[]{ item, itemFlipped };

        for (int i = 0; i < 10; i++)
          renderer.Render(renderView, modelArray, modelArray.Length);

        string imagePath = "tmp.png";
        SaveColorImage(imagePath, width, height, colorBuffer);

        if (pause)
        {
          Console.WriteLine(imagePath + " written to the build directory.");
          Console.WriteLine("Press any key to exit.");
          Console.ReadKey();
        }

        //! Uncomment the following line to test the convert API
        //Convert(modelName, modelName + ".uds", context);
      }
      finally
      {
        udModel.Unload();
        renderView.Destroy();
        renderer.Destroy();
        context.Disconnect();
      }
    }

    static void Convert(string inputPath, string outputPath, udSDK.udContext context)
    {
      udSDK.udConvertContext convertContext = new udSDK.udConvertContext();
      convertContext.Create(context);

      convertContext.AddFile(inputPath);
      convertContext.SetFileName(outputPath);

      convertContext.DoConvert();

      convertContext.Destroy();
    }

    static void SaveColorImage(string path, int width, int height, uint[] colorBufferArr)
    {
      Bitmap bmp = new Bitmap(width, height);

      for (int y = 0; y < height; y++)
      {
        for (int x = 0; x < width; x++)
        {
          byte a = (byte)(colorBufferArr[x + y * width] >> 24);
          byte r = (byte)(colorBufferArr[x + y * width] >> 16);
          byte g = (byte)(colorBufferArr[x + y * width] >> 8);
          byte b = (byte)colorBufferArr[x + y * width];

          Color color = Color.FromArgb(a, r, g, b);
          bmp.SetPixel(x, y, color);
        }
      }

      bmp.Save(path);
    }
  }
}
