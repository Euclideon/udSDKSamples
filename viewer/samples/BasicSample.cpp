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
  result = udPointCloud_Load(info.pContext, &pModel, UDSAMPLE_ASSETDIR "/HistogramTest.uds", &header);
  if (result == udE_Success)
  {
    camera = udDouble4x4::identity();
    udDouble4x4 mat = udDouble4x4::create(header.storedMatrix);
    camera.axis.t = mat.axis.t + ((mat.axis.x + mat.axis.z) * 0.25);
  }
  return result;
}

// ----------------------------------------------------------------------------
// Free any resources and leave the sample in a state to be re-initialised
void BasicSample::Deinit()
{
  udPointCloud_Unload(&pModel);
}

// ----------------------------------------------------------------------------
// Render one frame
udError BasicSample::Render(udSampleRenderInfo &info)
{
  udError result;
  udRenderSettings options = {};
  udRenderInstance instance = {};
  int imgPitch = 0;
  void *pSdlPixels = nullptr;

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));

  UpdateCamera(&camera, info.dt, info.moveSpeed, info.turnSpeed);

  SDL_LockTexture(info.pSDLTexture, NULL, &pSdlPixels, &imgPitch);

  result = udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, camera.a);
  if (result == udR_Success)
    result = udRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options);

  //bind pColorBuffer to SDL_window
  memcpy(pSdlPixels, info.pColorBuffer, imgPitch * info.height);
  SDL_UnlockTexture(info.pSDLTexture);

  return result;
}
