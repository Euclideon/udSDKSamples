#ifndef SharedSample_h__
#define SharedSample_h__

#include "udMath.h"
#include "udError.h"
#include "udBlockRenderContext.h"

union SDL_Event;
#include <vector>

struct udSampleRenderInfo
{
  double dt;
  double moveSpeed, turnSpeed; // Camera control speeds

  int width;
  int height;
  bool useGpuRenderer;

  struct udContext *pContext;
  struct udRenderContext *pRenderContext;
  struct udRenderTarget *pRenderTarget;

  int *pColorBuffer;
  float *pDepthBuffer;
};


// Base class for samples
class udSample
{
public:
  udSample(const char *pSampleName);
  virtual ~udSample() {}

  virtual udError Init(udSampleRenderInfo &info) = 0;           // Initialise for running the sample
  virtual void Deinit() = 0;                                    // Tear-down resources to run another sample, or to re-run from the start
  virtual udError Render(udSampleRenderInfo &info) = 0;         // Render one frame, error codes are shown to the user
  virtual bool Event(udSampleRenderInfo &, const SDL_Event &);  // Handle SDL events, return true if handled (implying the framework should ignore)

  // Simple camera inputs handler available to all samples for consistency
  void UpdateCamera(udDouble4x4 *pCamera, double dt, double &moveSpeed, double &turnSpeed);
  
  static udSample *pSamplesHead; // Head of static list of samples
  udSample *pNextSample;
  const char *pName;
};

#ifndef UDSAMPLE_ASSETDIR
#define UDSAMPLE_ASSETDIR "../../samplefiles"
#endif

#endif // SharedSample_h__
