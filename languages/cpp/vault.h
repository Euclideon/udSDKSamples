#include "vdkContext.h"
#include "vdkRenderContext.h"
#include "vdkRenderView.h"
#include "vdkPointCloud.h"
#include "vdkConvert.h"
#include "vdkError.h"

#include <string>
#include <vector>

namespace Vault
{
  class Context;
  class RenderContext;
  class RenderView;
  class PointCloud;
  class Convert;

  class Context
  {
  public:
    Context(std::string URL, std::string applicationName, std::string username, std::string password);
    ~Context();

    vdkError Disconnect();

    vdkError RequestLicense(vdkLicenseType licenseType);
    vdkError CheckLicense(vdkLicenseType licenseType);

  private:
    vdkContext *m_pContext;

    friend class RenderContext;
    friend class RenderView;
    friend class PointCloud;
    friend class Convert;
  };

  class RenderContext
  {
  public:
    RenderContext(Vault::Context *pContext);
    ~RenderContext();

    vdkError Render(Vault::RenderView *pRenderView, const std::vector<Vault::PointCloud*> &models, bool completeRender);

  private:
    vdkRenderContext *m_pRenderer;
    friend class RenderView;
  };

  class RenderView
  {
  public:
    RenderView(Vault::Context *pContext, Vault::RenderContext *pRenderer, uint32_t width, uint32_t height);
    ~RenderView();

    vdkError SetTargets(void *pColorBuffer, uint32_t colorClearValue, void *pDepthBuffer);

    vdkError GetMatrix(vdkRenderViewMatrix matrixType, double cameraMatrix[16]) const;
    vdkError SetMatrix(vdkRenderViewMatrix matrixType, const double cameraMatrix[16]);

  private:
    vdkRenderView *m_pRenderView;
    friend class RenderContext;
  };

  class PointCloud
  {
  public:
    PointCloud(Vault::Context *pContext, std::string modelLocation);
    ~PointCloud();

    vdkError GetMetadata(std::string *pJSONMetadata);
    vdkError GetStoredMatrix(double matrix[16]);

  private:
    vdkPointCloud *m_pModel;
    vdkPointCloudHeader m_header;
    friend class RenderContext;
  };

  class Convert
  {
  public:
    Convert(Vault::Context *pContext, const vdkConvertInfo **ppInfo);
    ~Convert();

    vdkError SetOutputFilename(std::string filename);
    vdkError SetTempDirectory(std::string folder);
    vdkError AddItem(std::string filename);

    vdkError DoConvert();

  private:
    vdkConvertContext *m_pConvertContext;
  };
}
