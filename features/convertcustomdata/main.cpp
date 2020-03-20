#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "VaultSDKFeatureSamples.h"
#include "vdkContext.h"
#include "vdkConvertCustom.h"

const char s_SampleName[] = "SampleCustomConvert";

const int Size = 256;

struct CustomConvert
{
  uint32_t pointsWritten;
};

vdkError CustomConvertTest_Open(struct vdkConvertCustomItem * /*pConvertInput*/, uint32_t /*everyNth*/, const double /*origin*/[3], double /*pointResolution*/, enum vdkConvertCustomItemFlags /*flags*/)
{
  return vE_Success;
}

vdkError CustomConvertTest_ReadFloat(struct vdkConvertCustomItem *pConvertInput, struct vdkPointBufferF64 *pBuffer)
{
  CustomConvert *pItem = (CustomConvert *)pConvertInput->pData;

  pBuffer->pointCount = 0;

  for (uint32_t i = pBuffer->pointCount; i < pBuffer->pointsAllocated && pItem->pointsWritten < Size * Size; ++i)
  {
    uint32_t xPos = (pItem->pointsWritten % Size);
    uint32_t yPos = (pItem->pointsWritten / Size);

    uint8_t red = (uint8_t)xPos;
    uint8_t green = (uint8_t)yPos;
    uint8_t blue = red/2+green/2;

    pBuffer->pAttributes[i * pBuffer->attributeStride + 0] = red;
    pBuffer->pAttributes[i * pBuffer->attributeStride + 1] = green;
    pBuffer->pAttributes[i * pBuffer->attributeStride + 2] = blue;

    pBuffer->pAttributes[i * pBuffer->attributeStride + 4] = (uint8_t)(pItem->pointsWritten >> 8);
    pBuffer->pAttributes[i * pBuffer->attributeStride + 5] = (uint8_t)(pItem->pointsWritten & 0xFF);

    pBuffer->pPositions[i * 3 + 0] = (xPos * 0.3);
    pBuffer->pPositions[i * 3 + 1] = (yPos * 0.3);
    pBuffer->pPositions[i * 3 + 2] = 0.0; // This will make a flat plane

    // Or to do something a little fancier uncomment this:
    //pBuffer->pPositions[i * 3 + 2] = sin(xPos / (double)(Size * 5.0));

    ++pItem->pointsWritten;
    ++pBuffer->pointCount;
  }

  return vE_Success;
}

void CustomConvertTest_Destroy(struct vdkConvertCustomItem *pConvertInput)
{
  vdkAttributeSet_Free(&pConvertInput->attributes);
}

void CustomConvertTest_Close(struct vdkConvertCustomItem * /*pConvertInput*/)
{
  //Do nothing
}

int main(int argc, char **ppArgv)
{
  // This confirms that the statics have been configured correctly
  static_assert(s_VaultUsername[0] != '\0', "Username needs to be configured in VaultSDKFeatureSamples.h");
  static_assert(s_VaultPassword[0] != '\0', "Password needs to be configured in VaultSDKFeatureSamples.h");

  // Define our variables
  vdkError vdkResult = vE_Success;
  vdkContext *pContext = nullptr;

  // Resume Session or Login
  if (vdkContext_TryResume(&pContext, s_VaultServer, s_SampleName, s_VaultUsername, false) != vE_Success)
    vdkResult = vdkContext_Connect(&pContext, s_VaultServer, s_SampleName, s_VaultUsername, s_VaultPassword);

  if (vdkResult != vE_Success)
    ExitWithMessage(vdkResult, "Could not login!");

  // Initialise Convert State
  vdkConvertContext *pConvertCtx = nullptr;
  if (vdkConvert_CreateContext(pContext, &pConvertCtx) != vE_Success)
    ExitWithMessage(1, "Could not create convert context!");

  vdkConvert_SetOutputFilename(pConvertCtx, "ConvertCustom.uds");

  // Setup the custom item
  vdkConvertCustomItem item = {};
  CustomConvert itemInfo = {};

  item.pOpen = CustomConvertTest_Open;
  item.pReadPointsFloat = CustomConvertTest_ReadFloat;
  item.pDestroy = CustomConvertTest_Destroy;
  item.pClose = CustomConvertTest_Close;

  item.pData = &itemInfo;

  item.pName = "Custom Item";
  item.boundsKnown = false;
  item.pointCount = Size * Size;
  item.pointCountIsEstimate = false;

  item.sourceResolution = 0.1;
  vdkAttributeSet_Generate(&item.attributes, vdkSAC_ARGB | vdkSAC_Intensity | vdkSAC_Classification, 0);

  // Do the actual conversion
  if (vdkConvert_AddCustomItem(pConvertCtx, &item) != vE_Success)
    ExitWithMessage(1, "Could not add custom convert item!");

  // Start the conversion
  if (vdkConvert_DoConvert(pConvertCtx) != vE_Success)
    ExitWithMessage(1, "Conversion failed!");

  // Cleanup
  vdkConvert_DestroyContext(&pConvertCtx);
  vdkContext_Disconnect(&pContext);

  return 0;
}
