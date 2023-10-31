#include "../udSample.h"
#include "udSDKFeatureSamples.h"

#include "udRenderContext.h"
#include "udRenderTarget.h"

#include "udMath.h"
#include "udPlatformUtil.h"

#include "SDL.h"
#include "imgui.h"

class BasicSample : public udSample
{
public:
  BasicSample(const char *pSampleName) : udSample(pSampleName) {}
  udError Init(udSampleRenderInfo &info) override;
  void Deinit() override;
  udError Render(udSampleRenderInfo &info) override;

  udDouble4x4 mat;
  udDouble4x4 camera;
  udPointCloud *pModel;
  udPointCloudHeader header;
};
static BasicSample instance("Basic Sample");


// ----------------------------------------------------------------------------
// Initalise the sample, this is where resources should be allocated rather than in the constructor
udError BasicSample::Init(udSampleRenderInfo &info)
{
  udError result;

  mat = udDouble4x4::identity();
  UD_ERROR_CHECK(udPointCloud_Load(info.pContext, &pModel, UDSAMPLE_ASSETDIR "/HistogramTest.uds", &header));

  camera = udDouble4x4::identity();
  mat = udDouble4x4::create(header.storedMatrix);
  camera.axis.t = mat.axis.t + ((mat.axis.x + mat.axis.z) * 0.25);

  glGenTextures(1, &udQuadTexture);
  result = udE_Success;

  epilogue:
  return result;
}

// ----------------------------------------------------------------------------
// Free any resources and leave the sample in a state to be re-initialised
void BasicSample::Deinit()
{
  udPointCloud_Unload(&pModel);
  DeinitUDQuad();
}

// ----------------------------------------------------------------------------
// Render one frame
udError BasicSample::Render(udSampleRenderInfo &info)
{
  udError result;
  udRenderSettings options = {};
  udRenderInstance instance = {};

  options.pointMode = info.pointMode;
  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));

  UpdateCamera(&camera, info.dt, info.moveSpeed, info.turnSpeed);

  UD_ERROR_CHECK(udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, camera.a));
  if (info.useGpuRenderer)
  {
    UD_ERROR_CHECK(udBlockRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options));
  }
  else
  {
    UD_ERROR_CHECK(udRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options));
    UD_ERROR_CHECK(RenderUDQuad(info));
  }

epilogue:
  return result;
}
