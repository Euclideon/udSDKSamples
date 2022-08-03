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

void BasicSample_Init(void **ppSampleData, udContext *pContext)
{
  BasicSampleData *pData = udAllocType(BasicSampleData, 1, udAF_Zero);

  pData->mat = udDouble4x4::identity();

  pData->camera = {
    +1.0,+0.0,+0.0,0,
    +0.0,+0.5,-0.5,0,
    +0.0,+0.5,+0.5,0,

    +50.0,-55.0,+55.0,1
  };


  //udError res = udPointCloud_Load(pContext, &pModel, "../../samplefiles/DirCube.uds", &header);
  udError res = udPointCloud_Load(pContext, &pData->pModel, "../../samplefiles/HistogramTest.uds", &pData->header);
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

  udUpdateCamera(pData->camera.a, yaw, pitch, position.x, position.y, position.z);

  int imgPitch = 0;
  void *pSdlPixels = nullptr;

  SDL_LockTexture(info.pSDLTexture, NULL, &pSdlPixels, &imgPitch);

  if (udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, pData->camera.a) != udE_Success)
    printf("Could not set render target matrix\n");

  if (udRenderContext_Render(info.pRenderContext, info.pRenderTarget, &instance, 1, &options) != udE_Success)
    printf("Rendering failed!\n");

  //bind pColorBuffer to SDL_window
  memcpy(pSdlPixels, info.pColorBuffer, info.width * info.height);
  SDL_UnlockTexture(info.pSDLTexture);
}
