// excludeStreamedAttributes.cpp : This file contains the 'main' function. Program execution begins and ends there.
// example showing how to limit attributes on load of point cloud - this increases streaming speed at the expense of needing to reload the model to view the ignored attributes for models with a large number of channels
#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udPointCloud.h"
#include <string.h>


int main(int argc, char **argv)
{
  udError result = udE_Success;
  BasicParseLogin(argc, argv); // handle log in to udCloud / udServer
  const char *modelLocation = "";

  udPointCloud *pCloudAllAttributes = nullptr;
  udPointCloud *pCloudRGBOnly = nullptr;

  udPointCloudLoadOptions loadOptions = {};
  loadOptions.numberAttributesLimited = 64;
  uint32_t limitedAttributes[64] = {}; // this is copied when passed to udPointCloud_LoadAdv so only needs to exist until that function is called

  // the cloud is initialially loaded with all attributes present - we do not actually use this copy of the cloud for rendering, just to extract the list of attributes from the file
  udPointCloud_Load(g_pContext, &pCloudAllAttributes, modelLocation, nullptr);
  udAttributeSet presentAttributes = {};
  udPointCloud_GetSourceAttributes(pCloudAllAttributes, &presentAttributes);
  for (uint32_t i = 0; i < presentAttributes.count; ++i)
  {
    // we set the attributes corresponding to those we want to keep from the original attribute set to 1, in this case the colour channel. See udAttribute.h for details on attribute system
    limitedAttributes[i] = !strcmp("udRGB", presentAttributes.pDescriptors[i].name); 
  }
  loadOptions.pLimitedAttributes = limitedAttributes;

  // finally we can load the model with only the RGB stream requested:
  udPointCloud_LoadAdv(g_pContext, &pCloudRGBOnly, modelLocation, nullptr, &loadOptions);

  // 

epilogue:
  udPointCloud_Unload(&pCloudAllAttributes);
  udPointCloud_Unload(&pCloudRGBOnly);
  udContext_Disconnect(&g_pContext, 0);

}

