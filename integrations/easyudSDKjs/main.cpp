#include "udContext.h"

#include "vHTTPRequest.h"

#include "udSocket.h"
#include "udFile.h"
#include "udPlatformUtil.h"
#include "udStringUtil.h"
#include "udGeoZone.h"
#include "udJSON.h"

#include "vCore.h"

#include "udContext.h"
#include "udRenderContext.h"
#include "udRenderTarget.h"
#include "udError.h"
#include "udPointCloud.h"
#include "udScene.h"
#include "udWorkerPool.h"

#include "udChunkedArray.h"

#if UDPLATFORM_EMSCRIPTEN
# include <emscripten/threading.h>
#endif

enum
{
  MaxRenderInstances = 64,
  InvalidID = -1
};

struct udSDKJSVoxelShaderData
{
  uint32_t attributeOffset;
  float maxIntensity;
  bool isGreyScale;
  bool isHighlighted;
};

struct udSDKJSModelInstance
{
  udRenderInstance *pRenderInstance;
  udSDKJSVoxelShaderData shaderData;
};

class udSDKJSRenderDataMap
{
public:

  udSDKJSRenderDataMap();
  ~udSDKJSRenderDataMap();

  udResult Init(int count);
  udRenderInstance *GetRenderInstanceArray() const { return m_pRenderInstanceArray; }
  uint32_t Size() const { return m_count; }
  udSDKJSModelInstance *Get(int key) const;
  udSDKJSModelInstance *GetFromIndex(int index) const;
  void Erase(int key);
  int InsertNew(udSDKJSModelInstance **ppData);
  void Clear();

private:
  int m_arraySize;
  int m_count;
  udSDKJSModelInstance *m_pModelArray;
  udRenderInstance *m_pRenderInstanceArray;
  udChunkedArray<int> m_availableSlotArray;
};

uint32_t g_highlightColour = (0xFF000000) | (253ul << 16) | (173ul << 8) | (53ul);
float g_highlightStrength = 0.2f;

char g_serverAddress[256] = "https://udcloud.euclideon.com/";
const char g_udCloudAddress[] = "https://udcloud.euclideon.com/";

udContext *g_pContext = nullptr;
udRenderContext *g_pRenderer = nullptr;
udRenderTarget *g_pRenderTarget = nullptr;

udScene *g_pScene = nullptr;

udSDKJSRenderDataMap g_renderData;

bool g_providedBuffers = false;
uint32_t *g_pColourBuffer = nullptr;
float *g_pDepthBuffer = nullptr;

uint32_t g_width = 1280;
uint32_t g_height = 720;

// This is a temp string so there is a window of time when the string is valid after being returned but won't leak
const char *g_tempStr = nullptr;

// Render data
udRenderSettings g_renderSettings = {};
udRenderPicking g_renderPicking = {};

udWorkerPool *g_pWorkerPool = nullptr;

#define WORKER_THREAD_COUNT 1

#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE 
#endif // !EMSCRIPTEN_KEEPALIVE

#define TO_JS_CODE(err) -((int)err)

extern "C" {
  uint32_t udSDKJS_VoxelShader_Colour(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData);
  uint32_t udSDKJS_VoxelShader_Classification(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData);
  uint32_t udSDKJS_VoxelShader_Intensity(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData);
  
  void udSDKJS_ClearBuffers()
  {
    udFree(g_pColourBuffer);
    udFree(g_pDepthBuffer);
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_GetErrorString(int code)
  {
    return udResultAsString(udResult(code));
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_SetServerAddress(const char *pNewServerAddress)
  {
    udError err = udE_Success;

    if (udStrlen(pNewServerAddress) < udLengthOf(g_serverAddress))
      udStrcpy(g_serverAddress, pNewServerAddress);
    else
      err = udE_MemoryAllocationFailure;

    return TO_JS_CODE(err);
  }

  udResult udSDKJS_Initialise()
  {
    udResult result;

    UD_ERROR_CHECK(udWorkerPool_Create(&g_pWorkerPool, WORKER_THREAD_COUNT));
    UD_ERROR_CHECK(g_renderData.Init(MaxRenderInstances));

    result = udR_Success;
  epilogue:
    return result;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_CreateShared(const char *pUsername, const char *pPassword, const char *pApplication)
  {
    udError err = udE_Success;

    udSDKJS_Initialise();

    printf("Trying resume\n");
    err = udContext_TryResume(&g_pContext, g_serverAddress, pApplication, pUsername, false);
    printf("... Resume: %d\n", (int)err);

    if (err != udE_Success)
    {
      printf("Trying login\n");
      err = udContext_ConnectLegacy(&g_pContext, g_serverAddress, pApplication, pUsername, pPassword);
      printf("... Login: %d\n", (int)err);
    }
    
    if (err != udE_Success)
      return TO_JS_CODE(err);

    err = udRenderContext_Create(g_pContext, &g_pRenderer);
    
    if (err != udE_Success)
      return TO_JS_CODE(err);

    err = udRenderTarget_Create(g_pContext, &g_pRenderTarget, g_pRenderer, g_width, g_height);
    
    if (err != udE_Success)
      return TO_JS_CODE(err);

    g_pColourBuffer = udAllocType(uint32_t, g_width * g_height, udAF_None);
    g_pDepthBuffer = udAllocType(float, g_width * g_height, udAF_None);
    err = udRenderTarget_SetTargets(g_pRenderTarget, g_pColourBuffer, 0xFF000000, g_pDepthBuffer);

    return TO_JS_CODE(err);
  }

  udResult udSDKJS_FinishSetup()
  {
    udResult result;

    UD_ERROR_CHECK((udResult)udRenderContext_Create(g_pContext, &g_pRenderer));
    UD_ERROR_CHECK((udResult)udRenderTarget_Create(g_pContext, &g_pRenderTarget, g_pRenderer, g_width, g_height));

    g_pColourBuffer = udAllocType(uint32_t, g_width * g_height, udAF_None);
    g_pDepthBuffer = udAllocType(float, g_width * g_height, udAF_None);
    UD_ERROR_IF(g_pColourBuffer == nullptr || g_pDepthBuffer == nullptr, udR_MemoryAllocationFailure);

    UD_ERROR_CHECK((udResult)udRenderTarget_SetTargets(g_pRenderTarget, g_pColourBuffer, 0xFF000000, g_pDepthBuffer));
    UD_ERROR_CHECK(udWorkerPool_Create(&g_pWorkerPool, WORKER_THREAD_COUNT));

    result = udR_Success;
  epilogue:

    if (result != udR_Success)
    {
      udRenderContext_Destroy(&g_pRenderer);
      udRenderTarget_Destroy(&g_pRenderTarget);
      udFree(g_pColourBuffer);
      udFree(g_pDepthBuffer);
    }

    return result;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_CreateSharedFromDomain(const char *pApplication)
  {
    udError err = udE_Success;

    udSDKJS_Initialise();
    err = udContext_ConnectFromDomain(&g_pContext, g_serverAddress, pApplication);

    if (err != udE_Success)
      return TO_JS_CODE(err);

    return TO_JS_CODE(udSDKJS_FinishSetup());
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_CreateSharedFrom_udCloud(const char *pApplication, void (*successCB)(), void (*failureCB)(int code))
  {
    udSDKJS_Initialise();
    char *pApp = udStrdup(pApplication);
    udWorkerPool_AddTask(g_pWorkerPool, [successCB, failureCB](void *pData)
      {
        udResult result;
        char *pApp = (char *)pData;

        UD_ERROR_CHECK((udResult)udContext_ConnectWithKey(&g_pContext, g_udCloudAddress, pApp, "1.0", nullptr));
        UD_ERROR_CHECK(udSDKJS_FinishSetup());

        result = udR_Success;
      epilogue:

#if UDPLATFORM_EMSCRIPTEN
        if (result == udR_Success)
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, successCB);
        else
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
#endif
        udFree(pApp);
      }, pApp, false);
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_CreateFrom_udCloud(const char *pApplication, void (*successCB)(), void (*failureCB)(int code))
  {
    udSDKJS_Initialise();
    char *pApp = udStrdup(pApplication);
    udWorkerPool_AddTask(g_pWorkerPool, [successCB, failureCB](void *pData)
      {
#if UDPLATFORM_EMSCRIPTEN
        udResult result;
        char *pApp = (char *)pData;

        udContextPartial *pPartial = nullptr;
        const char *pApprovePath = nullptr;

        UD_ERROR_CHECK((udResult)udContext_ConnectStart(&pPartial, g_udCloudAddress, pApp, "1", &pApprovePath, nullptr));
        UD_ERROR_IF(pApprovePath == nullptr, udR_InternalError);

        MAIN_THREAD_EM_ASM(window.open(UTF8ToString($0)), pApprovePath);

        do
        {
          result = (udResult)udContext_ConnectComplete(&g_pContext, &pPartial);
        } while (result == (udResult)udE_NothingToDo);

        if (pPartial != nullptr)
          udContext_ConnectCancel(&pPartial);

        UD_ERROR_CHECK(result);
        UD_ERROR_CHECK(udSDKJS_FinishSetup());

        result = udR_Success;
      epilogue:

        if (result == udR_Success)
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, successCB);
        else
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
        udFree(pApp);
#else
        udFree(pData);
#endif
      }, pApp, false);
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_DestroyShared(void (*successCB)(), void (*failureCB)(int code))
  {
    udWorkerPool_AddTask(g_pWorkerPool, [successCB, failureCB](void *)
      {
        udResult result = udR_Success;
        udError tempResult;

        tempResult = udRenderTarget_Destroy(&g_pRenderTarget);
        if (tempResult != udE_Success)
          result = (udResult)tempResult;

        tempResult = udRenderContext_Destroy(&g_pRenderer);
        if (tempResult != udE_Success)
          result = (udResult)tempResult;

        tempResult = udContext_Disconnect(&g_pContext, true);
        if (tempResult != udE_Success)
          result = (udResult)tempResult;

        udWorkerPool_Destroy(&g_pWorkerPool);

        udSDKJS_ClearBuffers();
        udFree(g_tempStr);

        tempResult = udScene_Release(&g_pScene);
        if (tempResult != udE_Success)
          result = (udResult)tempResult;

#if UDPLATFORM_EMSCRIPTEN
        if (result == udR_Success)
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, successCB);
        else
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
#endif
      }, nullptr);
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_ServerProjectLoad(const char *pSceneUUID, const char *pGroupID, void (*successCB)(), void (*failureCB)(int code))
  {
    char *pProjID = udStrdup(pSceneUUID);
    char *pGrpID = udStrdup(pGroupID);

    udWorkerPool_AddTask(g_pWorkerPool, [pProjID, pGrpID, successCB, failureCB](void *)
      {
        udResult result;

        if (g_pScene != nullptr)
          UD_ERROR_CHECK((udResult)udScene_Release(&g_pScene));

        UD_ERROR_CHECK((udResult)udScene_LoadFromServer(g_pContext, &g_pScene, pProjID, pGrpID));

        result = udR_Success;
      epilogue:

#if UDPLATFORM_EMSCRIPTEN
        if (result == udR_Success)
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, successCB);
        else
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
#endif
        char *pProjTemp = pProjID;
        char *pGrpTemp = pGrpID;
        udFree(pProjTemp);
        udFree(pGrpTemp);
      });
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ServerProjectRelease()
  {
    return TO_JS_CODE(udScene_Release(&g_pScene));
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_ServerProjectSave(void (*successCB)(), void (*failureCB)(int code))
  {
    udWorkerPool_AddTask(g_pWorkerPool, [successCB, failureCB](void *)
      {
         udError result = udScene_Save(g_pScene);
#if UDPLATFORM_EMSCRIPTEN
         if (result == udE_Success)
           emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, successCB);
         else
           emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
#endif
      });
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode* udSDKJS_GetProjectRoot()
  {
    udSceneNode *pNode = nullptr;

    udScene_GetProjectRoot(g_pScene, &pNode);

    return pNode;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode *udSDKJS_ProjectNodeCreate(udSceneNode *pParent, const char *pType, const char *pName, const char *pURI)
  {
    udSceneNode *pNode = nullptr;

    // Do we need to handle pUserData?
    udSceneNode_Create(g_pScene, &pNode, pParent, pType, pName, pURI, nullptr);
    
    return pNode;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode *udSDKJS_ProjectNodeCreateNoURI(udSceneNode *pParent, const char *pType, const char *pName)
  {
    udSceneNode *pNode = nullptr;

    // Do we need to handle pUserData?
    udSceneNode_Create(g_pScene, &pNode, pParent, pType, pName, nullptr, nullptr);

    return pNode;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNodeMoveChild(udSceneNode *pCurrentParent, udSceneNode *pNewParent, udSceneNode *pNode, udSceneNode *pInsertBeforeChild)
  {
    return TO_JS_CODE(udSceneNode_MoveChild(g_pScene, pCurrentParent, pNewParent, pNode, pInsertBeforeChild));
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNodeRemoveChild(udSceneNode *pParentNode, udSceneNode *pNode)
  {
    return TO_JS_CODE(udSceneNode_RemoveChild(g_pScene, pParentNode, pNode));
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNodeSetGeometry(udSceneNode *pNode, const char *pNodeType, int geometryCount, double *pCoordinates)
  {
    udError err = udE_InvalidParameter;

    if (udStrEqual(pNodeType, "Point"))
      err = udSceneNode_SetGeometry(g_pScene, pNode, udPGT_Point, geometryCount, pCoordinates);
    else if (udStrEqual(pNodeType, "MultiPoint"))
      err = udSceneNode_SetGeometry(g_pScene, pNode, udPGT_MultiPoint, geometryCount, pCoordinates);
    else if (udStrEqual(pNodeType, "LineString"))
      err = udSceneNode_SetGeometry(g_pScene, pNode, udPGT_LineString, geometryCount, pCoordinates);
    else if (udStrEqual(pNodeType, "MultiLineString"))
      err = udSceneNode_SetGeometry(g_pScene, pNode, udPGT_MultiLineString, geometryCount, pCoordinates);
    else if (udStrEqual(pNodeType, "Polygon"))
      err = udSceneNode_SetGeometry(g_pScene, pNode, udPGT_Polygon, geometryCount, pCoordinates);

    return TO_JS_CODE(err);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_GetVisibility(udSceneNode *pSceneNode)
  {
    return pSceneNode->isVisible;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetVisibility(udSceneNode *pSceneNode, int visibility)
  {
    return TO_JS_CODE(udSceneNode_SetVisibility(pSceneNode, visibility));
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_ProjectNode_GetUUID(udSceneNode *pSceneNode)
  {
    return pSceneNode->UUID;
  }

  EMSCRIPTEN_KEEPALIVE double udSDKJS_ProjectNode_GetLastUpdate(udSceneNode *pSceneNode)
  {
    return pSceneNode->lastUpdate;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNodeType udSDKJS_ProjectNode_GetItemType(udSceneNode *pSceneNode)
  {
    return pSceneNode->itemtype;
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_ProjectNode_GetItemTypeStr(udSceneNode *pSceneNode)
  {
    return pSceneNode->itemtypeStr;
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_ProjectNode_GetName(udSceneNode *pSceneNode)
  {
    return pSceneNode->pName;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetName(udSceneNode *pSceneNode, const char *pName)
  {
    return TO_JS_CODE(udSceneNode_SetName(g_pScene, pSceneNode, pName));
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_ProjectNode_GetURI(udSceneNode *pSceneNode)
  {
    return pSceneNode->pURI;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetURI(udSceneNode *pSceneNode, const char *pURI)
  {
    return TO_JS_CODE(udSceneNode_SetURI(g_pScene, pSceneNode, pURI));
  }

  EMSCRIPTEN_KEEPALIVE uint32_t udSDKJS_ProjectNode_GetHasBoundingBox(udSceneNode *pSceneNode)
  {
    return pSceneNode->hasBoundingBox;
  }

  EMSCRIPTEN_KEEPALIVE double* udSDKJS_ProjectNode_GetBoundingBox(udSceneNode *pSceneNode)
  {
    return pSceneNode->boundingBox;
  }

  EMSCRIPTEN_KEEPALIVE udSceneGeometryType udSDKJS_ProjectNode_GetGeomType(udSceneNode *pSceneNode)
  {
    return pSceneNode->geomtype;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_GetGeomCount(udSceneNode *pSceneNode)
  {
    return pSceneNode->geomCount;
  }

  EMSCRIPTEN_KEEPALIVE double* udSDKJS_ProjectNode_GetCoordinates(udSceneNode *pSceneNode)
  {
    return pSceneNode->pCoordinates;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode* udSDKJS_ProjectNode_GetParent(udSceneNode *pSceneNode)
  {
    return pSceneNode->pParent;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode* udSDKJS_ProjectNode_GetNextSibling(udSceneNode *pSceneNode)
  {
    return pSceneNode->pNextSibling;
  }

  EMSCRIPTEN_KEEPALIVE udSceneNode* udSDKJS_ProjectNode_GetFirstChild(udSceneNode *pSceneNode)
  {
    return pSceneNode->pFirstChild;
  }

  typedef void (udSceneNode_UserDataCleanup)(struct udSceneNode *pNode, void *pUserData);
  EMSCRIPTEN_KEEPALIVE udSceneNode_UserDataCleanup* udSDKJS_ProjectNode_GetUserDataCleanupCallback(udSceneNode *pSceneNode)
  {
    return pSceneNode->pUserDataCleanup;
  }

  EMSCRIPTEN_KEEPALIVE void* udSDKJS_ProjectNode_GetUserData(udSceneNode *pSceneNode)
  {
    return pSceneNode->pUserData;
  }

  EMSCRIPTEN_KEEPALIVE double udSDKJS_ProjectNode_GetMetadataInt(udSceneNode *pSceneNode, const char *pKey, int32_t defaultValue)
  {
    int32_t value = 0;
    udSceneNode_GetMetadataInt(pSceneNode, pKey, &value, defaultValue);
    return (double)value;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataInt(udSceneNode *pSceneNode, const char *pKey, int32_t value)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataInt(pSceneNode, pKey, value));
  }

  EMSCRIPTEN_KEEPALIVE double udSDKJS_ProjectNode_GetMetadataUint(udSceneNode *pSceneNode, const char *pKey, uint32_t defaultValue)
  {
    uint32_t value = 0;
    udSceneNode_GetMetadataUint(pSceneNode, pKey, &value, defaultValue);
    return (double)value;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataUint(udSceneNode *pSceneNode, const char *pKey, uint32_t value)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataUint(pSceneNode, pKey, value));
  }

  EMSCRIPTEN_KEEPALIVE double udSDKJS_ProjectNode_GetMetadataInt64(udSceneNode *pSceneNode, const char *pKey, int64_t defaultValue)
  {
    int64_t value = 0;
    udSceneNode_GetMetadataInt64(pSceneNode, pKey, &value, defaultValue);
    return (double)value;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataInt64(udSceneNode *pSceneNode, const char *pKey, int64_t value)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataInt64(pSceneNode, pKey, value));
  }

  EMSCRIPTEN_KEEPALIVE double udSDKJS_ProjectNode_GetMetadataDouble(udSceneNode *pSceneNode, const char *pKey, double defaultValue)
  {
    double value = 0;
    udSceneNode_GetMetadataDouble(pSceneNode, pKey, &value, defaultValue);
    return value;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataDouble(udSceneNode *pSceneNode, const char *pKey, double value)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataDouble(pSceneNode, pKey, value));
  }

  EMSCRIPTEN_KEEPALIVE uint32_t udSDKJS_ProjectNode_GetMetadataBool(udSceneNode *pSceneNode, const char *pKey, uint32_t defaultValue)
  {
    uint32_t value = 0;
    udSceneNode_GetMetadataBool(pSceneNode, pKey, &value, defaultValue);
    return value;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataBool(udSceneNode *pSceneNode, const char *pKey, uint32_t value)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataBool(pSceneNode, pKey, value));
  }

  EMSCRIPTEN_KEEPALIVE const char* udSDKJS_ProjectNode_GetMetadataString(udSceneNode *pSceneNode, const char *pKey, const char *pDefaultValue)
  {
    const char *pValue = 0;
    udSceneNode_GetMetadataString(pSceneNode, pKey, &pValue, pDefaultValue);
    return pValue;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ProjectNode_SetMetadataString(udSceneNode *pSceneNode, const char *pKey, const char *pValue)
  {
    return TO_JS_CODE(udSceneNode_SetMetadataString(pSceneNode, pKey, pValue));
  }

  EMSCRIPTEN_KEEPALIVE void udSDKJS_LoadModel(const char *pModelURL, void (*successCB)(int model), void (*failureCB)(int code))
  {
    char *pURL = udStrdup(pModelURL);

    // Danger! Worker thread touches global objects. This whole system needs to be revisited.
    udWorkerPool_AddTask(g_pWorkerPool, [successCB, failureCB](void *pData)
      {
        udPointCloud *pModel = nullptr;
        char *pURL = (char*)pData;
        udError result = udPointCloud_Load(g_pContext, &pModel, pURL, NULL);
#if UDPLATFORM_EMSCRIPTEN
        if (result == udE_Success)
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, successCB, (int)pModel);
        else
          emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, failureCB, TO_JS_CODE(result));
#endif
        udFree(pURL);
      }, pURL, false);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_SetMatrix(const char *pMatType, double v0, double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8, double v9, double v10, double v11, double v12, double v13, double v14, double v15)
  {
    udError err = udE_InvalidParameter;
    udDouble4x4 mat = udDouble4x4::create(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);

    if (udStrEqual(pMatType, "view"))
      err = udRenderTarget_SetMatrix(g_pRenderTarget, udRTM_View, mat.a);
    else if (udStrEqual(pMatType, "projection"))
      err = udRenderTarget_SetMatrix(g_pRenderTarget, udRTM_Projection, mat.a);
    else if (udStrEqual(pMatType, "camera"))
      err = udRenderTarget_SetMatrix(g_pRenderTarget, udRTM_Camera, mat.a);
    else if (udStrEqual(pMatType, "viewport"))
      err = udRenderTarget_SetMatrix(g_pRenderTarget, udRTM_Viewport, mat.a);

    return TO_JS_CODE(err);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueAddModel(udPointCloud *pModel, double zOffset, int targetZone) // zone == -1 is Esri ECEF, 0 is native
  {
    udResult result;
    udGeoZone targetGeoZone = {};
    udRenderInstance *pInstance = nullptr;
    udSDKJSModelInstance *pModelInstance = nullptr;
    udPointCloudHeader header = {};
    udDouble4x4 modelMat = {};
    udGeoZone srcZone = {};
    udDouble3 pivot = {};
    udDouble3 mid = {};
    int epsgCode = 0;
    int slotID = InvalidID;
    const char *pData = nullptr;

    UD_ERROR_IF(pModel == 0 || targetZone < -4, udR_InvalidParameter);

    slotID = g_renderData.InsertNew(&pModelInstance);
    UD_ERROR_IF(slotID == InvalidID, udR_CountExceeded);

    if (targetZone > 0)
      UD_ERROR_CHECK(udGeoZone_SetFromSRID(&targetGeoZone, targetZone));

    pInstance = pModelInstance->pRenderInstance;
    pInstance->pVoxelShader = udSDKJS_VoxelShader_Colour;
    pInstance->pPointCloud = pModel;

    udPointCloud_GetHeader(pModel, &header);

    modelMat = udDouble4x4::create(header.storedMatrix);

    pivot = udDouble3::create(header.pivot[0], header.pivot[1], header.pivot[2]);
    mid = (modelMat * udDouble4::create(pivot, 1.0)).toVector3();

    if (udPointCloud_GetMetadata(pModel, &pData) == udE_Success)
    {
      udJSON json = {};
      if (json.Parse(pData) == udR_Success)
      {
        const char *pSRID = json.Get("ProjectionID").AsString();

        if (pSRID != nullptr)
        {
          pSRID = udStrchr(pSRID, ":");
          if (pSRID != nullptr)
            epsgCode = udStrAtou(&pSRID[1]);

          if (udGeoZone_SetFromSRID(&srcZone, epsgCode) != udR_Success)
            epsgCode = 0;
        }
      }
    }

    if (epsgCode != 0 && targetZone != 0)
    {
      if (targetZone == -1) // Esri ECEF
      {
        udDouble3 latLong = udGeoZone_CartesianToLatLong(srcZone, mid);

        udDouble3 ypr = udDouble3::create(UD_DEG2RAD(latLong.y) - UD_HALF_PI, UD_DEG2RAD(latLong.x), 0);
        udDouble3 ecefPos = (6378137.0 + zOffset) * udDirectionFromYPR(ypr);

        modelMat = udDouble4x4::translation(pivot) * udDouble4x4::rotationYPR(ypr, ecefPos) * udDouble4x4::rotationYPR(UD_PI, UD_HALF_PI, 0.0) * udDouble4x4::scaleUniform(modelMat.axis.x.x) * udDouble4x4::translation(-pivot);
      }
      else if (targetZone == -2) // MapBox
      {
        udDouble3 latLong = udGeoZone_CartesianToLatLong(srcZone, mid);
        udDouble3 pos = {};

        double scale = (modelMat.a[0]) / (20026376.39 * 2.0) * zOffset;

        double yS = udLogN(udTan(UD_PI / 4.0 + UD_DEG2RAD(latLong.x) / 2.0));

        pos.x = (180.0 + latLong.y) / 360.0;
        pos.y = (180.0 - UD_RAD2DEG(yS)) / 360.0;
        pos.z = 0;

        modelMat = udDouble4x4::scaleNonUniform(scale, -scale, scale, pos) * udDouble4x4::translation(-pivot);
      }
      else if (targetZone == -3) // Remove Translation
      {
        modelMat.axis.t.x = 0.0;
        modelMat.axis.t.y = 0.0;
        modelMat.axis.t.z = 0.0;
      }
      else if (targetZone == -4) // Unit Cube
      {
        modelMat = udDouble4x4::identity();
      }
      else
      {
        modelMat = udGeoZone_TransformMatrix(modelMat, srcZone, targetGeoZone);
      }
    }

    memcpy(pInstance->matrix, modelMat.a, sizeof(pInstance->matrix));

    result = udR_Success;
  epilogue:

    if (result == udR_Success)
      return slotID;
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_SetModelMatrix(int slotID, double *pMatrix)
  {
    udResult result;
    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);
    
    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);
    UD_ERROR_IF(pMatrix == nullptr, udR_InvalidParameter);

    memcpy(pInstance->pRenderInstance->matrix, pMatrix, sizeof(pInstance->pRenderInstance->matrix));

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueAddModelWithMatrix(udPointCloud *pModel, double *pMatrix)
  {
    udResult result;
    udSDKJSModelInstance *pInstance = nullptr;
    int slotID = InvalidID;

    UD_ERROR_IF(pModel == nullptr || pMatrix == nullptr, udR_InvalidParameter);

    slotID = g_renderData.InsertNew(&pInstance);
    UD_ERROR_IF(slotID == InvalidID, udR_NotFound);

    pInstance->pRenderInstance->pVoxelShader = udSDKJS_VoxelShader_Colour;
    pInstance->pRenderInstance->pPointCloud = pModel;

    memcpy(pInstance->pRenderInstance->matrix, pMatrix, sizeof(pInstance->pRenderInstance->matrix));

    result = udR_Success;
  epilogue:

    if (result == udR_Success)
      return slotID;
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueRemoveItem(int slotID)
  {
    g_renderData.Erase(slotID);
    return TO_JS_CODE(udR_Success);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_SetColour(int slotID)
  {
    udResult result;
    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);

    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);

    pInstance->pRenderInstance->pVoxelShader = udSDKJS_VoxelShader_Colour;

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_SetIntensity(int slotID, int maxIntensity)
  {
    udResult result;

    udPointCloudHeader header = {};
    uint32_t offset = 0;

    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);

    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);
    UD_ERROR_IF(maxIntensity > 65535 || maxIntensity < 0, udR_InvalidParameter);

    UD_ERROR_CHECK((udResult)udPointCloud_GetHeader(pInstance->pRenderInstance->pPointCloud, &header));
    UD_ERROR_CHECK((udResult)udAttributeSet_GetOffsetOfStandardAttribute(&header.attributes, udSA_Intensity, &offset));

    pInstance->shaderData.attributeOffset = offset;
    pInstance->shaderData.maxIntensity = (float)maxIntensity;
    pInstance->pRenderInstance->pVoxelShader = udSDKJS_VoxelShader_Intensity;

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_ToggleGreyScale(int slotID, int greyScaleOn)
  {
    udResult result;
    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);

    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);

    pInstance->shaderData.isGreyScale = greyScaleOn == 0 ? false : true;

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_ToggleHighlight(int slotID, int highlightOn)
  {
    udResult result;
    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);

    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);

    pInstance->shaderData.isHighlighted = highlightOn == 0 ? false : true;

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_SetHighlightColour(int red, int green, int blue, float mix)
  {
    uint32_t ured = (uint32_t)udClamp(red, 0, 255);
    uint32_t ugreen = (uint32_t)udClamp(green, 0, 255);
    uint32_t ublue = (uint32_t)udClamp(blue, 0, 255);

    g_highlightColour = (ured << 16) | (ugreen << 8) | (ublue << 0);
    g_highlightStrength = udClamp(mix, 0.0f, 1.0f);

    return 0;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueItem_SetClassification(int slotID)
  {
    udResult result;
    udPointCloudHeader header = {};
    uint32_t offset = 0;
    udSDKJSModelInstance *pInstance = g_renderData.Get(slotID);

    UD_ERROR_IF(pInstance == nullptr, udR_NotFound);

    UD_ERROR_CHECK((udResult)udPointCloud_GetHeader(pInstance->pRenderInstance->pPointCloud, &header));
    UD_ERROR_CHECK((udResult)udAttributeSet_GetOffsetOfStandardAttribute(&header.attributes, udSA_Classification, &offset));

    pInstance->shaderData.attributeOffset = offset;
    pInstance->pRenderInstance->pVoxelShader = udSDKJS_VoxelShader_Classification;

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueueClear()
  {
    g_renderData.Clear();
    return (int)udE_Success;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_SetPick(double x, double y)
  {
    g_renderPicking.x = (unsigned int)x;
    g_renderPicking.y = (unsigned int)y;
    g_renderSettings.pPick = &g_renderPicking;
    return (int)udE_Success;
  }

  // TODO Implement user defined classifications.
  const char* GetClassificationName(uint8_t classification)
  {
    const int firstReserved = 19;
    const int firstUserDefined = 64;

    if (classification < firstReserved)
    {
      const char *names[] = {
        "Never Classified",
        "Unclassified",
        "Ground",
        "Low Vegetation",
        "Medium Vegetation",
        "High Vegetation",
        "Building",
        "Low Point",
        "Key Point",
        "Water",
        "Rail",
        "Road Surface",
        "Reserved",
        "Wire Guard",
        "Wire Conductor",
        "Transmission Tower",
        "Wire Structure Connector",
        "Bridge Deck",
        "High Noise"
      };

      return udTempStr("%d. %s", classification, names[classification]);
    }
    else if (classification < firstUserDefined)
    {
      return udTempStr("%d. %s", classification, "Reserved");
    }
    else // User defined
    {
      return udTempStr("%d. %s", classification, "User defined");
    }
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_GetVoxelData(char *pBuffer, int bufLen)
  {
    udError result;
    uint8_t *pAttributePtr = nullptr;
    const char *pTempStr = nullptr;
    udPointCloudHeader header = {};
    udJSON attributes = {};
    udPointCloud *pModel = nullptr;
    udSDKJSModelInstance *pInstance = g_renderData.GetFromIndex(g_renderPicking.modelIndex);

    UD_ERROR_IF(pInstance == nullptr, udE_NotFound);
    UD_ERROR_IF(g_renderPicking.voxelID.pTrav == nullptr, udE_NotFound);

    pModel = pInstance->pRenderInstance->pPointCloud;

    UD_ERROR_CHECK(udPointCloud_GetAttributeAddress(pModel, &g_renderPicking.voxelID, 0, (const void **)&pAttributePtr));
    UD_ERROR_CHECK(udPointCloud_GetHeader(pModel, &header));

    for (uint32_t i = 0; i < header.attributes.count; ++i)
    {
      if (header.attributes.pDescriptors[i].typeInfo == udATI_uint8 && udStrEqual(header.attributes.pDescriptors[i].name, "udClassification"))
      {
        uint8_t classificationID;
        udReadFromPointer(&classificationID, pAttributePtr);

        const char *pClassificationName = GetClassificationName(classificationID);

        attributes.Set("%s = '%s'", header.attributes.pDescriptors[i].name, pClassificationName);
        continue;
      }

      switch (header.attributes.pDescriptors[i].typeInfo)
      {
      case udATI_uint8:
      {
        uint8_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %u", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_uint16:
      {
        uint16_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %u", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_uint32:
      {
        uint32_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %u", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_uint64:
      {
        uint64_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %" PRIu64, header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_int8:
      {
        int8_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %i", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_int16:
      {
        int16_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %i", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_int32:
      {
        int32_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %i", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_int64:
      {
        int64_t val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %" PRId64, header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_float32:
      {
        float val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %.9f", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_float64:
      {
        double val;
        udReadFromPointer(&val, pAttributePtr);
        attributes.Set("%s = %.17f", header.attributes.pDescriptors[i].name, val);
        break;
      }
      case udATI_color32:
      {
        attributes.Set("%s.red = %u", header.attributes.pDescriptors[i].name, pAttributePtr[2]); //BGRA internally
        attributes.Set("%s.green = %u", header.attributes.pDescriptors[i].name, pAttributePtr[1]);
        attributes.Set("%s.blue = %u", header.attributes.pDescriptors[i].name, pAttributePtr[0]);
        pAttributePtr += 4;
        break;
      }
      case udATI_normal32: // Not currently supported
      case udATI_vec3f32: // Not currently supported
      default:
      {
        attributes.Set("%s = 'UNKNOWN'", header.attributes.pDescriptors[i].name);
        pAttributePtr += ((header.attributes.pDescriptors[i].typeInfo & udATI_SizeMask) >> udATI_SizeShift);
        break;
      }
      }
    }

    UD_ERROR_CHECK((udError)attributes.Export(&pTempStr));
    UD_ERROR_IF(udStrlen(pTempStr) > (size_t)bufLen, udE_BufferTooSmall);
    udStrcpy(pBuffer, (size_t)bufLen, pTempStr);

    result = udE_Success;
  epilogue:

    udFree(pTempStr);
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE udRenderPicking* udSDKJS_GetPickData()
  {
    return &g_renderPicking;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_GetPickedModelID()
  {
    if (g_renderPicking.hit != 0)
      return g_renderPicking.modelIndex;

    return TO_JS_CODE(udR_NotFound);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_RenderQueue()
  {
    udError err = udRenderContext_Render(g_pRenderer, g_pRenderTarget, g_renderData.GetRenderInstanceArray(), g_renderData.Size(), &g_renderSettings);
    g_renderSettings.pPick = nullptr;
    return TO_JS_CODE(err);
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ReleaseModel(udPointCloud *pModel)
  {
    return TO_JS_CODE(udPointCloud_Unload(&pModel));
  }

  EMSCRIPTEN_KEEPALIVE const char * udSDKJS_GetHeaderData(udPointCloud *pModel)
  {
    udFree(g_tempStr);

    udJSON retObj = {};

    udPointCloudHeader header = {};
    if (udPointCloud_GetHeader(pModel, &header) != udE_Success)
      return "{}";

    retObj.Set("scaledRange = %f", header.scaledRange);
    retObj.Set("unitMeterScale = %f", header.unitMeterScale);
    retObj.Set("totalLODLayers = %u", header.totalLODLayers);
    retObj.Set("convertedResolution = %f", header.convertedResolution);
    retObj.Set("storedMatrix = []");

    for (size_t i = 0; i < udLengthOf(header.storedMatrix); ++i)
      retObj.Set("storedMatrix[] = %f", header.storedMatrix[i]);

    retObj.Set("baseOffset = [ %f, %f, %f ]", header.baseOffset[0], header.baseOffset[1], header.baseOffset[2]);
    retObj.Set("pivot = [ %f, %f, %f ]", header.pivot[0], header.pivot[1], header.pivot[2]);
    retObj.Set("boundingBoxCenter = [ %f, %f, %f ]", header.boundingBoxCenter[0], header.boundingBoxCenter[1], header.boundingBoxCenter[2]);
    retObj.Set("boundingBoxExtents = [ %f, %f, %f ]", header.boundingBoxExtents[0], header.boundingBoxExtents[1], header.boundingBoxExtents[2]);

    retObj.Set("attributes = []");

    for (uint32_t i = 0; i < header.attributes.count; ++i)
    {
      udJSON attr = {};

      attr.Set("name = '%s'", header.attributes.pDescriptors[i].name);
      attr.Set("type = %u", header.attributes.pDescriptors[i].typeInfo);
      attr.Set("blending = %u", header.attributes.pDescriptors[i].blendType);

      retObj.Set(&attr, "attributes[]");
    }

    retObj.Set("metadata = {}");

    const char *pMetadata = nullptr;
    if (udPointCloud_GetMetadata(pModel, &pMetadata) == udE_Success)
    {
      udJSON temp = {};
      if (temp.Parse(pMetadata) == udR_Success)
      {
        bool foundLatLong = false;
        udGeoZone zone = {};

        if (!foundLatLong && temp.Get("ProjectionID").IsString())
        {
          const char *projID = temp.Get("ProjectionID").AsString();
          int srid = 0;

          if (udStrBeginsWithi(projID, "EPSG:"))
          {
            srid = udStrAtoi(&projID[5]);
            if (srid > 0 && udGeoZone_SetFromSRID(&zone, srid) == udR_Success)
              foundLatLong = true;
          }
        }

        if (!foundLatLong && temp.Get("ProjectionWKT").IsString())
        {
          const char *projWKT = temp.Get("ProjectionWKT").AsString();
          if (udGeoZone_SetFromWKT(&zone, projWKT) == udR_Success)
            foundLatLong = true;
        }

        if (foundLatLong)
        {
          udDouble3 center = (udDouble4x4::create(header.storedMatrix) * udDouble4::create(header.boundingBoxCenter[0], header.boundingBoxCenter[1], header.boundingBoxCenter[2], 1.0)).toVector3();
          udDouble3 position = udGeoZone_CartesianToLatLong(zone, center);

          retObj.Set("latLong = [ %f, %f ]", position.x, position.y);
        }

        retObj.Set(&temp, "metadata");
      }
    }

    retObj.Export(&g_tempStr);

    return g_tempStr;
  }

  EMSCRIPTEN_KEEPALIVE int udSDKJS_ResizeScene(uint32_t width, uint32_t height, uint32_t *pColourBuffer, float *pDepthBuffer)
  {
    udResult result;

    g_width = width;
    g_height = height;

    if (!g_providedBuffers)
      udSDKJS_ClearBuffers();

    if (pColourBuffer != nullptr && pDepthBuffer != nullptr)
    {
      g_pColourBuffer = pColourBuffer;
      g_pDepthBuffer = pDepthBuffer;
      g_providedBuffers = true;
    }
    else
    {
      g_pColourBuffer = udAllocType(uint32_t, width * height, udAF_None);
      g_pDepthBuffer = udAllocType(float, width * height, udAF_None);

      if (g_pColourBuffer == nullptr || g_pDepthBuffer == nullptr)
      {
        udSDKJS_ClearBuffers();
        result = udR_MemoryAllocationFailure;
        UD_ERROR_HANDLE();
      }

      g_providedBuffers = false;
    }

    UD_ERROR_CHECK((udResult)udRenderTarget_Destroy(&g_pRenderTarget));
    UD_ERROR_CHECK((udResult)udRenderTarget_Create(g_pContext, &g_pRenderTarget, g_pRenderer, width, height));
    UD_ERROR_CHECK((udResult)udRenderTarget_SetTargets(g_pRenderTarget, g_pColourBuffer, 0xFF000000, g_pDepthBuffer));

    result = udR_Success;
  epilogue:
    return TO_JS_CODE(result);
  }

  EMSCRIPTEN_KEEPALIVE uint32_t* udSDKJS_GetColourBuffer()
  {
    return g_pColourBuffer;
  }

  EMSCRIPTEN_KEEPALIVE float* udSDKJS_GetDepthBuffer()
  {
    return g_pDepthBuffer;
  }

  uint32_t PostVoxelShader(uint32_t colourIn, udSDKJSVoxelShaderData *pData)
  {
    uint32_t colourOut = colourIn;

    if (pData->isGreyScale)
    {
      uint32_t grey = (((colourOut >> 16) & 0xFFul) + ((colourOut >> 8) & 0xFFul) + ((colourOut >> 0) & 0xFFul)) / 3;
      colourOut = (colourOut & 0xFF000000ul) | (grey << 16) | (grey << 8) | (grey << 0);
    }
    if (pData->isHighlighted)
    {
      uint32_t r = (colourOut >> 16) & 0xFFul;
      uint32_t g = (colourOut >> 8) & 0xFFul;
      uint32_t b = (colourOut >> 0) & 0xFFul;

      float tr = float((g_highlightColour >> 16) & 0xFFul);
      float tg = float((g_highlightColour >> 8) & 0xFFul);
      float tb = float((g_highlightColour >> 0) & 0xFFul);

      r = uint32_t(r + g_highlightStrength * (tr - r));
      g = uint32_t(g + g_highlightStrength * (tg - g));
      b = uint32_t(b + g_highlightStrength * (tb - b));

      colourOut = (colourOut & 0xFF000000ul) | (r << 16) | (g << 8) | (b << 0);
    }

    return colourOut;
  }

  uint32_t udSDKJS_VoxelShader_Colour(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData)
  {
    udSDKJSVoxelShaderData *pData = (udSDKJSVoxelShaderData *)pUserData;

    uint32_t color = 0;
    udPointCloud_GetNodeColour(pPointCloud, pVoxelID, &color);

    return PostVoxelShader(color, pData);
  }

  uint32_t udSDKJS_VoxelShader_Intensity(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData)
  {
    udSDKJSVoxelShaderData *pData = (udSDKJSVoxelShaderData *)pUserData;

    uint32_t result = 0;
    const uint16_t *pIntensity = nullptr;
    udPointCloud_GetAttributeAddress(pPointCloud, pVoxelID, pData->attributeOffset, (const void**)&pIntensity);
    if (pIntensity != nullptr)
    {
      float value = float(*pIntensity) / pData->maxIntensity;
      uint32_t channel = udMin((uint32_t)(udPow(value, 1) * 255.0), (uint32_t)255);
      result = (channel * 0x00010101);
    }

    return PostVoxelShader(result, pData);
  }

  // This is the table from Geoverse MDM
  const uint32_t GeoverseClassificationColours[256] = {
    0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF008F00, 0xFF00BF00, 0xFF00FF00, 0xFFFF0000, 0xFF000000, 0xFFFFFF00, 0xFF0000FF, 0xFF00E0E0, 0xFFE000E0, 0xFF42F93F, 0xFFBD9E03, 0xFF59D67D, 0xFF9DA378,
    0xFF7B322A, 0xFF24783C, 0xFF572641, 0xFF11933F, 0xFF2DE7F1, 0xFFC4BE8D, 0xFF848277, 0xFFC7B531, 0xFF038006, 0xFF4BC007, 0xFF4AE9E5, 0xFF902579, 0xFF255060, 0xFFF13816, 0xFF573065, 0xFF008FB6,
    0xFF119417, 0xFFF01920, 0xFF01B37C, 0xFFF820F8, 0xFF360B65, 0xFF3AC7AA, 0xFF599DC2, 0xFF789E6E, 0xFF2C924A, 0xFF607084, 0xFFCB02F0, 0xFF4F3A2B, 0xFFEF5AD0, 0xFFC58F72, 0xFFFC2D2F, 0xFF68BB2C,
    0xFFB01C69, 0xFFBBA6F2, 0xFFF498F6, 0xFFA8C386, 0xFFD5A043, 0xFFF82C1E, 0xFF08E6D1, 0xFF3A4B6D, 0xFFD9940E, 0xFF4F7C71, 0xFF3FC4D7, 0xFF1EAF54, 0xFF1B105C, 0xFFFE5BA1, 0xFF354797, 0xFF4AEFEE,
    0xFF2E0401, 0xFF105FD5, 0xFF5B76D1, 0xFFFCE112, 0xFFE4A41C, 0xFF8568FE, 0xFFAFB7F0, 0xFFEFDD1C, 0xFFC1098B, 0xFF2F0504, 0xFFD68FA4, 0xFF1F2178, 0xFF6F2E64, 0xFF01C339, 0xFF821E2E, 0xFF491AB6,
    0xFF84B0DE, 0xFF989A38, 0xFFFD3F26, 0xFFE54F56, 0xFF4A2B71, 0xFFCD2651, 0xFF6DC483, 0xFFA41539, 0xFF99ADF4, 0xFFB76CBE, 0xFFED78EF, 0xFFD4AB55, 0xFF253DF5, 0xFF6C21E1, 0xFF518ADF, 0xFF0EB798,
    0xFFC7B5F8, 0xFFDCDB55, 0xFF33E09A, 0xFFA4FEED, 0xFF4345D3, 0xFF68F7D6, 0xFF667680, 0xFF1CA7E8, 0xFF5C01A2, 0xFFA1538A, 0xFF598AF6, 0xFF5CF0B7, 0xFFC5119C, 0xFF9AA0CA, 0xFFBA803E, 0xFF0728DB,
    0xFF0EE893, 0xFF09A841, 0xFF17AB0D, 0xFF0F7FDA, 0xFFB2928D, 0xFF104B73, 0xFFF7E755, 0xFF1BA2E6, 0xFF3ACCD5, 0xFFB56227, 0xFFA955D8, 0xFFC7936A, 0xFFF750E2, 0xFFB44444, 0xFFA9B691, 0xFFC2E80E,
    0xFF3DA935, 0xFFCA1040, 0xFF86F246, 0xFFADCEDC, 0xFF6B6249, 0xFFE6652A, 0xFF01EBFF, 0xFF2319FA, 0xFF9142A3, 0xFF1172EC, 0xFF5478E4, 0xFFC778B8, 0xFFF2DE44, 0xFF9740A0, 0xFF234A81, 0xFF0648E1,
    0xFFFBC350, 0xFF67C0FC, 0xFF0F1A90, 0xFF3A39CD, 0xFF9D7580, 0xFF5A5856, 0xFFF8A4EA, 0xFF90F487, 0xFFE56AAA, 0xFF5EFFCF, 0xFF798AF9, 0xFFC115AF, 0xFF9CEDB8, 0xFFE7B85E, 0xFF28B7A5, 0xFF76FACB,
    0xFF20C9F1, 0xFF2A0EA4, 0xFFF76BCF, 0xFFC98155, 0xFF52B01A, 0xFFC1C0AE, 0xFFD4DB5F, 0xFFD1BD6B, 0xFF9E10EB, 0xFF8C673C, 0xFFE16F6F, 0xFFDE3E35, 0xFFDD4E7C, 0xFF54CAA9, 0xFFDFD36B, 0xFF629407,
    0xFF4367F6, 0xFFF6A21F, 0xFF6C4F0B, 0xFF868BFD, 0xFFB86B03, 0xFF92E711, 0xFFE67372, 0xFFE4E372, 0xFF317F10, 0xFFB5F1EA, 0xFFFF3C7C, 0xFFE44800, 0xFF45EBCD, 0xFF5072EE, 0xFF2A57C2, 0xFF7F9B5F,
    0xFFA64DEB, 0xFF5A7B21, 0xFF955E83, 0xFF84C422, 0xFF0EB65E, 0xFF20864F, 0xFF80D422, 0xFF034DFB, 0xFF449601, 0xFF83BA8C, 0xFFAFB1E3, 0xFFB10FA4, 0xFFFEA5C1, 0xFF5A4EE9, 0xFF7CC026, 0xFFAA41BB,
    0xFF9009D0, 0xFF73FEC6, 0xFF3CA3FF, 0xFFF015E5, 0xFF1F11A4, 0xFFF7D512, 0xFF3F5DA6, 0xFFB921E1, 0xFF8E2586, 0xFF38868C, 0xFFD49A54, 0xFF45F3B1, 0xFFBB3BB2, 0xFF1C746C, 0xFFA40E8F, 0xFFDB3B3D,
    0xFF4584EB, 0xFF146BE4, 0xFF15D6E7, 0xFFEFE5D8, 0xFF40041C, 0xFF721B7B, 0xFF90DA1E, 0xFFF33C99, 0xFF5F22E8, 0xFF3A1513, 0xFF51B2EA, 0xFF48F302, 0xFFA1C02E, 0xFF70301C, 0xFFCACB56, 0xFFD9DB70,
    0xFF5031F3, 0xFF144A61, 0xFF1F5248, 0xFF51B448, 0xFFFCDAC1, 0xFFD39E32, 0xFFEEF810, 0xFF1AD15A, 0xFF4C20A9, 0xFF022971, 0xFF438646, 0xFF77EAE3, 0xFF203C67, 0xFF8929A1, 0xFFBA4F0C, 0xFFE67455 };

  uint32_t udSDKJS_VoxelShader_Classification(udPointCloud *pPointCloud, const udVoxelID *pVoxelID, const void *pUserData)
  {
    udSDKJSVoxelShaderData *pData = (udSDKJSVoxelShaderData *)pUserData;

    uint32_t result = 0;
    uint8_t *pClassification = nullptr;

    udPointCloud_GetAttributeAddress(pPointCloud, pVoxelID, pData->attributeOffset, (const void **)&pClassification);
    if (pClassification)
      result = GeoverseClassificationColours[*pClassification];

    return PostVoxelShader(result, pData);
  }

  udSDKJSRenderDataMap::udSDKJSRenderDataMap()
    : m_arraySize(0)
    , m_count(0)
    , m_pModelArray(nullptr)
    , m_pRenderInstanceArray(nullptr)
  {

  }

  udSDKJSRenderDataMap::~udSDKJSRenderDataMap()
  {
    udFree(m_pModelArray);
    udFree(m_pRenderInstanceArray);
    m_availableSlotArray.Deinit();
  }

  udResult udSDKJSRenderDataMap::Init(int count)
  {
    udResult result;

    UD_ERROR_IF(count < 1, udR_InvalidParameter);

    m_arraySize = count;

    m_pModelArray = udAllocType(udSDKJSModelInstance, m_arraySize, udAF_Zero);
    m_pRenderInstanceArray = udAllocType(udRenderInstance, m_arraySize, udAF_Zero);
    m_availableSlotArray.Init(16);

    UD_ERROR_IF(m_pModelArray == nullptr || m_pRenderInstanceArray == nullptr, udR_MemoryAllocationFailure);

    for (int i = 0; i < m_arraySize; ++i)
    {
      m_pModelArray[i].pRenderInstance = &m_pRenderInstanceArray[i];
      m_pRenderInstanceArray[i].pVoxelUserData = &m_pModelArray[i].shaderData;
    }

    result = udR_Success;
  epilogue:

    if (result != udR_Success)
    {
      udFree(m_pModelArray);
      udFree(m_pRenderInstanceArray);
      m_availableSlotArray.Deinit();
      m_arraySize = 0;
      m_count = 0;
    }

    return result;
  }

  udSDKJSModelInstance *udSDKJSRenderDataMap::Get(int key) const
  {
    if (key < 0 || key >= m_arraySize)
      return nullptr;

    return &m_pModelArray[key];
  }

  udSDKJSModelInstance *udSDKJSRenderDataMap::GetFromIndex(int index) const
  {
    return index >= m_arraySize || index < 0 ? nullptr : &m_pModelArray[index];
  }

  void udSDKJSRenderDataMap::Clear()
  {
    m_count = 0;
    m_availableSlotArray.Clear();
  }

  void udSDKJSRenderDataMap::Erase(int index)
  {
    if (index < 0 || index >= m_arraySize)
      return;

    m_pRenderInstanceArray[index].skipRender = 1;
    m_availableSlotArray.PushBack(index);
  }

  int udSDKJSRenderDataMap::InsertNew(udSDKJSModelInstance **ppData)
  {
    if (m_availableSlotArray.length == 0 && m_count + 1 > m_arraySize)
      return InvalidID;

    if (m_availableSlotArray.length > 0)
    {
      int index = m_availableSlotArray[0];
      memset(&m_pModelArray[index], 0, sizeof(*m_pModelArray));
      memset(&m_pRenderInstanceArray[index], 0, sizeof(*m_pRenderInstanceArray));

      m_pModelArray[index].pRenderInstance = &m_pRenderInstanceArray[index];
      m_pRenderInstanceArray[index].pVoxelUserData = &m_pModelArray[index].shaderData;

      *ppData = &m_pModelArray[index];

      m_availableSlotArray.PopFront();
      return index;
    }
    else
    {
      memset(&m_pModelArray[m_count], 0, sizeof(*m_pModelArray));
      memset(&m_pRenderInstanceArray[m_count], 0, sizeof(*m_pRenderInstanceArray));

      m_pModelArray[m_count].pRenderInstance = &m_pRenderInstanceArray[m_count];
      m_pRenderInstanceArray[m_count].pVoxelUserData = &m_pModelArray[m_count].shaderData;

      *ppData = &m_pModelArray[m_count];

      ++m_count;
      return m_count - 1; //last
    }
  }

#if UDPLATFORM_WINDOWS
  int main()
  {
    while (true)
      ;

    return 0;
  }
#endif
}
