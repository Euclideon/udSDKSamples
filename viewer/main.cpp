
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"

// external
#include "SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_sdlrenderer.h"

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

  float menuBarHeight = 0;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, "staging.udcloud.euclideon.com", "udViewer", nullptr, false) != udE_Success)
    udResult = udContext_ConnectWithKey(&pContext, "staging.udcloud.euclideon.com", "udViewer", "1.0", s_udCloudKey);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;// | SDL_WINDOW_RESIZABLE;
  bool isRunning = true;
  uint32_t lastRenderTime = udGetTimeMs() - 16;
  double dt = 1000.f / 60.f;
  double moveSpeed = 100;
  int moveScale = -1;
  udDouble4x4 mat = udDouble4x4::identity(), camera;

  int *pColorBuffer = new int[g_windowWidth * g_windowHeight];
  float *pDepthBuffer = new float[g_windowWidth * g_windowHeight];
  int selectedItem = -1;

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    goto epilogue;

  // Stop window from being minimized while fullscreened and focus is lost
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  SDL_Window *pWindow = SDL_CreateWindow("udViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_windowWidth, g_windowHeight, windowFlags);
  if (!pWindow)
    goto epilogue;

  uint32_t render_flags = SDL_RENDERER_ACCELERATED;
  SDL_Renderer *pSdlRenderer = SDL_CreateRenderer(pWindow, -1, render_flags);
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(pWindow, pSdlRenderer);
  ImGui_ImplSDLRenderer_Init(pSdlRenderer);
  SDL_Texture *pSdlTexture = SDL_CreateTexture(pSdlRenderer,
    SDL_PIXELFORMAT_BGRA8888,
    SDL_TEXTUREACCESS_STREAMING,
    g_windowWidth,
    g_windowHeight);

  //camera = camera.identity();
  camera = {
    +1.0,+0.0,+0.0,0,
    +0.0,+0.5,-0.5,0,
    +0.0,+0.5,+0.5,0,

    +50.0,-55.0,+55.0,1
  };

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

  //udError res = udPointCloud_Load(pContext, &pModel, "../../samplefiles/DirCube.uds", &header);
  udError res = udPointCloud_Load(pContext, &pModel, "../../samplefiles/HistogramTest.uds", &header);
  if (res != udE_Success)
  {
    printf("Could not load sample UDS file\n");
    goto epilogue;
  }

  if (udRenderTarget_SetTargets(pRenderView, pColorBuffer, 0, pDepthBuffer) != udE_Success)
  {
    printf("Could not set render target buffers\n");
    goto epilogue;
  }

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));

  options.flags = udRCF_None;

  while (isRunning)
  {
    double yaw = 0, pitch = 0, tx = 0, ty = 0, tz = 0;
    int frameTimeMs = (udGetTimeMs() - lastRenderTime);
    dt = udMin(frameTimeMs / 1000.f, 1 / 60.f); // Clamp dt at 60fps
    lastRenderTime = udGetTimeMs();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
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
    void *pSdlPixels;
    udUpdateCamera(camera.a, yaw, pitch, tx * moveScaler, ty * moveScaler, tz * moveScaler);
    SDL_LockTexture(pSdlTexture,
      NULL,      // NULL means the *whole texture* here.
      &pSdlPixels,
      &g_windowWidth);

    if (udRenderTarget_SetMatrix(pRenderView, udRTM_Camera, camera.a) != udE_Success)
      printf("Could not set render target matrix\n");

    if (udRenderContext_Render(pRenderer, pRenderView, &instance, 1, &options) != udE_Success)
      printf("Rendering failed!\n");

    //bind pColorBuffer to SDL_window
    memcpy(pSdlPixels, pColorBuffer, g_windowWidth * g_windowHeight);
    SDL_UnlockTexture(pSdlTexture);

    //ImGUI
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    if (ImGui::Begin("Samples"))
    {
      ImGui::LabelText("Samples", "Samples");
      if (ImGui::BeginListBox("Samples"))
      {
        if (ImGui::Selectable("Basic Sample", selectedItem == 0))
          selectedItem = 0;

        ImGui::EndListBox();
      }
      ImGui::End();
    }

    ImGui::Render();
    SDL_RenderClear(pSdlRenderer);
    SDL_RenderCopy(pSdlRenderer, pSdlTexture, NULL, NULL);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(pSdlRenderer);
  }

epilogue:
  // Clean up
  //ImGuiSDL::Deinitialize();

  delete[] pDepthBuffer;
  delete[] pColorBuffer;
  udPointCloud_Unload(&pModel);
  udRenderTarget_Destroy(&pRenderView);
  udRenderContext_Destroy(&pRenderer);
  udContext_Disconnect(&pContext, true);

  SDL_DestroyTexture(pSdlTexture);
  SDL_DestroyRenderer(pSdlRenderer);
  SDL_DestroyWindow(pWindow);

  return 0;
}
