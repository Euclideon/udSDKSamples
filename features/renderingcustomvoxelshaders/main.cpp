#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"
#include "udPointCloud.h"
#include "udError.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const char s_SampleName[] = "SampleCustomConvert";

const int Width = 1280;
const int Height = 720;

struct CustomVoxelShaderData
{
  uint32_t attributeOffset;
};

uint32_t CustomVoxelShader(struct udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pVoxelUserData)
{
  CustomVoxelShaderData *pData = (CustomVoxelShaderData*)pVoxelUserData;
  const uint16_t *pIntensity;

  udPointCloud_GetAttributeAddress(pPointCloud, pVoxelID, pData->attributeOffset, (const void**)&pIntensity);

  uint32_t intensityValue = (*pIntensity >> 8);

  return 0xFF000000 | intensityValue;
}

int main(int argc, char **ppArgv)
{
  // This confirms that the statics have been configured correctly
  static_assert(s_udStreamEmail[0] != '\0', "Email needs to be configured in VaultSDKFeatureSamples.h");
  static_assert(s_udStreamPassword[0] != '\0', "Password needs to be configured in VaultSDKFeatureSamples.h");

  // Define our variables
  const double cameraMatrix[] = {
    +1.0,+0.0,+0.0,0,
    +0.0,+0.5,-0.5,0,
    +0.0,+0.5,+0.5,0,

    +50.0,-55.0,+55.0,1
  };

  udError vdkResult = udE_Success;
  udContext *pContext = nullptr;
  udRenderContext *pRenderer = nullptr;
  udRenderTarget *pRenderView = nullptr;
  udRenderInstance instance = {};
  udRenderSettings options = {};
  udPointCloud *pModel = nullptr;
  udPointCloudHeader header;

  CustomVoxelShaderData vsData = {};

  int *pColorBuffer = new int[Width * Height];
  float *pDepthBuffer = new float[Width * Height];

  // Resume Session or Login
  if (udContext_TryResume(&pContext, s_udStreamServer, s_SampleName, s_udStreamEmail, false) != udE_Success)
    vdkResult = udContext_Connect(&pContext, s_udStreamServer, s_SampleName, s_udStreamEmail, s_udStreamPassword);

  if (vdkResult != udE_Success)
    ExitWithMessage(vdkResult, "Could not login!");

  if (udRenderContext_Create(pContext, &pRenderer) != udE_Success)
    ExitWithMessage(2, "Could not create render context");

  if (udRenderTarget_Create(pContext, &pRenderView, pRenderer, Width, Height) != udE_Success)
    ExitWithMessage(3, "Could not create render target");

  if (udRenderTarget_SetTargets(pRenderView, pColorBuffer, 0, pDepthBuffer) != udE_Success)
    ExitWithMessage(4, "Could not set render target buffers");

  if (udRenderTarget_SetMatrix(pRenderView, udRTM_Camera, cameraMatrix) != udE_Success)
    ExitWithMessage(5, "Could not set render target matrix");

  if (udPointCloud_Load(pContext, &pModel, "../../../samplefiles/HistogramTest.uds", &header) != udE_Success)
    ExitWithMessage(6, "Could not load sample UDS file");

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));
  instance.pVoxelShader = CustomVoxelShader;
  instance.pVoxelUserData = &vsData;

  options.flags = udRCF_BlockingStreaming; // This is required to do the full streaming within 1 frame rather than progressively refining over multiple frames

  if (udPointCloud_GetHeader(pModel, &header) != udE_Success)
    ExitWithMessage(7, "Rendering failed!");

  if (udAttributeSet_GetOffsetOfNamedAttribute(&header.attributes, "udIntensity", &vsData.attributeOffset) != udE_Success)
    ExitWithMessage(8, "Attribute not found!");
  
  if (udRenderContext_Render(pRenderer, pRenderView, &instance, 1, &options) != udE_Success)
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
  udPointCloud_Unload(&pModel);
  udRenderTarget_Destroy(&pRenderView);
  udRenderContext_Destroy(&pRenderer);
  udContext_Disconnect(&pContext, true);

  return 0;
}
