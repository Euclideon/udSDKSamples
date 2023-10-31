#ifndef UDGLIMPL_H
#define UDGLIMPL_H

// ---------------------------------------------------------------------
// Default implementation of udBlockRenderGPUInterface for OpenGL
// ---------------------------------------------------------------------

#include "udBlockRenderContext.h"
#include "udThread.h"
#include "udPlatformUtil.h"

#define BLOCKSPACE_POSITIONS 1

#define VERIFY_GL() { GLuint glErrorCode = glGetError(); if (glErrorCode != GL_NO_ERROR) udDebugPrintf("%s(%d): glError %d\n", __FILE__, __LINE__, glErrorCode); UDASSERT(glErrorCode == GL_NO_ERROR, ""); }

// ---------------------------------------------------------------------
struct GLBlockRenderProgram
{
  const char *pDescription;
  GLuint programID;
  GLint worldViewProjLoc;
  GLint screenSizeLoc;

  udError Compile(const char *pVertShader, const char *pFragShader, const char *pDesc)
  {
    udError result;

    programID = GL_INVALID_INDEX;
    GLuint vertexShaderID = GL_INVALID_INDEX;
    GLuint fragShaderID = GL_INVALID_INDEX;
    GLint compileResult = GL_FALSE;

    // Compile Vertex Shader
    vertexShaderID = glCreateShader(GL_VERTEX_SHADER); VERIFY_GL();
    glShaderSource(vertexShaderID, 1, &pVertShader, NULL); VERIFY_GL();
    glCompileShader(vertexShaderID); VERIFY_GL();
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &compileResult); VERIFY_GL();
    UD_ERROR_IF(!compileResult, udE_Failure);

    // Compile Fragment Shader
    fragShaderID = glCreateShader(GL_FRAGMENT_SHADER); VERIFY_GL();
    glShaderSource(fragShaderID, 1, &pFragShader, NULL); VERIFY_GL();
    glCompileShader(fragShaderID); VERIFY_GL();
    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &compileResult); VERIFY_GL();
    UD_ERROR_IF(!compileResult, udE_Failure);

    // Link the program
    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &compileResult);
    UD_ERROR_IF(!compileResult, udE_Failure);

    // Get the uniform locations
    worldViewProjLoc = glGetUniformLocation(programID, "u_worldViewProj");
    screenSizeLoc = glGetUniformLocation(programID, "u_screenSize");

    pDescription = pDesc;
    result = udE_Success;

  epilogue:
    if (result != udE_Success)
    {
      char log[1024] = "";
      const char *pThing;
      if (programID != GL_INVALID_INDEX)
      {
        pThing = "program";
      }
      else if (fragShaderID != GL_INVALID_INDEX)
      {
        pThing = "fragment shader";
        glGetShaderInfoLog(fragShaderID, sizeof(log), NULL, log);
      }
      else
      {
        pThing = "vertex shader";
        glGetShaderInfoLog(vertexShaderID, sizeof(log), NULL, log);
      }

      udDebugPrintf("Error compiling %s shader for %s: %s\n", pThing, pDesc, log);
    }

    // These shaders are referenced by the program now, so deleting them just
    // reduces their reference count so when we delete the program they are deleted
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragShaderID);

    return result;
  }
  void Use()
  {
    glUseProgram(programID); VERIFY_GL();
  }
  void Bind(GLuint vaoId)
  {
    glBindVertexArray(vaoId);
  }
  void Destroy()
  {
    glDeleteProgram(programID);
  }
};

// ---------------------------------------------------------------------
struct udGLImplContext
{
  GLBlockRenderProgram pointsProgram;
  GLBlockRenderProgram quadsProgram;
  GLBlockRenderProgram *pBlockRenderProgram; // Current program in use
  const udBlockRenderModel *pUploadedModelData; // For tracking which model shader current constants refer to

  uint32_t width, height; 

  int32_t numberOfBlocksRendered;
  int32_t numberOfDrawCalls;
  int32_t numberOfVBOsUploaded;
  int64_t numberOfVerticesUploaded;
  int64_t numberOfVerticesRendered;
  float createVertexBufferTimeMs;
  float uploadTimeMs;
  float drawTimeMs;
};

udGLImplContext g_udGLImplContextMem;


static const char *pVoxelVertShader_Points = R"(
#version 330 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
out vec4 v_color;
uniform mat4 u_worldViewProj;
uniform vec2 u_screenSize;

void main()
{
  v_color = a_color.bgra;
  vec4 pos = u_worldViewProj * vec4(a_position.xyz + a_position.www,1);
  gl_Position = pos / pos.w;
  gl_PointSize = 1;
};
)";

static const char *pVoxelVertShader_Quads = R"(
#version 330 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
out vec4 v_color;
uniform mat4 u_worldViewProj;
uniform vec2 u_screenSize;

void main()
{
  v_color = a_color.bgra;

  // Points of each corner
  vec4 off = vec4(a_position.www * 2.0, 0);
  vec4 pos0 = u_worldViewProj * vec4(a_position.xyz + off.www, 1);
  vec4 pos1 = u_worldViewProj * vec4(a_position.xyz + off.xww, 1);
  vec4 pos2 = u_worldViewProj * vec4(a_position.xyz + off.xyw, 1);
  vec4 pos3 = u_worldViewProj * vec4(a_position.xyz + off.wyw, 1);
  vec4 pos4 = u_worldViewProj * vec4(a_position.xyz + off.wwz, 1);
  vec4 pos5 = u_worldViewProj * vec4(a_position.xyz + off.xwz, 1);
  vec4 pos6 = u_worldViewProj * vec4(a_position.xyz + off.xyz, 1);
  vec4 pos7 = u_worldViewProj * vec4(a_position.xyz + off.wyz, 1);
  vec4 minPos, maxPos;
  minPos = min(pos0, pos1);
  minPos = min(minPos, pos2);
  minPos = min(minPos, pos3);
  minPos = min(minPos, pos4);
  minPos = min(minPos, pos5);
  minPos = min(minPos, pos6);
  minPos = min(minPos, pos7);
  maxPos = max(pos0, pos1);
  maxPos = max(maxPos, pos2);
  maxPos = max(maxPos, pos3);
  maxPos = max(maxPos, pos4);
  maxPos = max(maxPos, pos5);
  maxPos = max(maxPos, pos6);
  maxPos = max(maxPos, pos7);
  gl_Position = minPos + (maxPos - minPos) * 0.5;
  // To find correct size of this point, need to be in NDC space. 
  pos0.xy /= pos0.w;
  pos1.xy /= pos1.w;
  pos2.xy /= pos2.w;
  pos3.xy /= pos3.w;
  pos4.xy /= pos4.w;
  pos5.xy /= pos5.w;
  pos6.xy /= pos6.w;
  pos7.xy /= pos7.w;
  minPos = min(pos0, pos1);
  minPos = min(minPos, pos2);
  minPos = min(minPos, pos3);
  minPos = min(minPos, pos4);
  minPos = min(minPos, pos5);
  minPos = min(minPos, pos6);
  minPos = min(minPos, pos7);
  maxPos = max(pos0, pos1);
  maxPos = max(maxPos, pos2);
  maxPos = max(maxPos, pos3);
  maxPos = max(maxPos, pos4);
  maxPos = max(maxPos, pos5);
  maxPos = max(maxPos, pos6);
  maxPos = max(maxPos, pos7);
  float m = max((maxPos.x - minPos.x) * u_screenSize.x, (maxPos.y - minPos.y) * u_screenSize.y);
  m = min(m, 150.0); // Cap size of huge points
  const float maxPointSize = 2.0;
  gl_PointSize = max(maxPointSize, ceil(m * 0.5));
};
)";

static const char *pVoxelFragShader = R"(
#version 330 core
in vec4 v_color;
out vec4 out_color;

void main()
{
  out_color = v_color;
};
)";

// ---------------------------------------------------------------------
struct udGLImplVertexBuffer
{
  enum { MAX_VBOS = 9 };
  int vertexCount;
  unsigned vbo;
  unsigned vao;

  // TEMP HARD CODED VERTEX
  // The vertex buffer is divided into "pieces", which correspond to child block
  // sizes. The main vbo is populated from pieces as required
  struct Vertex
  {
    udFloat4 pos;
    uint32_t color;
  } *pVertexData; // In-memory temp buffer of data waiting to be uploaded as a VBO before being freed
  class udBlock *pBlock;
  udDouble4x4 blockToModel;
};

/*----------------------------------------------------------------------------------------------------*/
void udGLImpl_BeginRender(void *pGPUContext, const udRenderView *pView, uint32_t width, uint32_t height)
{
  udGLImplContext *pCtx = (udGLImplContext*)pGPUContext;

  // Store copies of the render dimensions
  pCtx->width = width;
  pCtx->height = height;

  pCtx->numberOfBlocksRendered = 0;
  pCtx->numberOfDrawCalls = 0;
  pCtx->numberOfVBOsUploaded = 0;
  pCtx->numberOfVerticesUploaded = 0;
  pCtx->numberOfVerticesRendered = 0;
  pCtx->createVertexBufferTimeMs = 0;
  pCtx->uploadTimeMs = 0;
  pCtx->drawTimeMs = 0;
  pCtx->pBlockRenderProgram = nullptr;

  glDisable(GL_CULL_FACE); VERIFY_GL();
  glEnable(GL_DEPTH_TEST); VERIFY_GL();
  glDepthMask(GL_TRUE); VERIFY_GL();
  glDepthFunc(GL_LEQUAL); VERIFY_GL();

  glEnable(GL_PROGRAM_POINT_SIZE); VERIFY_GL();
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); VERIFY_GL();

  glDisable(GL_BLEND); VERIFY_GL();
}

// ---------------------------------------------------------------------
udError udGLImpl_CreateVertexBuffer(void *pGPUContext, const udBlockRenderModel *, const udBlockRenderVertexData vertexData, void **ppVertexBuffer)
{
  udError result;
  udGLImplVertexBuffer *pVertexBuffer = nullptr;
  uint32_t colorOffset = 0, intensityOffset = 0;

  udAttributeSet_GetOffsetOfStandardAttribute(&vertexData.pPointBuffer->attributes, udSA_ARGB, &colorOffset);
  udAttributeSet_GetOffsetOfStandardAttribute(&vertexData.pPointBuffer->attributes, udSA_ARGB, &intensityOffset);

  uint64_t startTm = udPerfCounterStart();
  pVertexBuffer = udAllocType(udGLImplVertexBuffer, 1, udAF_Zero);
  UD_ERROR_NULL(pVertexBuffer, udE_MemoryAllocationFailure);
  pVertexBuffer->vertexCount = (int)vertexData.pPointBuffer->pointCount;
  pVertexBuffer->blockToModel = udInverse(vertexData.modelToBlock);

  pVertexBuffer->vao = GL_INVALID_INDEX;
  pVertexBuffer->vbo = GL_INVALID_INDEX;
  pVertexBuffer->pBlock = (class udBlock *)vertexData.pBlock;
  pVertexBuffer->pVertexData = udAllocType(udGLImplVertexBuffer::Vertex, vertexData.pPointBuffer->pointCount, udAF_None);
  UD_ERROR_NULL(pVertexBuffer->pVertexData, udE_MemoryAllocationFailure);

  // HARD CODING TO fixed vert format for the moment
  for (uint32_t i = 0; i < vertexData.pPointBuffer->pointCount; ++i)
  {
    udDouble3 p = udBlockRenderVertexData_GetFloatPosition(&vertexData, i);
    if constexpr (BLOCKSPACE_POSITIONS)
      p = udMul(vertexData.modelToBlock, p);
    pVertexBuffer->pVertexData[i].pos = udFloat4::create((float)p.x, (float)p.y, (float)p.z, (float)vertexData.childSize);
  }
  if (vertexData.pPointBuffer->attributes.content & udSAC_ARGB)
  {
    for (uint32_t i = 0; i < vertexData.pPointBuffer->pointCount; ++i)
      pVertexBuffer->pVertexData[i].color = *(uint32_t *)(vertexData.pPointBuffer->GetAttributes(i) + colorOffset);
  }
  else if (vertexData.pPointBuffer->attributes.content & udSAC_Intensity)
  {
    for (uint32_t i = 0; i < vertexData.pPointBuffer->pointCount; ++i)
      pVertexBuffer->pVertexData[i].color = 0xff000000 | *(uint16_t *)(vertexData.pPointBuffer->GetAttributes(i) + intensityOffset);
  }
  else // Colour by height in the absence of anything else
  {
    for (uint32_t i = 0; i < vertexData.pPointBuffer->pointCount; ++i)
      pVertexBuffer->pVertexData[i].color = 0xff000000 | (0x010101 * int(udBlockRenderVertexData_GetFloatPosition(&vertexData, i).y * 255));
  }

  *ppVertexBuffer = (void *)pVertexBuffer;
  pVertexBuffer = nullptr;
  result = udE_Success;

epilogue:
  if (pVertexBuffer)
  {
    udFree(pVertexBuffer->pVertexData);
    udFree(pVertexBuffer);
  }
  if (pGPUContext)
    ((udGLImplContext *)pGPUContext)->createVertexBufferTimeMs = udPerfCounterMilliseconds(startTm);
  return (udError)result;
}


// ---------------------------------------------------------------------
udError udGLImpl_UploadVertexBuffer(void *pGPUContext, const udBlockRenderModel * /*pModel*/, void *pVertexBuffer, void *pVoxelShaderData)
{
  udUnused(pVoxelShaderData);
  udResult result;
  udGLImplVertexBuffer *pVB = (udGLImplVertexBuffer*)pVertexBuffer;
  udGLImplContext *pCtx = (udGLImplContext*)pGPUContext;

  if (pVB->vao == GL_INVALID_INDEX)
  {
    uint64_t startTm = udPerfCounterStart();

    glGenVertexArrays(1, &pVB->vao);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
    glBindVertexArray(pVB->vao);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);

    glGenBuffers(1, &pVB->vbo);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
    glBindBuffer(GL_ARRAY_BUFFER, pVB->vbo);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);

    glEnableVertexAttribArray(0);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
    glEnableVertexAttribArray(1);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(udGLImplVertexBuffer::Vertex), reinterpret_cast<void*>(offsetof(udGLImplVertexBuffer::Vertex, pos)));
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(udGLImplVertexBuffer::Vertex), reinterpret_cast<void*>(offsetof(udGLImplVertexBuffer::Vertex, color)));
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);

    glBufferData(GL_ARRAY_BUFFER, pVB->vertexCount * sizeof(udGLImplVertexBuffer::Vertex), pVB->pVertexData, GL_STATIC_DRAW);
    UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
    pCtx->numberOfVerticesUploaded += pVB->vertexCount;
    ++pCtx->numberOfVBOsUploaded;
    pCtx->uploadTimeMs += udPerfCounterMilliseconds(startTm);
  }

  result = udR_Success;

epilogue:
  glBindVertexArray(0);
  return (udError)result;
}

/*----------------------------------------------------------------------------------------------------*/
udError udGLImpl_RenderVertexBuffer(void *pGPUContext, const udBlockRenderModel *pModel, void *pVertexBuffer, void * /*pVoxelShaderData*/, const udBlockRenderInfo *pInfo)
{
  udResult result;
  udGLImplVertexBuffer *pVB = (udGLImplVertexBuffer *)pVertexBuffer;
  udGLImplContext *pCtx = (udGLImplContext *)pGPUContext;
  GLBlockRenderProgram *pProgramToUse;

  if (pInfo->pointMode == udRCPM_Points)
    pProgramToUse = &pCtx->pointsProgram;
  else
    pProgramToUse = &pCtx->quadsProgram;

  if (pCtx->pBlockRenderProgram != pProgramToUse)
  {
    pCtx->pBlockRenderProgram = pProgramToUse;
    pCtx->pBlockRenderProgram->Use();
    pCtx->pUploadedModelData = nullptr;
    if (pCtx->pBlockRenderProgram->screenSizeLoc >= 0)
      glUniform2f(pCtx->pBlockRenderProgram->screenSizeLoc, (float)pCtx->width, (float)pCtx->height); VERIFY_GL();
  }

  ++pCtx->numberOfBlocksRendered;
  if (pCtx->pUploadedModelData != pModel || BLOCKSPACE_POSITIONS)
  {
    // Upload shader constants
    udDouble4x4 wvp;
    if constexpr (BLOCKSPACE_POSITIONS)
      wvp = udMul(pModel->projection, udMul(pModel->worldView, pVB->blockToModel));
    else
      wvp = udMul(pModel->projection, pModel->worldView);
    udFloat4x4 fwvp = udFloat4x4::create(wvp);


    glUniformMatrix4fv(pCtx->pBlockRenderProgram->worldViewProjLoc, 1, GL_FALSE, fwvp.a); VERIFY_GL();
    pCtx->pUploadedModelData = pModel;
  }

  pCtx->pBlockRenderProgram->Bind(pVB->vao);

  uint64_t startTm = udPerfCounterStart();
  for (int dc = 0; dc < pInfo->pDrawList->drawCount; ++dc)
  {
    if (pInfo->pDrawList->draws[dc].mask & pInfo->divisionsMask)
    {
      int start = pInfo->pDrawList->draws[dc].start;
      int count = pInfo->pDrawList->draws[dc].count;

      ++pCtx->numberOfDrawCalls;
      glDrawArrays(GL_POINTS, start, count); VERIFY_GL();
      UD_ERROR_IF(glGetError() != GL_NO_ERROR, udR_InternalError);
      pCtx->numberOfVerticesRendered += count;
    }
  }

  glBindVertexArray(0);

  pCtx->drawTimeMs += udPerfCounterMilliseconds(startTm);
  result = udR_Success;

epilogue:
  return (udError)result;
}


/*----------------------------------------------------------------------------------------------------*/
void udGLImpl_EndRender(void *pGPUContext)
{
  udGLImplContext *pCtx = (udGLImplContext *)pGPUContext;
  glUseProgram(0); VERIFY_GL();
  pCtx->pBlockRenderProgram = nullptr;
}


/*----------------------------------------------------------------------------------------------------*/
udError udGLImpl_DestroyVertexBuffer(void * /*pGPUContext*/, void *pVertexBuffer)
{
  udGLImplVertexBuffer *pVB = (udGLImplVertexBuffer*)pVertexBuffer;
  if (pVB->vao != GL_INVALID_INDEX)
  {
    glDeleteVertexArrays(1, &pVB->vao);
    glDeleteBuffers(1, &pVB->vbo);
  }
  udFree(pVB->pVertexData);
  udFree(pVertexBuffer);
  return (glGetError() != GL_NO_ERROR) ? udE_Success : udE_InternalError;
}

// ---------------------------------------------------------------------
udError udGLImpl_Init(int targetPointCount = 7000000, float threshold = 0.5f)
{
  udError result;

  UD_ERROR_CHECK(g_udGLImplContextMem.pointsProgram.Compile(pVoxelVertShader_Points, pVoxelFragShader, "Points"));
  UD_ERROR_CHECK(g_udGLImplContextMem.quadsProgram.Compile(pVoxelVertShader_Quads, pVoxelFragShader, "Quads"));

  static udBlockRenderGPUInterface gpuInterface;
  gpuInterface.pBeginRender = udGLImpl_BeginRender;
  gpuInterface.pEndRender = udGLImpl_EndRender;
  gpuInterface.pCreateVertexBuffer = udGLImpl_CreateVertexBuffer;
  gpuInterface.pUploadVertexBuffer = udGLImpl_UploadVertexBuffer;
  gpuInterface.pRenderVertexBuffer = udGLImpl_RenderVertexBuffer;
  gpuInterface.pDestroyVertexBuffer = udGLImpl_DestroyVertexBuffer;
  gpuInterface.pGPUContext = &g_udGLImplContextMem;

  UD_ERROR_CHECK(udBlockRenderContext_Init(&gpuInterface));

epilogue:
  return result;
}

// ---------------------------------------------------------------------
void udGLImpl_Deinit()
{
  g_udGLImplContextMem.pointsProgram.Destroy();
  g_udGLImplContextMem.quadsProgram.Destroy();
}


#endif // UDGLIMPL_H
