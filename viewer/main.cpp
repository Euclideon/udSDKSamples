
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

int g_windowWidth = 1280;
int g_windowHeight = 720;

int main(int argc, char **args)
{
  // This confirms that the static key have been configured
  static_assert(s_udCloudKey[0] != '\0', "udCloud key needs to be configured in udSDKFeatureSamples.h");

  // Define our variables
  udError udResult = udE_Success;
  udContext *pContext = nullptr;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, "udcloud.euclideon.com", "udViewer", nullptr, false) != udE_Success)
    udResult = udContext_ConnectWithKey(&pContext, "udcloud.euclideon.com", "udViewer", "1.0", s_udCloudKey);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    goto epilogue;

  // Stop window from being minimized while fullscreened and focus is lost
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  SDL_Window *pWindow = SDL_CreateWindow("udViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_windowWidth, g_windowHeight, windowFlags);
  if (!pWindow)
    goto epilogue;

  SDL_GLContext pGLContext = SDL_GL_CreateContext(pWindow);
  ImGui::CreateContext();
  //ImGui_ImplSdlGL3_Init(pWindow);


epilogue:
  return 0;
}
