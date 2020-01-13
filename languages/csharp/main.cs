// A Vault Client Hello World! program in C#.
using System;
using System.Runtime.InteropServices;
using System.Drawing;

namespace VaultClient
{
  class Program
  {
    static void Main(string[] args)
    {
      const int width = 1280;
      const int height = 720;

      Vault.vdkContext context = new Vault.vdkContext();
      Vault.vdkRenderContext renderer = new Vault.vdkRenderContext();
      Vault.vdkRenderView renderView = new Vault.vdkRenderView();
      Vault.vdkPointCloud udModel = new Vault.vdkPointCloud();
      Vault.vdkPointCloudHeader header = new Vault.vdkPointCloudHeader();
      uint[] colorBuffer = new uint[width * height];
      float[] depthBuffer = new float[width * height];

      string server = "https://earth.vault.euclideon.com";
      string username = "Username";
      string password = "Password";
      string modelName = "DirCube.uds";
      bool pause = false;

      for (int i = 0; i < args.Length; ++i)
      {
        if (args[i] == "-u" && i + 1 < args.Length)
          username = args[++i];
        else if (args[i] == "-p" && i + 1 < args.Length)
          password = args[++i];
        else if (args[i] == "-s" && i + 1 < args.Length)
          server = args[++i];
        else if (args[i] == "-m" && i + 1 < args.Length)
          modelName = args[++i];
        else if (args[i] == "-pause")
          pause = true;
      }

      try
      {
        context.Connect(server, "C# Sample", username, password);
        context.RequestLicense(Vault.LicenseType.Render);

        Vault.vdkLicenseInfo info = new Vault.vdkLicenseInfo();
        context.GetLicenseInfo(Vault.LicenseType.Render, ref info);

        if (info.queuePosition == -1)
        {
          UInt64 unixTimestamp = (UInt64)(DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1))).TotalSeconds;
          Console.WriteLine("License fetched and available for another {0} seconds.", info.expiresTimestamp - unixTimestamp);
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

        renderView.SetMatrix(Vault.RenderViewMatrix.Camera, cameraMatrix);

        Vault.vdkRenderInstance item = new Vault.vdkRenderInstance();
        item.pointCloud = udModel.pModel;
        item.worldMatrix = header.storedMatrix;

        Vault.vdkRenderInstance itemFlipped = new Vault.vdkRenderInstance();
        itemFlipped.pointCloud = udModel.pModel;
        itemFlipped.worldMatrix = header.storedMatrix;
        itemFlipped.worldMatrix[0] = -itemFlipped.worldMatrix[0];
        itemFlipped.worldMatrix[5] = -itemFlipped.worldMatrix[5];
        itemFlipped.worldMatrix[10] = -itemFlipped.worldMatrix[10];

        Vault.vdkRenderInstance[] modelArray = new Vault.vdkRenderInstance[]{ item, itemFlipped };

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

    static void Convert(string inputPath, string outputPath, Vault.vdkContext context)
    {
      Vault.vdkLicenseInfo info = new Vault.vdkLicenseInfo();
      context.GetLicenseInfo(Vault.LicenseType.Convert, ref info);

      if (info.queuePosition == -1)
        context.RequestLicense(Vault.LicenseType.Convert);

      Vault.vdkConvertContext convertContext = new Vault.vdkConvertContext();
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
