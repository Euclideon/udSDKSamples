#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udCloudWorkspace.h"

int main(int argc, char **ppArgv)
{
  // This confirms that the static key have been configured
  static_assert(s_udCloudKey[0] != '\0', "udCloud key needs to be configured in udSDKFeatureSamples.h");

  // Define our variables
  udError udResult = udE_Success;
  udContext *pContext = nullptr;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, "udcloud.euclideon.com", "getSceneFromudCloud", nullptr, false) != udE_Success)
    udResult = udContext_ConnectWithKey(&pContext, "udcloud.euclideon.com", "getSceneFromudCloud", "1.0", s_udCloudKey);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  // Pull available Workspaces
  int workspaceCount = 0;
  udCloudWorkspace *pWorkspaces = nullptr;
  udCloudWorkspace_GetWorkspaceList(pContext, &pWorkspaces, &workspaceCount);

  if (workspaceCount == 0)
  {
    printf("No Workspace available for this account ....\n");
    udCloudWorkspace_ReleaseWorkspaceList(&pWorkspaces, workspaceCount);
    return 0;
  }

  int projectCount = 0;
  udCloudProject *pProjects = nullptr;
  udCloudWorkspace_GetProjectList(pContext, &pWorkspaces[0], &pProjects, &projectCount);

  if (projectCount == 0)
  {
    printf("No Project available for in this workspace ....\n");
    udCloudWorkspace_ReleaseWorkspaceList(&pWorkspaces, workspaceCount);
    udCloudWorkspace_ReleaseProjectList(&pProjects, projectCount);
    return 0;
  }

  int sceneCount = 0;
  udCloudScene *pScenes = nullptr;
  udCloudProject_GetSceneList(pContext, &pProjects[0], &pScenes, &sceneCount);

  for (int i = 0; i < sceneCount; ++i)
    printf("Scene name: %s created: %s\n", pScenes[i].pName, pScenes[i].pCreated);


  int fileCount = 0;
  udCloudFile *pFiles = nullptr;
  udCloudProject_GetFileList(pContext, &pProjects[0], &pFiles, &fileCount);

  for (int i = 0; i < fileCount; ++i)
    printf("File path: %s created: %s\n", pFiles[i].pPath, pFiles[i].pCreated);

  // Cleanup
  udCloudWorkspace_ReleaseWorkspaceList(&pWorkspaces, workspaceCount);
  udCloudWorkspace_ReleaseProjectList(&pProjects, projectCount);
  udCloudProject_ReleaseSceneList(&pScenes, sceneCount);
  udCloudProject_ReleaseFileList(&pFiles, fileCount);
  udContext_Disconnect(&pContext, false);

  return 0;
}
