// A Vault Client Hello World! program in C++.

#include "udSDK.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb_image_write.h"

#include <iostream>
#include <string.h>

bool Convert(std::string inputPath, std::string outputPath, udSDK::Context &context)
{
  const udConvertInfo *pConvertInfo = nullptr;
  udSDK::Convert convert(&context, &pConvertInfo);

  convert.SetOutputFilename(outputPath.c_str());
  convert.AddItem(inputPath);

  return (convert.DoConvert() == udE_Success);
}

bool Render(std::string inputPath, udSDK::Context &context)
{
  const int width = 1280;
  const int height = 720;

  const double cameraMatrix[] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,-5,0,1
  };

  int *pColorBuffer = new int[width * height];
  float *pDepthBuffer = new float[width * height];

  std::vector<udSDK::PointCloud*> models;

  udSDK::RenderContext renderer(&context);
  udSDK::RenderTarget renderView(&context, &renderer, width, height);

  udSDK::PointCloud pointcloud(&context, inputPath);

  models.push_back(&pointcloud);

  renderView.SetTargets(pColorBuffer, 0, pDepthBuffer);
  renderView.SetMatrix(udRTM_Camera, cameraMatrix);
  
  udError error = renderer.Render(&renderView, models, true);
  
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int color = pColorBuffer[x + y * width];
      int newColor = 0;

      // Flip R and B - STBI expects ABGR (documented as ARGB) and pColorBuffer is ARGB
      newColor = newColor | (((color >> 24) & 0xFF) << 24);
      newColor = newColor | (((color >> 16) & 0xFF) << 0);
      newColor = newColor | (((color >> 8) & 0xFF) << 8);
      newColor = newColor | (((color >> 0) & 0xFF) << 16);

      pColorBuffer[x + y * width] = newColor;
    }
  }

  if (stbi_write_png("tmp.png", width, height, 4, pColorBuffer, width * sizeof(int)) == 0)
    error = udE_Failure;

  delete[] pDepthBuffer;
  delete[] pColorBuffer;

  return error;
}

int main(int argc, char **ppArgv)
{
  bool legacyConnection = false;
  std::string serverPath = legacyConnection ? "https://udstream.euclideon.com" : "https://udcloud.euclideon.com"; //examples of servers supporting legacy and non legacy login methods
  std::string email = "";
  std::string password = "";
  std::string modelName = "https://models.euclideon.com/DirCube.uds"; //Can be any local or remote file

  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(ppArgv[i], "-u") == 0 && i + 1 < argc) // only required for legacy login
      email = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-p") == 0 && i + 1 < argc) // only required for legacy login
      password = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-s") == 0 && i + 1 < argc)
      serverPath = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-m") == 0 && i + 1 < argc)
      modelName = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-legacyServer") == 0)
      legacyConnection = true;
  }

  udSDK::Context context(serverPath, "C++ Sample", email, password, legacyConnection);

  bool vRender = Render(modelName, context);
  bool vConvert = Convert(modelName, modelName + ".uds", context);

  return ((vRender && vConvert) ? 0 : 1);
}
