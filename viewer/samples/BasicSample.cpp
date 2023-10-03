#include "../udSample.h"
#include "udSDKFeatureSamples.h"

#include "udRenderContext.h"
#include "udRenderTarget.h"

#include "udMath.h"
#include "udPlatformUtil.h"

#include "SDL.h"
#include "imgui.h"

struct BasicSampleData
{
  udDouble4x4 mat;
  udDouble4x4 camera;
  udPointCloud *pModel;
  udPointCloudHeader header;
};

void UpdateCamera(double camera[16], double yawRadians, double pitchRadians, double tx, double ty, double tz)
{
  udDouble4x4 rotation = udDouble4x4::create(camera);
  udDouble3 pos = rotation.axis.t.toVector3();
  rotation.axis.t = udDouble4::identity();

  if (yawRadians != 0.0)
    rotation = udDouble4x4::rotationZ(yawRadians) * rotation;   // Yaw on global axis
  if (pitchRadians != 0.0)
    rotation = rotation * udDouble4x4::rotationX(pitchRadians); // Pitch on local axis
  udDouble3 trans = udDouble3::zero();
  trans += rotation.axis.x.toVector3() * tx;
  trans += rotation.axis.y.toVector3() * ty;
  trans += rotation.axis.z.toVector3() * tz;
  rotation.axis.t = udDouble4::create(pos + trans, 1.0);

  memcpy(camera, rotation.a, sizeof(rotation));
}


void BasicSample_Init(void **ppSampleData, const struct udSampleRenderInfo &info)
{
  BasicSampleData *pData = udAllocType(BasicSampleData, 1, udAF_Zero);

  pData->mat = udDouble4x4::identity();

  pData->camera = {
    +1.0,+0.0,+0.0,0,
    +0.0,+0.5,-0.5,0,
    +0.0,+0.5,+0.5,0,

    +50.0,-55.0,+55.0,1
  };


  //udError res = udPointCloud_Load(info.pContext, &pData->pModel, UDSAMPLE_ASSETDIR "/DirCube.uds", &pData->header);
  udError res = udPointCloud_Load(info.pContext, &pData->pModel, UDSAMPLE_ASSETDIR "/HistogramTest.uds", &pData->header);
  if (res != udE_Success)
  {
    printf("Could not load sample UDS file\n");
  }

  *ppSampleData = pData;
}

void BasicSample_Deinit(void *pSampleData)
{
  BasicSampleData *pData = (BasicSampleData*)pSampleData;

  udPointCloud_Unload(&pData->pModel);
  udFree(pSampleData);
}

void BasicSample_Render(void *pSampleData, const udSampleRenderInfo &info)
{
  const double MoveSpeed = 100;

  BasicSampleData *pData = (BasicSampleData*)pSampleData;

  udRenderSettings options = {};
  udRenderInstance instance = {};

  instance.pPointCloud = pData->pModel;
  memcpy(instance.matrix, pData->header.storedMatrix, sizeof(pData->header.storedMatrix));

  double yaw = 0;
  double pitch = 0;
  udDouble3 position = {};

  if (ImGui::IsKeyDown(ImGuiKey_W))
    position.y += MoveSpeed * info.dt;
  if (ImGui::IsKeyDown(ImGuiKey_S))
    position.y -= MoveSpeed * info.dt;
  if (ImGui::IsKeyDown(ImGuiKey_A))
    position.x -= MoveSpeed * info.dt;
  if (ImGui::IsKeyDown(ImGuiKey_D))
    position.x += MoveSpeed * info.dt;

  if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
  {
    ImVec2 mouseDrag = ImGui::GetMouseDragDelta();
    yaw = mouseDrag.x / 100;
    pitch = mouseDrag.y / 100;
    ImGui::ResetMouseDragDelta();
  }

  UpdateCamera(pData->camera.a, yaw, pitch, position.x, position.y, position.z);

  int imgPitch = 0;
  void *pSdlPixels = nullptr;

  SDL_LockTexture(info.pSDLTexture, NULL, &pSdlPixels, &imgPitch);

  if (udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, pData->camera.a) != udE_Success)
    printf("Could not set render target matrix\n");

  if (udRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options) != udE_Success)
    printf("Rendering failed!\n");

  //bind pColorBuffer to SDL_window
  memcpy(pSdlPixels, info.pColorBuffer, imgPitch * info.height);
  SDL_UnlockTexture(info.pSDLTexture);
}
