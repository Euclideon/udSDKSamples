#ifndef SharedSample_h__
#define SharedSample_h__

struct udSampleRenderInfo
{
  double dt;

  int width;
  int height;

  struct udRenderContext *pRenderContext;
  struct udRenderTarget *pRenderTarget;

  int *pColorBuffer;
  float *pDepthBuffer;

  struct SDL_Texture *pSDLTexture;
};

typedef void (udSampleInit)(void **ppSampleData, struct udContext *pContext);
typedef void (udSampleDeinit)(void *pSampleData);
typedef void (udSampleRender)(void *pSampleData, const udSampleRenderInfo &info);

struct udSample
{
  const char *pName;

  udSampleInit *pInit; // Called when the Sample is 'activated'
  udSampleDeinit *pDeinit; // Called when the Sample is 'deactivated'
  udSampleRender *pRender; // Called each frame
};

#define UDSAMPLE_PREDECLARE_SAMPLE(x) \
  void x##_Init(void **ppSampleData, udContext *pContext); \
  void x##_Deinit(void *pSampleData); \
  void x##_Render(void *pSampleData, const udSampleRenderInfo &info); \

#define UDSAMPLE_REGISTER_SAMPLE(x) \
  { #x, x##_Init, x##_Deinit, x##_Render }

#ifndef UDSAMPLE_ASSETDIR
#define UDSAMPLE_ASSETDIR "../../samplefiles"
#endif

#endif // SharedSample_h__
