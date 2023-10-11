#include "udSample.h"
#include "ImGui.h"
#include "SDL.h"

std::vector<udSample *> udSample::samples;

void udSample::UpdateCamera(udDouble4x4 *pCamera, double dt, double &moveSpeed, double &turnSpeed)
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

udError udSample::Event(udSampleRenderInfo &, const SDL_Event &)
{
  return udE_NothingToDo; // By default, signal to the caller that this sample doesn't handle the event
}
