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

  // example showing how to limit attributes on load - this increases streaming speed at the expense of needing to reload the model to view the ignored attributes for models with a large number of channels
  // this is an optional step and may be skipped by passing pLoadOptions = nullptr to the pointcloud constructor
  bool restrictToRGBAttribute = true;
  udPointCloudLoadOptions *pLoadOptions = nullptr;
  if (restrictToRGBAttribute)
  {
    udPointCloudLoadOptions loadOptions = {};
    loadOptions.numberAttributesLimited = 64;
    uint32_t limitedAttributes[64] = {}; // this is copied when passed to udPointCloud_LoadAdv so only needs to exist until that function is called

    // the cloud is initialially loaded with all attributes present - we do not actually use this copy of the cloud for rendering, just to extract the list of attributes from the file
    udSDK::PointCloud pointcloudOriginal(&context, inputPath);
    udAttributeSet presentAttributes = {};
    pointcloudOriginal.GetOriginalAttributes(&presentAttributes); //here we retrieve the attributes present
    for (uint32_t i = 0; i < presentAttributes.count; ++i)
    {
      // we set the attributes corresponding to those we want to keep from the original attribute set to 1, in this case the colour channel. See udAttribute.h for details on attribute system
      limitedAttributes[i] = !strcmp("udRGB", presentAttributes.pDescriptors[i].name); 
    }
    loadOptions.pLimitedAttributes = limitedAttributes;
    pLoadOptions = &loadOptions;
  }

  udSDK::PointCloud pointcloud(&context, inputPath, pLoadOptions);

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

      // Flip R and B - STBI expects ARGB and pColorBuffer is ABGR
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
