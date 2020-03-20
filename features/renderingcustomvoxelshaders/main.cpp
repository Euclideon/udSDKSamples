#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "VaultSDKFeatureSamples.h"
#include "vdkContext.h"
#include "vdkRenderContext.h"
#include "vdkRenderView.h"
#include "vdkPointCloud.h"
#include "vdkError.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const char s_SampleName[] = "SampleCustomConvert";

const int Width = 1280;
const int Height = 720;

struct CustomVoxelShaderData
{
  uint32_t attributeOffset;
};

uint32_t CustomVoxelShader(struct vdkPointCloud *pPointCloud, uint64_t voxelID, const void *pVoxelUserData)
{
  CustomVoxelShaderData *pData = (CustomVoxelShaderData*)pVoxelUserData;
  const uint16_t *pIntensity;

  vdkPointCloud_GetAttributeAddress(pPointCloud, voxelID, pData->attributeOffset, (const void**)&pIntensity);

  uint32_t intensityValue = (*pIntensity >> 8);

  return 0xFF000000 | intensityValue;
}

int main(int argc, char **ppArgv)
{
  // This confirms that the statics have been configured correctly
  static_assert(s_VaultUsername[0] != '\0', "Username needs to be configured in VaultSDKFeatureSamples.h");
  static_assert(s_VaultPassword[0] != '\0', "Password needs to be configured in VaultSDKFeatureSamples.h");

  // Define our variables
  const double cameraMatrix[] = {
    +1.0,+0.0,+0.0,0,
    +0.0,+0.5,-0.5,0,
    +0.0,+0.5,+0.5,0,

    +50.0,-55.0,+55.0,1
  };

  vdkError vdkResult = vE_Success;
  vdkContext *pContext = nullptr;
  vdkRenderContext *pRenderer = nullptr;
  vdkRenderView *pRenderView = nullptr;
  vdkRenderInstance instance = {};
  vdkRenderOptions options = {};
  vdkPointCloud *pModel = nullptr;
  vdkPointCloudHeader header;

  CustomVoxelShaderData vsData = {};

  int *pColorBuffer = new int[Width * Height];
  float *pDepthBuffer = new float[Width * Height];

  // Resume Session or Login
  if (vdkContext_TryResume(&pContext, s_VaultServer, s_SampleName, s_VaultUsername, false) != vE_Success)
    vdkResult = vdkContext_Connect(&pContext, s_VaultServer, s_SampleName, s_VaultUsername, s_VaultPassword);

  if (vdkResult != vE_Success)
    ExitWithMessage(vdkResult, "Could not login!");

  // This is only required because we're doing a single render.
  // If we were trying to render a viewport over and over the render call does this internally async
  if (vdkContext_RequestLicense(pContext, vdkLT_Render) != vE_Success)
    ExitWithMessage(1, "Could not get license");

  if (vdkRenderContext_Create(pContext, &pRenderer) != vE_Success)
    ExitWithMessage(2, "Could not create render context");

  if (vdkRenderView_Create(pContext, &pRenderView, pRenderer, Width, Height) != vE_Success)
    ExitWithMessage(3, "Could not create render view");

  if (vdkRenderView_SetTargets(pRenderView, pColorBuffer, 0, pDepthBuffer) != vE_Success)
    ExitWithMessage(4, "Could not create render view");

  if (vdkRenderView_SetMatrix(pRenderView, vdkRVM_Camera, cameraMatrix) != vE_Success)
    ExitWithMessage(5, "Could not create render view");

  if (vdkPointCloud_Load(pContext, &pModel, "../../../samplefiles/HistogramTest.uds", &header) != vE_Success)
    ExitWithMessage(6, "Could not create render view");

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));
  instance.pVoxelShader = CustomVoxelShader;
  instance.pVoxelUserData = &vsData;

  options.flags = vdkRF_BlockingStreaming; // This is required to do the full streaming within 1 frame rather than progressively refining over multiple frames

  if (vdkPointCloud_GetHeader(pModel, &header) != vE_Success)
    ExitWithMessage(7, "Rendering failed!");

  if (vdkAttributeSet_GetOffsetOfNamedAttribute(&header.attributes, "udIntensity", &vsData.attributeOffset) != vE_Success)
    ExitWithMessage(8, "Attribute note found!");
  
  if (vdkRenderContext_Render(pRenderer, pRenderView, &instance, 1, &options) != vE_Success)
    ExitWithMessage(9, "Rendering failed!");

  // pColorBuffer now has the contents of the intensity channel from the voxel shader

  // Because stbi write RGBA and VaultSDK outputs BGRA we need to flip
  {
    for (int i = 0; i < Width * Height; ++i)
      pColorBuffer[i] = (pColorBuffer[i] & 0xFF00FF00) | ((pColorBuffer[i] & 0xFF0000) >> 16) | ((pColorBuffer[i] & 0xFF) << 16);

    stbi_write_png("Output.png", Width, Height, 4, pColorBuffer, 0);
  }

  // Clean up
  delete pDepthBuffer;
  delete pColorBuffer;
  vdkPointCloud_Unload(&pModel);
  vdkRenderView_Destroy(&pRenderView);
  vdkRenderContext_Destroy(&pRenderer);
  vdkContext_Disconnect(&pContext);

  return 0;
}
