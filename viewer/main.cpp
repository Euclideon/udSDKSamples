
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"

// external
#include "SDL2/SDL.h"
#include "imgui.h"
//#include "imgui_impl_sdl_gl3.h"

// udcore
#include "udChunkedArray.h"
#include "udMath.h"

// udsdk
#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"

int g_windowWidth = 1280;
int g_windowHeight = 720;

int main(int argc, char **args)
{
  // This confirms that the static key have been configured
  static_assert(s_udCloudKey[0] != '\0', "udCloud key needs to be configured in udSDKFeatureSamples.h");

  // Define our variables
  udError udResult = udE_Success;
  udContext *pContext = nullptr;
  udRenderContext *pRenderer = nullptr;
  udRenderTarget *pRenderView = nullptr;
  udRenderInstance instance = {};
  udRenderSettings options = {};
  udPointCloud *pModel = nullptr;
  udPointCloudHeader header;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, "udcloud.euclideon.com", "udViewer", nullptr, false) != udE_Success)
    udResult = udContext_ConnectWithKey(&pContext, "udcloud.euclideon.com", "udViewer", "1.0", s_udCloudKey);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  uint32_t windowFlags = SDL_WINDOW_SHOWN;// | SDL_WINDOW_RESIZABLE;
  bool isRunning = true;
  uint32_t lastRenderTime = udGetTimeMs() - 16;
  double dt = 1000.f / 60.f;
  double moveSpeed = 100;
  int moveScale = -1;
  udDouble4x4 mat = udDouble4x4::identity(), camera;

  int *pColorBuffer = new int[g_windowWidth * g_windowHeight];
  float *pDepthBuffer = new float[g_windowWidth * g_windowHeight];

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    goto epilogue;

  // Stop window from being minimized while fullscreened and focus is lost
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  SDL_Window *pWindow = SDL_CreateWindow("udViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_windowWidth, g_windowHeight, windowFlags);
  if (!pWindow)
    goto epilogue;

  SDL_GLContext pGLContext = SDL_GL_CreateContext(pWindow);
  ImGui::CreateContext();
  //ImGui_ImplSdlGL3_Init(pWindow);

  camera = camera.identity();

  if (udRenderContext_Create(pContext, &pRenderer) != udE_Success)
  {
    printf("Could not create render context\n");
    goto epilogue;
  }

  if (udRenderTarget_Create(pContext, &pRenderView, pRenderer, g_windowWidth, g_windowHeight) != udE_Success)
  {
    printf("Could not create render target\n");
    goto epilogue;
  }

  if (udPointCloud_Load(pContext, &pModel, "../samplefiles/DirCube.uds", &header) != udE_Success)
  {
    printf("Could not load sample UDS file\n");
    goto epilogue;
  }

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));
  //instance.pVoxelShader = CustomVoxelShader;
  //instance.pVoxelUserData = &vsData;

  options.flags = udRCF_None;

  if (udPointCloud_GetHeader(pModel, &header) != udE_Success)
  {
    printf("Could not get uds header\n");
    goto epilogue;
  }

  while (isRunning)
  {
    double yaw = 0, pitch = 0, tx = 0, ty = 0, tz = 0;
    int frameTimeMs = (udGetTimeMs() - lastRenderTime);
    dt = udMin(frameTimeMs / 1000.f, 1 / 60.f); // Clamp dt at 60fps
    lastRenderTime = udGetTimeMs();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      //ImGui_ImplSdlGL3_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        isRunning = false;

      if (event.type == SDL_MOUSEWHEEL)
        moveScale = udClamp(moveScale + event.wheel.y, -100, 100);
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w)
        ty += moveSpeed * dt;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s)
        ty -= moveSpeed * dt;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a)
        tx -= moveSpeed * dt;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d)
        tx += moveSpeed * dt;
    }

    float moveScaler = (moveScale < 0) ? -1.f / moveScale : moveScale + 1;
    udUpdateCamera(camera.a, yaw, pitch, tx * moveScaler, ty * moveScaler, tz * moveScaler);

    if (udRenderTarget_SetTargets(pRenderView, pColorBuffer, 0, pDepthBuffer) != udE_Success)
      printf("Could not set render target buffers\n");
    if (udRenderTarget_SetMatrix(pRenderView, udRTM_Camera, camera.a) != udE_Success)
      printf("Could not set render target matrix\n");

    if (udRenderContext_Render(pRenderer, pRenderView, &instance, 1, &options) != udE_Success)
      printf("Rendering failed!\n");
  }

epilogue:

  // Clean up
  delete pDepthBuffer;
  delete pColorBuffer;
  udPointCloud_Unload(&pModel);
  udRenderTarget_Destroy(&pRenderView);
  udRenderContext_Destroy(&pRenderer);
  udContext_Disconnect(&pContext, true);

  return 0;
}
