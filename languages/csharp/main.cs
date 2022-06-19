// A Vault Client Hello World! program in C#.
using System;
using System.Runtime.InteropServices;
using System.Drawing;
using udSDK = Euclideon.udSDK;

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
      udSDK.PointCloud.udPointCloud udModel;
      udSDK.Render.udRenderContext renderer;
      udSDK.Render.udRenderTarget renderView;

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
          ConnectInteractive(ref context, server, applicationName, appversion);
        }
        else
        {
          context.ConnectWithKey(server, key, applicationName, appversion);
        }

        renderer = new udSDK.Render.udRenderContext(context);
        renderView = new udSDK.Render.udRenderTarget(context, renderer, width, height);

        udModel = new udSDK.PointCloud.udPointCloud(context, modelName);

        renderView.SetTargets(ref colorBuffer, 0, ref depthBuffer);

        double[] cameraMatrix = {
          1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0, 0,5,1
        };

        renderView.cameraMatrix = cameraMatrix;

        udSDK.Render.udRenderInstance item = new udSDK.Render.udRenderInstance(udModel);

        // modifying the transformation of the model and rendering it as a separate instance:
        udSDK.Render.udRenderInstance itemFlipped = new udSDK.Render.udRenderInstance(udModel);
        itemFlipped.worldMatrix[0] = 10 *itemFlipped.worldMatrix[0];
        itemFlipped.worldMatrix[5] = 10 *itemFlipped.worldMatrix[5];
        itemFlipped.worldMatrix[10] = 10 * itemFlipped.worldMatrix[10];


        //udSDK.Render.udRenderInstance[] modelArray = new udSDK.Render.udRenderInstance[]{ item, itemFlipped };
        udSDK.Render.udRenderInstance[] modelArray = new udSDK.Render.udRenderInstance[]{ item};

        udSDK.Render.udRenderSettings renderSettings = new udSDK.Render.udRenderSettings();
        renderSettings.flags = udSDK.Render.udRenderContextFlags.udRCF_BlockingStreaming;

        udSDK.Render.udRenderPicking pick = new udSDK.Render.udRenderPicking();
        pick.x = 2* width / 3;
        pick.y = height / 3;
        renderSettings.pick = pick;


        for (int i = 0; i < 10; i++)
          renderer.Render(renderView, modelArray, modelArray.Length, renderSettings);


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
        context.Disconnect();
      }
    }

    static void Convert(string inputPath, string outputPath, udSDK.udContext context)
    {
      udSDK.Convert.udConvertContext convertContext = new udSDK.Convert.udConvertContext();
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
    static void ConnectInteractive(ref udSDK.udContext udContext, string serverURL, string applicationName, string appversion)
    {
      try
      {
        udContext.TryResume(serverURL, applicationName);
      }
      catch (udSDK.UDException e)
      {
        string approvePath = "";
        string approveCode = "";

        udContext.ConnectStart(serverURL, applicationName, appversion, ref approvePath, ref approveCode);
        Console.WriteLine("Navigate to " + approvePath + " on this device to complete udCloud login");
        Console.WriteLine("Altenatively navigate to " + serverURL + "/link on any device and enter " + approveCode);
        Console.WriteLine("Press any key to continue...");
        Console.ReadKey();

        try
        {
          udContext.ConnectComplete();
        }
        catch (udSDK.UDException fatalError)
        {
          Console.WriteLine("udCloud Login failed: " + fatalError.Message);
          throw e;
        }
      }
    }
  }
}

