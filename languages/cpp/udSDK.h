#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"
#include "udPointCloud.h"
#include "udConvert.h"
#include "udError.h"

#include <string>
#include <vector>

namespace udSDK
{
  class Context;
  class RenderContext;
  class RenderTarget;
  class PointCloud;
  class Convert;

  class Context
  {
  public:
    Context(std::string URL, std::string applicationName, std::string email, std::string password, bool legacyConnection);
    ~Context();

    udError Disconnect();

  private:
    udContext *m_pContext;

    friend class RenderContext;
    friend class RenderTarget;
    friend class PointCloud;
    friend class Convert;
  };

  class RenderContext
  {
  public:
    RenderContext(Context *pContext);
    ~RenderContext();

    udError Render(RenderTarget *pRenderView, const std::vector<PointCloud*> &models, bool completeRender);

  private:
    udRenderContext *m_pRenderer;
    friend class RenderTarget;
  };

  class RenderTarget
  {
  public:
    RenderTarget(Context *pContext, RenderContext *pRenderer, uint32_t width, uint32_t height);
    ~RenderTarget();

    udError SetTargets(void *pColorBuffer, uint32_t colorClearValue, void *pDepthBuffer);

    udError GetMatrix(udRenderTargetMatrix matrixType, double cameraMatrix[16]) const;
    udError SetMatrix(udRenderTargetMatrix matrixType, const double cameraMatrix[16]);

  private:
    udRenderTarget *m_pRenderView;
    friend class RenderContext;
  };

  class PointCloud
  {
  public:
    PointCloud(Context *pContext, std::string modelLocation);
    ~PointCloud();

    udError GetMetadata(std::string *pJSONMetadata);
    udError GetStoredMatrix(double matrix[16]);
    udError GetOriginalAttributes(udAttributeSet *pSet);

  private:
    udPointCloud *m_pModel;
    udPointCloudHeader m_header;
    friend class RenderContext;
  };

  class Convert
  {
  public:
    Convert(Context *pContext, const udConvertInfo **ppInfo);
    ~Convert();

    udError SetOutputFilename(std::string filename);
    udError SetTempDirectory(std::string folder);
    udError AddItem(std::string filename);

    udError DoConvert();

  private:
    udConvertContext *m_pConvertContext;
  };
}
