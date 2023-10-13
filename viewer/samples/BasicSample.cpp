#include "../udSample.h"
#include "udSDKFeatureSamples.h"

#include "udRenderContext.h"
#include "udRenderTarget.h"

#include "udMath.h"
#include "udPlatformUtil.h"

#include "SDL.h"
#include "imgui.h"
#include "SDL_opengl.h"

class BasicSample : public udSample
{
public:
  BasicSample(const char *pSampleName) : udSample(pSampleName), udQuadTexture(GL_INVALID_INDEX), quadPixelCount(0) {}
  udError Init(udSampleRenderInfo &info) override;
  void Deinit() override;
  udError Render(udSampleRenderInfo &info) override;

  udDouble4x4 mat;
  udDouble4x4 camera;
  udPointCloud *pModel;
  udPointCloudHeader header;
  GLuint udQuadTexture;
  uint32_t quadPixelCount;
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
  glDeleteTextures(1, &udQuadTexture);
  udQuadTexture = GL_INVALID_INDEX;
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

  UD_ERROR_CHECK(udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, camera.a));
  if (info.useGpuRenderer)
  {
    UD_ERROR_CHECK(udBlockRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options));
  }
  else
  {
    UD_ERROR_CHECK(udRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options));

    glBindTexture(GL_TEXTURE_2D, udQuadTexture);
    if (quadPixelCount != (info.width * info.height))
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, info.width, info.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, info.pColorBuffer);
    else
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, info.width, info.height, GL_BGRA, GL_UNSIGNED_BYTE, info.pColorBuffer);
    quadPixelCount = (info.width * info.height);

    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, 1);

    glTexCoord2f(1, 0);
    glVertex2f(1, 1);

    glTexCoord2f(1, 1);
    glVertex2f(1, -1);

    glTexCoord2f(0, 1);
    glVertex2f(-1, -1);
    glEnd();

  }

epilogue:
  return result;
}
