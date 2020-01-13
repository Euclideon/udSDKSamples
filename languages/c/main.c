// A Vault Client Hello World! program in C.

#include "vdkContext.h"
#include "vdkRenderContext.h"
#include "vdkRenderView.h"
#include "vdkPointCloud.h"
#include "vdkError.h"

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
  char *pServerPath = "https://earth.vault.euclideon.com";
  char *pUsername = "Username";
  char *pPassword = "Password";
  char *pModelName = "DirCube.uds";
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
  }

  const double cameraMatrix[] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    5,-75,5,1
  };

  enum vdkError error;
  struct vdkContext *pContext = NULL;
  struct vdkRenderContext *pRenderer = NULL;
  struct vdkRenderView *pRenderView = NULL;
  struct vdkRenderInstance instance;
  struct vdkRenderOptions options;
  struct vdkPointCloud *pModel = NULL;
  struct vdkPointCloudHeader header;
  int *pColorBuffer = (int*)malloc(sizeof(int) * width * height);
  float *pDepthBuffer = (float*)malloc(sizeof(float) * width * height);

  memset(&instance, 0, sizeof(instance));
  memset(&header, 0, sizeof(header));
  memset(&options, 0, sizeof(options));

  error = vdkContext_Connect(&pContext, pServerPath, "CClient", pUsername, pPassword);
  if (error != vE_Success)
    goto epilogue;

  // This is only required because we're doing a single render.
  // If we were trying to render a viewport over and over the render call does this internally async
  error = vdkContext_RequestLicense(pContext, vdkLT_Render);
  if (error != vE_Success)
    goto epilogue;

  error = vdkRenderContext_Create(pContext, &pRenderer);
  if (error != vE_Success)
    goto epilogue;

  error = vdkRenderView_Create(pContext, &pRenderView, pRenderer, width, height);
  if (error != vE_Success)
    goto epilogue;

  error = vdkPointCloud_Load(pContext, &pModel, pModelName, &header);
  if (error != vE_Success)
    goto epilogue;

  instance.pPointCloud = pModel;
  memcpy(instance.matrix, header.storedMatrix, sizeof(header.storedMatrix));

  error = vdkRenderView_SetTargets(pRenderView, pColorBuffer, 0, pDepthBuffer);
  if (error != vE_Success)
    goto epilogue;

  error = vdkRenderView_SetMatrix(pRenderView, vdkRVM_Camera, cameraMatrix);
  if (error != vE_Success)
    goto epilogue;

  options.flags = vdkRF_BlockingStreaming; // This is required to do the full streaming within 1 frame rather than progressively refining over multiple frames

  error = vdkRenderContext_Render(pRenderer, pRenderView, &instance, 1, &options);
  if (error != vE_Success)
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
  vdkPointCloud_Unload(&pModel);
  vdkRenderView_Destroy(&pRenderView);
  vdkRenderContext_Destroy(&pRenderer);
  vdkContext_Disconnect(&pContext);

  return error;
}
