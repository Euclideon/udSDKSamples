#include "udSample.h"
#include "ImGui.h"
#include "SDL.h"

#include "udStringUtil.h"

udSample *udSample::pSamplesHead;


// ----------------------------------------------------------------------------
// Base constructor that adds all samples into a common static list
udSample::udSample(const char *pSampleName) : pName(pSampleName)
{
  udQuadTexture = GL_INVALID_INDEX;
  quadPixelCount = 0;

  // Insert sample into the static list, sorted by name
  udSample **ppLink = &pSamplesHead;
  while (*ppLink && udStrcmpi((*ppLink)->pName, pName) < 0)
    ppLink = &(*ppLink)->pNextSample;
  pNextSample = *ppLink;
  (*ppLink) = this;
}

// ----------------------------------------------------------------------------
// Simple camera controls
void udSample::UpdateCamera(udDouble4x4 *pCamera, double dt, float &moveSpeed, float &turnSpeed)
{
  double yaw = 0, pitch = 0, tx = 0, ty = 0, tz = 0;

  udDouble4x4 rotation = *pCamera;
  udDouble3 pos = rotation.axis.t.toVector3();
  rotation.axis.t = udDouble4::identity();

  moveSpeed += ImGui::GetIO().MouseWheel * 25;

  if (ImGui::IsKeyDown(ImGuiKey_W))
    ty += moveSpeed * dt;
  if (ImGui::IsKeyDown(ImGuiKey_S))
    ty -= moveSpeed * dt;
  if (ImGui::IsKeyDown(ImGuiKey_A))
    tx -= moveSpeed * dt;
  if (ImGui::IsKeyDown(ImGuiKey_D))
    tx += moveSpeed * dt;

  if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
  {
    ImVec2 mouseDrag = ImGui::GetMouseDragDelta();
    yaw = mouseDrag.x / turnSpeed;
    pitch = mouseDrag.y / turnSpeed;
    ImGui::ResetMouseDragDelta();
  }

  if (yaw != 0.0)
    rotation = udDouble4x4::rotationZ(yaw) * rotation;   // Yaw on global axis
  if (pitch != 0.0)
    rotation = rotation * udDouble4x4::rotationX(pitch); // Pitch on local axis

  pos += rotation.axis.x.toVector3() * tx;
  pos += rotation.axis.y.toVector3() * ty;
  pos += rotation.axis.z.toVector3() * tz;
  rotation.axis.t = udDouble4::create(pos, 1.0);

  *pCamera = rotation;
};

// ----------------------------------------------------------------------------
// Default event handler
bool udSample::Event(udSampleRenderInfo &, const SDL_Event &)
{
  return false; // By default, signal to the caller that this sample doesn't handle the event
}

// ----------------------------------------------------------------------------
// Helper to render the UD quad
udError udSample::RenderUDQuad(udSampleRenderInfo &info)
{
  glBindTexture(GL_TEXTURE_2D, udQuadTexture);
  if (glGetError() != GL_NO_ERROR)
    return udE_InvalidConfiguration;
  if (quadPixelCount != (info.width * info.height))
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, info.width, info.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, info.pColorBuffer);
    quadPixelCount = (info.width * info.height); // Not strictly necessary but useful for cases where window can be resized
  }
  else
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, info.width, info.height, GL_BGRA, GL_UNSIGNED_BYTE, info.pColorBuffer);
  }
  if (glGetError() != GL_NO_ERROR)
    return udE_InvalidConfiguration;

  glEnable(GL_TEXTURE_2D);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(-1, 1);

  glTexCoord2f(1, 0);
  glVertex2f(1, 1);

  glTexCoord2f(1, 1);
  glVertex2f(1, -1);

  glTexCoord2f(0, 1);
  glVertex2f(-1, -1);
  glEnd();
  if (glGetError() != GL_NO_ERROR)
    return udE_InvalidConfiguration;
  return udE_Success;
}


// ----------------------------------------------------------------------------
// Helper to deinitialise data related to rendering the UD quad
void udSample::DeinitUDQuad()
{
  if (udQuadTexture != GL_INVALID_INDEX)
  {
    glDeleteTextures(1, &udQuadTexture);
    udQuadTexture = GL_INVALID_INDEX;
  }
  quadPixelCount = 0;
}
