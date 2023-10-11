#ifndef SharedSample_h__
#define SharedSample_h__

#include "udMath.h"
#include "udError.h"
union SDL_Event;
#include <vector>

struct udSampleRenderInfo
{
  double dt;
  double moveSpeed, turnSpeed; // Camera control speeds

  int width;
  int height;

  struct udContext *pContext;
  struct udRenderContext *pRenderContext;
  struct udRenderTarget *pRenderTarget;

  int *pColorBuffer;
  float *pDepthBuffer;

  struct SDL_Texture *pSDLTexture;
};


// Base class for samples
class udSample
{
public:
  udSample() { samples.push_back(this); }
  virtual ~udSample() {}

  virtual const char *GetName() const = 0;                          // Return the name of the sample
  virtual udError Init(udSampleRenderInfo &info) = 0;               // Initialise for running the sample
  virtual udError Deinit() = 0;                                     // Tear-down resources to run another sample, or to re-run from the start
  virtual udError Render(udSampleRenderInfo &info) = 0;             // Render one frame
  virtual udError Event(udSampleRenderInfo &, const SDL_Event &);   // Handle SDL events, return udE_Success if handled, or udE_NothingToDo if not handled, all other returns are errors

  // Simple camera inputs handler available to all samples for consistency
  void UpdateCamera(udDouble4x4 *pCamera, double dt, double &moveSpeed, double &turnSpeed);
  
  static std::vector<udSample *> samples;
};

#ifndef UDSAMPLE_ASSETDIR
#define UDSAMPLE_ASSETDIR "../../samplefiles"
#endif

#endif // SharedSample_h__
