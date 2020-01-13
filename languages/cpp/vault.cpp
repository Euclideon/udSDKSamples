#include "vault.h"
#include <string.h>

namespace Vault
{
  Context::Context(std::string URL, std::string applicationName, std::string username, std::string password)
  {
    vdkError result = vdkContext_Connect(&m_pContext, URL.c_str(), applicationName.c_str(), username.c_str(), password.c_str());

    if (result != vE_Success)
      throw "Did not login!";
  }

  // Vault::Context
  Context::~Context()
  {
    vdkContext_Disconnect(&m_pContext);
  }

  vdkError Context::RequestLicense(vdkLicenseType licenseType)
  {
    return vdkContext_RequestLicense(m_pContext, licenseType);
  }

  vdkError Context::CheckLicense(vdkLicenseType licenseType)
  {
    return vdkContext_CheckLicense(m_pContext, licenseType);
  }

  // Vault::RenderContext
  RenderContext::RenderContext(Vault::Context *pContext)
  {
    vdkError result = vdkRenderContext_Create(pContext->m_pContext, &m_pRenderer);

    if (result != vE_Success)
      throw "Was not created";
  }

  RenderContext::~RenderContext()
  {
    vdkRenderContext_Destroy(&m_pRenderer);
  }

  vdkError RenderContext::Render(Vault::RenderView *pRenderView, const std::vector<Vault::PointCloud*> &models, bool completeRender)
  {
    vdkRenderInstance *pRenderInstances = new vdkRenderInstance[models.size()];
    memset(pRenderInstances, 0, sizeof(vdkRenderInstance) * models.size());

    for (int i = 0; i < models.size(); ++i)
    {
      pRenderInstances[i].pPointCloud = models[i]->m_pModel;
      memcpy(pRenderInstances[i].matrix, models[i]->m_header.storedMatrix, sizeof(models[i]->m_header.storedMatrix));
    }

    vdkRenderOptions options = {};

    if (completeRender)
      options.flags = vdkRF_BlockingStreaming;

    vdkError error = vdkRenderContext_Render(m_pRenderer, pRenderView->m_pRenderView, pRenderInstances, (int)models.size(), &options);

    delete[] pRenderInstances;

    return error;
  }

  // Vault::RenderView
  RenderView::RenderView(Vault::Context *pContext, Vault::RenderContext *pRenderer, uint32_t width, uint32_t height)
  {
    vdkError success = vdkRenderView_Create(pContext->m_pContext, &this->m_pRenderView, pRenderer->m_pRenderer, width, height);

    if (success != vE_Success)
      throw "Could not create RenderView!";
  }

  RenderView::~RenderView()
  {
    vdkRenderView_Destroy(&m_pRenderView);
  }

  vdkError RenderView::SetTargets(void *pColorBuffer, uint32_t colorClearValue, void *pDepthBuffer)
  {
    return vdkRenderView_SetTargets(m_pRenderView, pColorBuffer, colorClearValue, pDepthBuffer);
  }

  vdkError RenderView::GetMatrix(vdkRenderViewMatrix matrixType, double cameraMatrix[16]) const
  {
    return vdkRenderView_GetMatrix(m_pRenderView, matrixType, cameraMatrix);
  }

  vdkError RenderView::SetMatrix(vdkRenderViewMatrix matrixType, const double cameraMatrix[16])
  {
    return vdkRenderView_SetMatrix(m_pRenderView, matrixType, cameraMatrix);
  }

  // Vault::PointCloud
  PointCloud::PointCloud(Vault::Context *pContext, std::string modelLocation)
  {
    vdkError error = vdkPointCloud_Load(pContext->m_pContext, &m_pModel, modelLocation.c_str(), &m_header);

    if (error != vE_Success)
      throw "Could not load point cloud!";
  }

  PointCloud::~PointCloud()
  {
    vdkPointCloud_Unload(&m_pModel);
  }

  vdkError PointCloud::GetMetadata(std::string *pJSONMetadata)
  {
    const char *pMetadata = nullptr;
    vdkError ret = vdkPointCloud_GetMetadata(m_pModel, &pMetadata);
    *pJSONMetadata = pMetadata;
    return ret;
  }

  vdkError PointCloud::GetStoredMatrix(double matrix[16])
  {
    memcpy(matrix, m_header.storedMatrix, sizeof(m_header.storedMatrix));
    return vE_Success;
  }

  Convert::Convert(Vault::Context *pContext, const vdkConvertInfo **ppInfo)
  {
    vdkError error = vdkConvert_CreateContext(pContext->m_pContext, &m_pConvertContext);
    if (error != vE_Success)
      throw "Could not create convert context!";

    error = vdkConvert_GetInfo(m_pConvertContext, ppInfo);
    if (error != vE_Success)
      throw "Could not create convert context info!";
  }

  Convert::~Convert()
  {
    vdkConvert_DestroyContext(&m_pConvertContext);
  }

  vdkError Convert::SetOutputFilename(std::string filename)
  {
    return vdkConvert_SetOutputFilename(m_pConvertContext, filename.c_str());
  }

  vdkError Convert::SetTempDirectory(std::string folder)
  {
    return vdkConvert_SetTempDirectory(m_pConvertContext, folder.c_str());
  }

  vdkError Convert::AddItem(std::string filename)
  {
    return vdkConvert_AddItem(m_pConvertContext, filename.c_str());
  }

  vdkError Convert::DoConvert()
  {
    return vdkConvert_DoConvert(m_pConvertContext);
  }
}
