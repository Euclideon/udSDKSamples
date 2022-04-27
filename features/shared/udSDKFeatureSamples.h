#ifndef VaultSDKFeatureSamples_h__
#define VaultSDKFeatureSamples_h__
#include "udContext.h"
#include "string.h"
#include "stdio.h"
// Euclideon does NOT recommend storing user credentials in this format
// This is only done to simplify the feature demonstrations

constexpr char s_udStreamServer[] = "https://udstream.euclideon.com";
constexpr char s_udStreamEmail[] = "";
constexpr char s_udStreamPassword[] = "";

udContext *g_pContext = nullptr;
const char *g_pUsername = "";
const char *g_pServerPath = "https://udcloud.euclideon.com";
int g_legacyServer = 0;


inline udError BasicParseLogin(int argc, char **ppArgv)
{
  const char *pPassword = "";
  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(ppArgv[i], "-u") == 0 && i + 1 < argc)
      g_pUsername = ppArgv[++i]; //not required for udCloud based servers, but does allow for resuming sessions
    else if (strcmp(ppArgv[i], "-p") == 0 && i + 1 < argc)
      pPassword = ppArgv[++i]; // not required for udCloud
    else if (strcmp(ppArgv[i], "-s") == 0 && i + 1 < argc)
      g_pServerPath = ppArgv[++i];
    else if (strcmp(ppArgv[i], "-legacyServer") == 0)
      g_legacyServer = 1;
  }

  udError result = udE_Failure;

  result = udContext_TryResume(&g_pContext, g_pServerPath, ppArgv[0], g_pUsername, 0); // Set to 1 to try use the dongle (doesn't work in debug)

  if (result != udE_Success)
  {
    if (g_legacyServer)
    {
      result = udContext_ConnectLegacy(&g_pContext, g_pServerPath, ppArgv[0], g_pUsername, pPassword);
    }
    else
    {
      udContextPartial *pPartialConnection = nullptr;
      const char *pApprovePath = nullptr;
      const char *pApproveCode = nullptr;
      result = udContext_ConnectStart(&pPartialConnection, g_pServerPath, ppArgv[0], "0.1", &pApprovePath, &pApproveCode);
      if (pApprovePath != nullptr)
      {
        printf("visit \"%s\" on this device to complete connection\nAlternatively visit %s/link on any device and enter %s\nPress Enter to continue...\n", pApprovePath, g_pServerPath, pApproveCode);
        while (getc(stdin) != '\n');
        result = udContext_ConnectComplete(&g_pContext, &pPartialConnection);
      }

      if (pPartialConnection != nullptr)
      {
        udContext_ConnectCancel(&pPartialConnection);
      }
    }
  }
  return result;
}

// Some helper macro's so that the sample code can remain fairly tidy
#define ExitWithMessage(resultCode, pMsg) do { printf("[" __FILE__ ":%d] %s", __LINE__, pMsg); exit(resultCode); } while(false)

#endif //VaultSDKFeatureSamples_h__
