#include "udSDK.h"
#include <string.h>

namespace udSDK
{
  Context::Context(std::string URL, std::string applicationName, std::string email, std::string password, bool legacyConnection)
  {
    udError result = udE_Failure;

    result = udContext_TryResume(&m_pContext, URL.c_str(), applicationName.c_str(), email.c_str(), 0); // Set to 1 to try use the dongle (doesn't work in debug)

  
    if (result != udE_Success)
    {
      if (legacyConnection)
      {
        result = udContext_ConnectLegacy(&m_pContext, URL.c_str(), applicationName.c_str(), email.c_str(), password.c_str());
      }
      else
      {
        udContextPartial *pPartialConnection;
        const char *pApprovePath = nullptr;
        result = udContext_ConnectStart(&pPartialConnection, URL.c_str(), applicationName.c_str(), "0.1", &pApprovePath, nullptr);
        if (pApprovePath != nullptr)
        {
          printf("visit \"%s\" on this device to complete connection\nPress Enter to continue...\n", pApprovePath);
          while (getc(stdin) != '\n');
        }

        result = udContext_ConnectComplete(&m_pContext, &pPartialConnection);
        if (pPartialConnection != nullptr)
        {
          udContext_ConnectCancel(&pPartialConnection);
        }
      }
    }
      

    if (result != udE_Success)
      throw "Did not login!";
  }

  // Vault::Context
  Context::~Context()
  {
    udContext_Disconnect(&m_pContext, true);
  }

  // Vault::RenderContext
  RenderContext::RenderContext(Context *pContext)
  {
    udError result = udRenderContext_Create(pContext->m_pContext, &m_pRenderer);

    if (result != udE_Success)
      throw "Was not created";
  }

  RenderContext::~RenderContext()
  {
    udRenderContext_Destroy(&m_pRenderer);
  }

  udError RenderContext::Render(RenderTarget *pRenderView, const std::vector<PointCloud*> &models, bool completeRender)
  {
    udRenderInstance *pRenderInstances = new udRenderInstance[models.size()];
    memset(pRenderInstances, 0, sizeof(udRenderInstance) * models.size());

    for (int i = 0; i < models.size(); ++i)
    {
      pRenderInstances[i].pPointCloud = models[i]->m_pModel;
      memcpy(pRenderInstances[i].matrix, models[i]->m_header.storedMatrix, sizeof(models[i]->m_header.storedMatrix));
    }

    udRenderSettings options = {};

    if (completeRender)
      options.flags = udRCF_BlockingStreaming;

    udError error = udRenderContext_Render(m_pRenderer, pRenderView->m_pRenderView, pRenderInstances, (int)models.size(), &options);

    delete[] pRenderInstances;

    return error;
  }

  // Vault::RenderTarget
  RenderTarget::RenderTarget(Context *pContext, RenderContext *pRenderer, uint32_t width, uint32_t height)
  {
    udError success = udRenderTarget_Create(pContext->m_pContext, &this->m_pRenderView, pRenderer->m_pRenderer, width, height);

    if (success != udE_Success)
      throw "Could not create RenderTarget!";
  }

  RenderTarget::~RenderTarget()
  {
    udRenderTarget_Destroy(&m_pRenderView);
  }

  udError RenderTarget::SetTargets(void *pColorBuffer, uint32_t colorClearValue, void *pDepthBuffer)
  {
    return udRenderTarget_SetTargets(m_pRenderView, pColorBuffer, colorClearValue, pDepthBuffer);
  }

  udError RenderTarget::GetMatrix(udRenderTargetMatrix matrixType, double cameraMatrix[16]) const
  {
    return udRenderTarget_GetMatrix(m_pRenderView, matrixType, cameraMatrix);
  }

  udError RenderTarget::SetMatrix(udRenderTargetMatrix matrixType, const double cameraMatrix[16])
  {
    return udRenderTarget_SetMatrix(m_pRenderView, matrixType, cameraMatrix);
  }

  // Vault::PointCloud
  PointCloud::PointCloud(Context *pContext, std::string modelLocation)
  {
    udError error = udPointCloud_Load(pContext->m_pContext, &m_pModel, modelLocation.c_str(), &m_header);

    if (error != udE_Success)
      throw "Could not load point cloud!";
  }

  PointCloud::~PointCloud()
  {
    udPointCloud_Unload(&m_pModel);
  }

  udError PointCloud::GetMetadata(std::string *pJSONMetadata)
  {
    const char *pMetadata = nullptr;
    udError ret = udPointCloud_GetMetadata(m_pModel, &pMetadata);
    *pJSONMetadata = pMetadata;
    return ret;
  }

  udError PointCloud::GetStoredMatrix(double matrix[16])
  {
    memcpy(matrix, m_header.storedMatrix, sizeof(m_header.storedMatrix));
    return udE_Success;
  }

  Convert::Convert(Context *pContext, const udConvertInfo **ppInfo)
  {
    udError error = udConvert_CreateContext(pContext->m_pContext, &m_pConvertContext);
    if (error != udE_Success)
      throw "Could not create convert context!";

    error = udConvert_GetInfo(m_pConvertContext, ppInfo);
    if (error != udE_Success)
      throw "Could not create convert context info!";
  }

  udError PointCloud::GetOriginalAttributes(udAttributeSet *pSet)
  {
    return udPointCloud_GetSourceAttributes(m_pModel, pSet);
  }

  Convert::~Convert()
  {
    udConvert_DestroyContext(&m_pConvertContext);
  }

  udError Convert::SetOutputFilename(std::string filename)
  {
    return udConvert_SetOutputFilename(m_pConvertContext, filename.c_str());
  }

  udError Convert::SetTempDirectory(std::string folder)
  {
    return udConvert_SetTempDirectory(m_pConvertContext, folder.c_str());
  }

  udError Convert::AddItem(std::string filename)
  {
    return udConvert_AddItem(m_pConvertContext, filename.c_str());
  }

  udError Convert::DoConvert()
  {
    return udConvert_DoConvert(m_pConvertContext);
  }
}
