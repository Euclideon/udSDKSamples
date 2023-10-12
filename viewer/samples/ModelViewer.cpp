#include "../udSample.h"
#include "udSDKFeatureSamples.h"

#include "udRenderContext.h"
#include "udRenderTarget.h"
#include "udGeometry.h"
#include "udPlatformUtil.h"
#include "udStringUtil.h"

#include "SDL.h"
#include "SDL_opengl.h"
#include "imgui.h"

class ModelViewer : public udSample
{
public:
  ModelViewer(const char *pSampleName) : udSample(pSampleName), udQuadTexture(GL_INVALID_INDEX), quadPixelCount(0) { pFilename = nullptr; pModel = nullptr; }
  udError Init(udSampleRenderInfo &info) override;
  void Deinit() override;
  udError Render(udSampleRenderInfo &info) override;
  bool Event(udSampleRenderInfo &info, const SDL_Event &event) override;

protected:
  bool HandleSelection(); // Returns true when state moves to SS_Selected

  const char *pFilename;
  udDouble4x4 mat;
  udDouble4x4 camera;
  udPointCloud *pModel;
  udPointCloudHeader header;
  udRenderPicking pick;
  udError lastError;
  GLuint udQuadTexture;
  uint32_t quadPixelCount;

  // Selection data
  uint32_t sdlMouseButtonState;
  udDouble3 selectPoint1, selectPoint2;
  udGeometry selectionGeo, inverseGeo, rotatingGeo;
  bool filterSelection, inverseSelection, rotatingFilter;
  enum SelectState { SS_None, SS_FirstPoint, SS_SecondPoint, SS_Selected } selectState;
};
static ModelViewer instance("Model Viewer");


// ----------------------------------------------------------------------------
// Allocate and initialise the sample's state
udError ModelViewer::Init(udSampleRenderInfo &info)
{
  mat = camera = udDouble4x4::identity();
  pFilename = udStrdup(UDSAMPLE_ASSETDIR "/HistogramTest.uds");
  if (!pFilename)
    return udE_MemoryAllocationFailure;

  udDouble4x4 projMat = udDouble4x4::perspectiveZO(UD_PI / 3, info.width / (double)info.height, 0.01, 10000.0);
  udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Projection, projMat.a);

  glGenTextures(1, &udQuadTexture);

  return udE_Success;
}

// ----------------------------------------------------------------------------
// Tear-down for the sample, freeing resources
void ModelViewer::Deinit()
{
  udFree(pFilename);
  udPointCloud_Unload(&pModel);
  glDeleteTextures(1, &udQuadTexture);
  udQuadTexture = GL_INVALID_INDEX;
}

// ----------------------------------------------------------------------------
// Low-light voxel shader
static uint32_t VoxelShader_LowLight(struct udPointCloud *pPointCloud, const struct udVoxelID *pVoxelID, const void *pVoxelUserData)
{
  uint32_t colour = 0;
  udPointCloud_GetNodeColour(pPointCloud, pVoxelID, &colour);
  return ((colour & 0xfefefe) >> 1) | 0xff000000;
}

// ----------------------------------------------------------------------------
// Render a frame of the sample, which includes some update for simplicity
udError ModelViewer::Render(udSampleRenderInfo &info)
{
  udError result;

  // If a new model is waiting to be loaded, do that
  if (pFilename)
  {
    if (pModel)
      udPointCloud_Unload(&pModel);
    UD_ERROR_CHECK(udPointCloud_Load(info.pContext, &pModel, pFilename, &header));
    udFree(pFilename);
    camera = udDouble4x4::identity();
    mat = udDouble4x4::create(header.storedMatrix);
    camera.axis.t = mat.axis.t + ((mat.axis.x + mat.axis.z) * 0.25);
  }

  if (pModel)
  {
    // Take the inputs and update the camera
    UpdateCamera(&camera, info.dt, info.moveSpeed, info.turnSpeed);

    int imgPitch = 0;
    udRenderSettings options = {};
    udRenderInstance instances[2] = {};
    options.pPick = &pick;

    instances[0].pPointCloud = pModel;
    memcpy(instances[0].matrix, header.storedMatrix, sizeof(header.storedMatrix));

    sdlMouseButtonState = SDL_GetMouseState((int*)&pick.x, (int*)&pick.y);

    udRenderTarget_SetMatrix(info.pRenderTarget, udRTM_Camera, camera.a);

    if (HandleSelection() || selectState == SS_SecondPoint)
    {
      // An area has been selected so the geometry needs to be initialised
      filterSelection = inverseSelection = rotatingFilter = false;
      selectionGeo.type = udGT_Count;
      udDouble3 ofs = udDouble3::create(0.0, 0.0, header.boundingBoxExtents[2] * header.scaledRange);
      UD_ERROR_CHECK(udGeometry_InitAABBFromMinMax(&selectionGeo, selectPoint1 + ofs, selectPoint2 - ofs));
      udGeometry_InitInverse(&inverseGeo, &selectionGeo);
    }
    else if (selectState != SS_Selected)
    {
      selectionGeo.type = udGT_Count;
    }

    instances[0].pFilter = (selectionGeo.type != udGT_Count) ? &selectionGeo : nullptr;
    int instanceCount = 1;
    if (!filterSelection)
    {
      // Counter-intuitively, when we're not filtering the selection, we're actually going to render
      // another copy of the instance, but this time with an inverse of the filter and a low-light voxel shader
      instances[instanceCount] = instances[0];
      instances[instanceCount].pFilter = &inverseGeo;
      instances[instanceCount].pVoxelShader = VoxelShader_LowLight;
      ++instanceCount;
    }
    else
    {
      if (rotatingFilter)
      {
        static double rot = 0;
        if ((rot += UD_PI * info.dt) >= UD_2PI)
          rot -= UD_2PI;
        lastError = udGeometry_InitOBB(&rotatingGeo, selectionGeo.data.aabb.center, selectionGeo.data.aabb.extents, udDouble3::create(rot, 0.0, 0.0));
        if (lastError == udR_Success)
          instances[0].pFilter = &rotatingGeo;
      }
      if (inverseSelection)
      {
        udGeometry_InitInverse(&inverseGeo, instances[0].pFilter);
        instances[0].pFilter = &inverseGeo;
      }
    }

    if (ImGui::Begin("ModelViewer"))
    {
      ImGui::Text("Last result: %s", udError_GetErrorString(lastError));
      if (pick.hit)
        ImGui::Text("Pick: %f,%f,%f", pick.pointCenter[0], pick.pointCenter[1], pick.pointCenter[2]);
      else
        ImGui::Text("Pick: ");
      if (selectState != SS_Selected)
      {
        ImGui::Text(selectState == SS_None ? "Right-click and drag to select" : "Selecting");
      }
      else
      {
        ImGui::Checkbox("Filter", &filterSelection);
        if (filterSelection)
        {
          ImGui::SameLine();
          ImGui::Checkbox("Invert", &inverseSelection);
          ImGui::SameLine();
          ImGui::Checkbox("Rotating", &rotatingFilter);
        }
      }
    }
    ImGui::End();

    if (info.useGpuRenderer)
      info.useGpuRenderer = false; // When we don't support the gpu renderer, make it clear to the user
    UD_ERROR_CHECK(udRenderContext_Render(info.pRenderContext, info.pRenderTarget, instances, instanceCount, &options));
    UD_ERROR_CHECK(RenderUDQuad(info));
  }

epilogue:
  lastError = result;
  return result;
}

// ----------------------------------------------------------------------------
// Author: Dave Pevreal, October 2023
// Handle selection 
bool ModelViewer::HandleSelection()
{
  if (sdlMouseButtonState & SDL_BUTTON(SDL_BUTTON_RIGHT))
  {
    if (selectState == SS_None || selectState == SS_Selected)
      selectState = SS_FirstPoint; // Go into mode looking for first point of selection

    if (pick.hit)
    {
      if (selectState == SS_FirstPoint)
      {
        selectPoint1 = *(udDouble3 *)pick.pointCenter;
        selectState = SS_SecondPoint;
      }
      if (selectState == SS_SecondPoint)
      {
        selectPoint2 = *(udDouble3 *)pick.pointCenter;
      }
    }
  }
  else if (selectState != SS_None && selectState != SS_Selected) // Mouse button is raised while selecting
  {
    if (selectState == SS_SecondPoint && selectPoint1 != selectPoint2)
      selectState = SS_Selected;
    else
      selectState = SS_None; // No valid point-pair was selected
    return selectState == SS_Selected; // Return true when selected for the first time
  }
  return false;
}

// ----------------------------------------------------------------------------
// Author: Dave Pevreal, October 2023
// Handle imgui events
bool ModelViewer::Event(udSampleRenderInfo &info, const SDL_Event &event)
{
  if (event.type == SDL_DROPFILE)
  {
    pFilename = udStrdup(event.drop.file);
    return true;
  }
  return false;
}
