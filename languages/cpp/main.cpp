// A Vault Client Hello World! program in C++.

#include "vault.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb_image_write.h"

#include <iostream>
#include <string.h>

bool Convert(std::string inputPath, std::string outputPath, Vault::Context &context)
{
  const vdkConvertInfo *pConvertInfo = nullptr;
  Vault::Convert convert(&context, &pConvertInfo);

  context.RequestLicense(vdkLT_Convert);
  
  convert.SetOutputFilename(outputPath.c_str());
  convert.AddItem(inputPath);

  return (convert.DoConvert() == vE_Success);
}

bool Render(std::string inputPath, Vault::Context &context)
{
  const int width = 1280;
  const int height = 720;

  const double cameraMatrix[] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    5,-75,5,1
  };

  int *pColorBuffer = new int[width * height];
  float *pDepthBuffer = new float[width * height];

  std::vector<Vault::PointCloud*> models;

  Vault::RenderContext renderer(&context);
  Vault::RenderView renderView(&context, &renderer, width, height);
  Vault::PointCloud pointcloud(&context, inputPath);

  models.push_back(&pointcloud);

  context.RequestLicense(vdkLT_Render);

  renderView.SetTargets(pColorBuffer, 0, pDepthBuffer);
  renderView.SetMatrix(vdkRVM_Camera, cameraMatrix);
  
  vdkError error = renderer.Render(&renderView, models, true);
  
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
    error = vE_Failure;

  delete[] pDepthBuffer;
  delete[] pColorBuffer;

  return error;
}

int main(int argc, char **ppArgv)
{
  std::string serverPath = "https://earth.vault.euclideon.com";
  std::string username = "Username";
  std::string password = "Password";
  std::string modelName = "DirCube.uds";

  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(ppArgv[i], "-u") == 0 && i + 1 < argc)
      username = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-p") == 0 && i + 1 < argc)
      password = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-s") == 0 && i + 1 < argc)
      serverPath = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-m") == 0 && i + 1 < argc)
      modelName = ppArgv[++i];
  }

  Vault::Context context(serverPath, "C++ Sample", username, password);

  bool vRender = Render(modelName, context);
  bool vConvert = Convert(modelName, modelName + ".uds", context);

  return ((vRender && vConvert) ? 0 : 1);
}
