
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

#include "udSDKFeatureSamples.h"
#include "udSample.h"

// external
#include "SDL.h"
#include "GL/glew.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_sdlrenderer.h"

// udcore
#include "udChunkedArray.h"
#include "udMath.h"
#include "udStringUtil.h"
#include "udPlatformUtil.h"
#include "udJSON.h"
#include "udFile.h"

// udsdk
#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"


// ----------------------------------------------------------------------------
// Set up for licencing and render context
udError udSampleViewer_udSDKSetup(udSampleRenderInfo &renderInfo, const char *pAPIKey)
{
  udError result = udE_Failure;

  // Resume Session or Login
  if (udContext_ConnectWithKey(&renderInfo.pContext, "udcloud.com", "udSDK Sample Viewer", "1.0", pAPIKey) != udE_Success)
  {
    printf("Could not login\n");
    goto epilogue;
  }

  if (udRenderContext_Create(renderInfo.pContext, &renderInfo.pRenderContext) != udE_Success)
  {
    printf("Could not create render context\n");
    goto epilogue;
  }

  if (udRenderTarget_Create(renderInfo.pContext, &renderInfo.pRenderTarget, renderInfo.pRenderContext, renderInfo.width, renderInfo.height) != udE_Success)
  {
    printf("Could not create render target\n");
    goto epilogue;
  }

  if (udRenderTarget_SetTargets(renderInfo.pRenderTarget, renderInfo.pColorBuffer, 0, renderInfo.pDepthBuffer) != udE_Success)
  {
    printf("Could not set render target buffers\n");
    goto epilogue;
  }

  result = udE_Success;

epilogue:
  if (result != udE_Success)
  {
    udRenderTarget_Destroy(&renderInfo.pRenderTarget);
    udRenderContext_Destroy(&renderInfo.pRenderContext);
    udContext_Disconnect(&renderInfo.pContext, true);
  }

  return result;
}

// ----------------------------------------------------------------------------
// Save the settings to reload next time (for convenience)
void udSampleViewer_SaveSettings(const udJSON &settings)
{
  const char *pJSONString = nullptr;

  if (settings.Export(&pJSONString, udJEO_JSON | udJEO_FormatWhiteSpace) == udR_Success)
  {
    // This can fail but as this is a simple viewer we aren't handling it
    udFile_Save("settings.json", pJSONString, udStrlen(pJSONString));
    udFree(pJSONString);
  }
}

// ----------------------------------------------------------------------------
int main(int argc, char **args)
{
  udJSON settings = {};

  // Define our variables
  udSampleRenderInfo renderInfo = {};
  renderInfo.width = 1280;
  renderInfo.height = 720;
  // Defaults for camera move/turn speeds
  renderInfo.moveSpeed = 100;
  renderInfo.turnSpeed = -150;
  udSample *pCurrentSample = nullptr;
  const char *pSampleToInit = nullptr;
  udError errorToReport = udE_Success;
  const char *pErrorContext = "";
  const char *pSettingsData = nullptr;

  if (udFile_Load("settings.json", &pSettingsData) == udR_Success)
  {
    settings.Parse(pSettingsData);
    udFree(pSettingsData);
  }

  uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;// | SDL_WINDOW_RESIZABLE;
  bool isRunning = true;
  uint32_t lastRenderTime = udGetTimeMs() - 16;
  
  renderInfo.pColorBuffer = new int[renderInfo.width * renderInfo.height];
  renderInfo.pDepthBuffer = new float[renderInfo.width * renderInfo.height];

  char apikey[1024] = {};

  SDL_Window *pWindow = nullptr;
  SDL_Renderer *pSdlRenderer = nullptr;

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    goto epilogue;
  glewInit();

  // Stop window from being minimized while fullscreened and focus is lost
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  pWindow = SDL_CreateWindow("udSDK Sample Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderInfo.width, renderInfo.height, windowFlags);
  if (!pWindow)
    goto epilogue;

  uint32_t render_flags = SDL_RENDERER_ACCELERATED;
  pSdlRenderer = SDL_CreateRenderer(pWindow, -1, render_flags);
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(pWindow, pSdlRenderer);
  ImGui_ImplSDLRenderer_Init(pSdlRenderer);
  renderInfo.pSDLTexture = SDL_CreateTexture(pSdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, renderInfo.width, renderInfo.height);

  if (settings.Get("apikey").IsString())
    udSampleViewer_udSDKSetup(renderInfo, settings.Get("apikey").AsString());

  if (argc > 1 && udStrBeginsWithi(args[1], "sample="))
    pSampleToInit = args[1] + 7;

  while (isRunning)
  {
    int frameTimeMs = (udGetTimeMs() - lastRenderTime);
    renderInfo.dt = udMin(frameTimeMs / 1000.f, 1 / 60.f); // Clamp dt at 60fps
    lastRenderTime = udGetTimeMs();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        isRunning = false;

      if (!pCurrentSample || pCurrentSample->Event(renderInfo, event) == false)
      {
        // In here the viewer handles events that the sample has not handled themselves
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
          // Unhandled escape will exit the existing sample, and then exit the application
          if (pCurrentSample)
          {
            pCurrentSample->Deinit();
            pCurrentSample = nullptr;
          }
          else
          {
            isRunning = false;
          }
        }
      }
    }

    // ImGUI
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (renderInfo.pContext == nullptr)
    {
      if (ImGui::Begin("Login"))
      {
        ImGui::InputText("API Key", apikey, udLengthOf(apikey));

        if (ImGui::Button("Login"))
        {
          if (udSampleViewer_udSDKSetup(renderInfo, apikey) == udE_Success)
          {
            udJSON tempStr = {};
            tempStr.SetString(apikey);
            settings.Set(&tempStr, "apikey");
            memset(apikey, 0, sizeof(apikey));
            udSampleViewer_SaveSettings(settings);
          }
        }
      }
      ImGui::End();
    }
    else
    {
      ImGui::SetNextWindowSize(ImVec2(0, 0));
      if (ImGui::BeginMainMenuBar())
      {
        if (ImGui::BeginMenu("Samples"))
        {
          for (udSample *p = udSample::pSamplesHead; p; p = p->pNextSample)
          {
            if (ImGui::Selectable(udTempStr("%s##sample_%p", p->pName, p), p == pCurrentSample))
              pSampleToInit = p->pName;
          }
          
          isRunning = !ImGui::MenuItem("Exit", nullptr, nullptr);

          ImGui::EndMenu();
        }
      }

      if (pSampleToInit)
      {
        if (pCurrentSample)
        {
          pCurrentSample->Deinit();
          pCurrentSample = nullptr;
        }

        for (udSample *p = udSample::pSamplesHead; p; p = p->pNextSample)
        {
          if (udStrEquali(p->pName, pSampleToInit))
          {
            pCurrentSample = p;
            errorToReport = p->Init(renderInfo);
            if (errorToReport != udR_Success)
            {
              pErrorContext = "initialising";
              ImGui::OpenPopup("Error");
            }
            break;
          }
        }
        pSampleToInit = nullptr;
      }

      // Make a model popup to report any errors to the user
      if (ImGui::BeginPopupModal("Error"))
      {
        ImGui::Text("Error %s encountered %s %s", udError_GetErrorString(errorToReport), pErrorContext, pCurrentSample ? pCurrentSample->pName : "");
        if (ImGui::Button("OK"))
        {
          if (pCurrentSample)
          {
            pCurrentSample->Deinit();
            pCurrentSample = nullptr;
          }
          errorToReport = udE_Success;
          pErrorContext = "";
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      else
      {
        // If there's no errors, keep rendering more frames
        if (pCurrentSample)
        {
          errorToReport = pCurrentSample->Render(renderInfo);
          if (errorToReport != udR_Success)
          {
            pErrorContext = "rendering";
            ImGui::OpenPopup("Error");
          }
        }
        else
        {
          int imgPitch = 0;
          void *pSdlPixels = nullptr;

          SDL_LockTexture(renderInfo.pSDLTexture, NULL, &pSdlPixels, &imgPitch);
          memset(pSdlPixels, 0, imgPitch * renderInfo.height);
          SDL_UnlockTexture(renderInfo.pSDLTexture);
        }
      }

      ImGui::EndMainMenuBar();

      SDL_RenderClear(pSdlRenderer);
      SDL_RenderCopy(pSdlRenderer, renderInfo.pSDLTexture, NULL, NULL);
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(pSdlRenderer);
  }

epilogue:
  // Clean up
  delete[] renderInfo.pDepthBuffer;
  delete[] renderInfo.pColorBuffer;
  udRenderTarget_Destroy(&renderInfo.pRenderTarget);
  udRenderContext_Destroy(&renderInfo.pRenderContext);
  udContext_Disconnect(&renderInfo.pContext, true);

  SDL_DestroyTexture(renderInfo.pSDLTexture);
  SDL_DestroyRenderer(pSdlRenderer);
  SDL_DestroyWindow(pWindow);

  return 0;
}
