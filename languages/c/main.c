// A Vault Client Hello World! program in C.

#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"
#include "udPointCloud.h"
#include "udError.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb_image_write.h"

#ifdef __MACH__
# include <malloc/malloc.h>
#else
# include <malloc.h>
#endif

#include <stdio.h>
#include <string.h>

int main(int argc, char **ppArgv)
{
  int x, y;
  const int width = 1280;
  const int height = 720;
  int legacyServer = 0;
  char *pServerPath = legacyServer ? "https://udstream.euclideon.com" : "https://udcloud.euclideon.com";
  char *pUsername = "Username";
  char *pPassword = "Password";
  char *pModelName = "https://models.euclideon.com/DirCube.uds";
  int pause = 0;

  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(ppArgv[i], "-u") == 0 && i + 1 < argc)
      pUsername = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-p") == 0 && i + 1 < argc)
      pPassword = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-s") == 0 && i + 1 < argc)
      pServerPath = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-m") == 0 && i + 1 < argc)
      pModelName = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-pause") == 0)
      pause = 1;
    else if (strcmp(ppArgv[i], "-legacyServer") == 0)
      legacyServer = 1;
  }

  const double cameraMatrix[] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    5,-5,5,1
  };

  enum udError error;
  struct udContext *pContext = NULL;
  struct udRenderContext *pRenderer = NULL;
  struct udRenderTarget *pRenderTarget = NULL;
  struct udRenderInstance instance;
  struct udRenderSettings options;
  struct udPointCloud *pModel = NULL;
  struct udPointCloudHeader header;
  int *pColorBuffer = (int*)malloc(sizeof(int) * width * height);
  float *pDepthBuffer = (float*)malloc(sizeof(float) * width * height);

  memset(&instance, 0, sizeof(instance));
  memset(&header, 0, sizeof(header));
  memset(&options, 0, sizeof(options));

  // udCloud sign in does not require a username to be provided, however resuming a session does require the username
  error = udContext_TryResume(&pContext, pServerPath, "CClient", pUsername, 0);

  if (error != udE_Success && legacyServer)
  { 
    error = udContext_ConnectLegacy(&pContext, pServerPath, "CClient", pUsername, pPassword);
  }
  else if (error != udE_Success)
  {
    // udCloud sign in is completed in browser on the clients machine - alternatively sign in can be completed using the approval code pApproveCode
    struct udContextPartial *pPartialContext = NULL;
    const char *pApprovePath;
    const char *pApproveCode;
    error = udContext_ConnectStart(&pPartialContext, pServerPath, "CClient", "0.1", &pApprovePath, &pApproveCode);
    if (error == udE_Success)
    {
      printf("Visit \"%s\" on this device to log in to udSDK\nPress Enter to Continue...\n", pApprovePath);
      while (getc(stdin) != '\n');
      error = udContext_ConnectComplete(&pContext, &pPartialContext);
    }

    if (error != udE_Success)
    {
      udContext_ConnectCancel(&pPartialContext);
      goto epilogue;
    }
  }

  if (error != udE_Success)
    goto epilogue;

  error = udRenderContext_Create(pContext, &pRenderer);
  if (error != udE_Success)
    goto epilogue;

  error = udRenderTarget_Create(pContext, &pRenderTarget, pRenderer, width, height);
  if (error != udE_Success)
    goto epilogue;

  error = udPointCloud_Load(pContext, &pModel, pModelName, &header);
  if (error != udE_Success)
    goto epilogue;

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));

  error = udRenderTarget_SetTargets(pRenderTarget, pColorBuffer, 0, pDepthBuffer);
  if (error != udE_Success)
    goto epilogue;

  error = udRenderTarget_SetMatrix(pRenderTarget, udRTM_Camera, cameraMatrix);
  if (error != udE_Success)
    goto epilogue;

  options.flags = udRCF_BlockingStreaming; // This is required to do the full streaming within 1 frame rather than progressively refining over multiple frames

  error = udRenderContext_Render(pRenderer, pRenderTarget, &instance, 1, &options);
  if (error != udE_Success)
    goto epilogue;

  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      int color = pColorBuffer[x + y * width];
      int newColor = 0;

      // Flip R and B - STBI expects ABGR (documented as ARGB) and pColorBuffer is ARGB
      newColor = newColor | (((color >> 24) & 0xFF) << 24);
      newColor = newColor | (((color >> 16) & 0xFF) <<  0);
      newColor = newColor | (((color >>  8) & 0xFF) <<  8);
      newColor = newColor | (((color >>  0) & 0xFF) << 16);

      pColorBuffer[x + y * width] = newColor;
    }
  }

  if (stbi_write_png("tmp.png", width, height, 4, pColorBuffer, width * sizeof(int)) == 0)
    goto epilogue;

  if (pause)
  {
    printf("tmp.png written to the build directory.\n");
    printf("Press enter to exit.");
    getchar();
  }

epilogue:
  free(pDepthBuffer);
  free(pColorBuffer);
  udPointCloud_Unload(&pModel);
  udRenderTarget_Destroy(&pRenderTarget);
  udRenderContext_Destroy(&pRenderer);
  udContext_Disconnect(&pContext, 0);

  return error;
}
