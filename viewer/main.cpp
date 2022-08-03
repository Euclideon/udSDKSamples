
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"
#include "udSample.h"

// external
#include "SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_sdlrenderer.h"

// udcore
#include "udChunkedArray.h"
#include "udMath.h"
#include "udStringUtil.h"
#include "udPlatformUtil.h"

// udsdk
#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"

// samples (commenting out the includes removes them from the list)
UDSAMPLE_PREDECLARE_SAMPLE(BasicSample);

udSample samples[] = {
  UDSAMPLE_REGISTER_SAMPLE(BasicSample),
};

int main(int argc, char **args)
{
  // This confirms that the static key have been configured
  static_assert(s_udCloudKey[0] != '\0', "udCloud key needs to be configured in udSDKFeatureSamples.h");

  // Define our variables
  udError udResult = udE_Success;
  udContext *pContext = nullptr;

  udSampleRenderInfo renderInfo = {};
  renderInfo.width = 1280;
  renderInfo.height = 720;

  float menuBarHeight = 0;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, "udcloud.com", "udSDK Sample Viewer", nullptr, false) != udE_Success)
    udResult = udContext_ConnectWithKey(&pContext, "udcloud.com", "udSDK Sample Viewer", "1.0", s_udCloudKey);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;// | SDL_WINDOW_RESIZABLE;
  bool isRunning = true;
  uint32_t lastRenderTime = udGetTimeMs() - 16;
  
  renderInfo.pColorBuffer = new int[renderInfo.width * renderInfo.height];
  renderInfo.pDepthBuffer = new float[renderInfo.width * renderInfo.height];
  int selectedItem = -1;
  void *pSampleData = nullptr;

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    goto epilogue;

  // Stop window from being minimized while fullscreened and focus is lost
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  SDL_Window *pWindow = SDL_CreateWindow("udSDK Sample Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderInfo.width, renderInfo.height, windowFlags);
  if (!pWindow)
    goto epilogue;

  uint32_t render_flags = SDL_RENDERER_ACCELERATED;
  SDL_Renderer *pSdlRenderer = SDL_CreateRenderer(pWindow, -1, render_flags);
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(pWindow, pSdlRenderer);
  ImGui_ImplSDLRenderer_Init(pSdlRenderer);
  renderInfo.pSDLTexture = SDL_CreateTexture(pSdlRenderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, renderInfo.width, renderInfo.height);

  //camera = camera.identity();
  if (udRenderContext_Create(pContext, &renderInfo.pRenderContext) != udE_Success)
  {
    printf("Could not create render context\n");
    goto epilogue;
  }

  if (udRenderTarget_Create(pContext, &renderInfo.pRenderTarget, renderInfo.pRenderContext, renderInfo.width, renderInfo.height) != udE_Success)
  {
    printf("Could not create render target\n");
    goto epilogue;
  }

  if (udRenderTarget_SetTargets(renderInfo.pRenderTarget, renderInfo.pColorBuffer, 0, renderInfo.pDepthBuffer) != udE_Success)
  {
    printf("Could not set render target buffers\n");
    goto epilogue;
  }

  while (isRunning)
  {
    int frameTimeMs = (udGetTimeMs() - lastRenderTime);
    renderInfo.dt = udMin(frameTimeMs / 1000.f, 1 / 60.f); // Clamp dt at 60fps
    lastRenderTime = udGetTimeMs();

    if (selectedItem >= 0)
      samples[selectedItem].pRender(pSampleData, renderInfo);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        isRunning = false;
    }

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
        for (size_t i = 0; i < udLengthOf(samples); ++i)
        {
          if (ImGui::Selectable(udTempStr("%s##sample_%zu", samples[i].pName, i), selectedItem == i))
          {
            if (selectedItem >= 0)
            {
              samples[selectedItem].pDeinit(pSampleData);
              pSampleData = nullptr;
            }

            selectedItem = (int)i;

            samples[selectedItem].pInit(&pSampleData, pContext);
          }
        }

        ImGui::EndListBox();
      }
      ImGui::End();
    }

    ImGui::Render();
    SDL_RenderClear(pSdlRenderer);
    SDL_RenderCopy(pSdlRenderer, renderInfo.pSDLTexture, NULL, NULL);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(pSdlRenderer);
  }

epilogue:
  // Clean up
  delete[] renderInfo.pDepthBuffer;
  delete[] renderInfo.pColorBuffer;
  udRenderTarget_Destroy(&renderInfo.pRenderTarget);
  udRenderContext_Destroy(&renderInfo.pRenderContext);
  udContext_Disconnect(&pContext, true);

  SDL_DestroyTexture(renderInfo.pSDLTexture);
  SDL_DestroyRenderer(pSdlRenderer);
  SDL_DestroyWindow(pWindow);

  return 0;
}
