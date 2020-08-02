#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udConvertCustom.h"

const char s_SampleName[] = "SampleCustomConvert";

const int Size = 256;

struct CustomConvert
{
  uint32_t pointsWritten;
};

udError CustomConvertTest_Open(struct udConvertCustomItem * /*pConvertInput*/, uint32_t /*everyNth*/, const double /*origin*/[3], double /*pointResolution*/, enum udConvertCustomItemFlags /*flags*/)
{
  return udE_Success;
}

udError CustomConvertTest_ReadFloat(struct udConvertCustomItem *pConvertInput, struct udPointBufferF64 *pBuffer)
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

  return udE_Success;
}

void CustomConvertTest_Destroy(struct udConvertCustomItem *pConvertInput)
{
  udAttributeSet_Destroy(&pConvertInput->attributes);
}

void CustomConvertTest_Close(struct udConvertCustomItem * /*pConvertInput*/)
{
  //Do nothing
}

int main(int argc, char **ppArgv)
{
  // This confirms that the statics have been configured correctly
  static_assert(s_udStreamEmail[0] != '\0', "Email needs to be configured in VaultSDKFeatureSamples.h");
  static_assert(s_udStreamPassword[0] != '\0', "Password needs to be configured in VaultSDKFeatureSamples.h");

  // Define our variables
  udError udResult = udE_Success;
  udContext *pContext = nullptr;

  // Resume Session or Login
  if (udContext_TryResume(&pContext, s_udStreamServer, s_SampleName, s_udStreamEmail, false) != udE_Success)
    udResult = udContext_Connect(&pContext, s_udStreamServer, s_SampleName, s_udStreamEmail, s_udStreamPassword);

  if (udResult != udE_Success)
    ExitWithMessage(udResult, "Could not login!");

  // Initialise Convert State
  udConvertContext *pConvertCtx = nullptr;
  if (udConvert_CreateContext(pContext, &pConvertCtx) != udE_Success)
    ExitWithMessage(1, "Could not create convert context!");

  udConvert_SetOutputFilename(pConvertCtx, "ConvertCustom.uds");

  // Setup the custom item
  udConvertCustomItem item = {};
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
  udAttributeSet_Create(&item.attributes, udSAC_ARGB | udSAC_Intensity | udSAC_Classification, 0);

  // Do the actual conversion
  if (udConvert_AddCustomItem(pConvertCtx, &item) != udE_Success)
    ExitWithMessage(1, "Could not add custom convert item!");

  // Start the conversion
  if (udConvert_DoConvert(pConvertCtx) != udE_Success)
    ExitWithMessage(1, "Conversion failed!");

  // Cleanup
  udConvert_DestroyContext(&pConvertCtx);
  udContext_Disconnect(&pContext, false);

  return 0;
}
