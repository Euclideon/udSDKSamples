#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "stb_image.h"

#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udConvertCustom.h"

// Format available here https://en.wikipedia.org/wiki/USGS_DEM

// Please note: This sample project uses the udCore helper library
#include "udFile.h"
#include "udStringUtil.h"
#include "udMath.h"

const char s_SampleName[] = "ColourDEMConvert";

struct DEMConvert
{
  int32_t pointsWritten;
  int32_t pointsTotalOut;

  // DEM Data
  udDouble3 *pPoints;
  int32_t columns;
  int32_t rows;

  // Image Data
  uint8_t *pImage;
  int32_t width;
  int32_t height;
};

udError DEMConvertTest_Open(struct udConvertCustomItem * /*pConvertInput*/, uint32_t /*everyNth*/, const double /*origin*/[3], double /*pointResolution*/, enum udConvertCustomItemFlags /*flags*/)
{
  return udE_Success;
}

udError DEMConvertTest_ReadFloat(struct udConvertCustomItem *pConvertInput, struct udPointBufferF64 *pBuffer)
{
  DEMConvert *pItem = (DEMConvert *)pConvertInput->pData;

  pBuffer->pointCount = 0;

  if (pItem->pImage == nullptr) //DEM Only
  {
    for (uint32_t i = pBuffer->pointCount; i < pBuffer->pointsAllocated && pItem->pointsWritten < pItem->columns * pItem->rows; ++i)
    {
      uint8_t red = 0;
      uint8_t green = 255;
      uint8_t blue = 255;

      pBuffer->pAttributes[i * pBuffer->attributeStride + 0] = red;
      pBuffer->pAttributes[i * pBuffer->attributeStride + 1] = green;
      pBuffer->pAttributes[i * pBuffer->attributeStride + 2] = blue;

      int px = (pItem->pointsWritten % pItem->rows);
      int py = (pItem->pointsWritten / pItem->rows);

      pBuffer->pPositions[i * 3 + 0] = pItem->pPoints[pItem->pointsWritten].x;
      pBuffer->pPositions[i * 3 + 1] = pItem->pPoints[pItem->pointsWritten].y;
      pBuffer->pPositions[i * 3 + 2] = pItem->pPoints[pItem->pointsWritten].z;

      ++pItem->pointsTotalOut;
      ++pItem->pointsWritten;
      ++pBuffer->pointCount;
    }
  }
  else
  {
    printf("\rConverting... %.2f %u/%u", (double)pItem->pointsWritten / (pItem->width * pItem->height), pItem->pointsWritten, pItem->width * pItem->height);

    for (uint32_t i = pBuffer->pointCount; i < pBuffer->pointsAllocated && pItem->pointsWritten < pItem->width * pItem->height; ++i)
    {
      int px = (pItem->pointsWritten % pItem->width);
      int py = pItem->height - (pItem->pointsWritten / pItem->width) - 1;
      double imageX = (pItem->columns-1) * px / double(pItem->width);
      double imageY = (pItem->rows-1) * py / double(pItem->height);

      double yIntF, xIntF;
      double xFrac = modf(imageX, &xIntF);
      double yFrac = modf(imageY, &yIntF);

      int yInt = (int)yIntF;
      int xInt = (int)xIntF;

      while ((yInt+1) >= pItem->rows)
      {
        yFrac = 1.0;
        yInt = pItem->rows - 2;
      }

      while ((xInt + 1) >= pItem->columns)
      {
        xFrac = 1.0;
        xInt = pItem->columns - 2;
      }

      pBuffer->pAttributes[i * pBuffer->attributeStride + 0] = pItem->pImage[pItem->pointsWritten * 3 + 0];
      pBuffer->pAttributes[i * pBuffer->attributeStride + 1] = pItem->pImage[pItem->pointsWritten * 3 + 1];
      pBuffer->pAttributes[i * pBuffer->attributeStride + 2] = pItem->pImage[pItem->pointsWritten * 3 + 2];

      udDouble3 corners[4];
      corners[0] = pItem->pPoints[(xInt + 0) * pItem->rows + (yInt + 0)];
      corners[1] = pItem->pPoints[(xInt + 0) * pItem->rows + (yInt + 1)];
      corners[2] = pItem->pPoints[(xInt + 1) * pItem->rows + (yInt + 0)];
      corners[3] = pItem->pPoints[(xInt + 1) * pItem->rows + (yInt + 1)];

      udDouble3 position = udLerp(udLerp(corners[0], corners[2], xFrac), udLerp(corners[1], corners[3], xFrac), yFrac);

      pBuffer->pPositions[i * 3 + 0] = position.x;
      pBuffer->pPositions[i * 3 + 1] = position.y;
      pBuffer->pPositions[i * 3 + 2] = position.z;

      ++pItem->pointsTotalOut;
      ++pItem->pointsWritten;
      ++pBuffer->pointCount;
    }
  }

  return udE_Success;
}

void DEMConvertTest_Destroy(struct udConvertCustomItem *pConvertInput)
{
  udAttributeSet_Destroy(&pConvertInput->attributes);
}

void DEMConvertTest_Close(struct udConvertCustomItem * /*pConvertInput*/)
{
  //Do nothing
}

int main(int argc, char **ppArgv)
{
  // This confirms that the statics have been configured correctly
  static_assert(s_udStreamEmail[0] != '\0', "Email needs to be configured in VaultSDKFeatureSamples.h");
  static_assert(s_udStreamPassword[0] != '\0', "Password needs to be configured in VaultSDKFeatureSamples.h");

  int width = 0;
  int height = 0;
  int channels = 0;

  if (argc < 2)
    ExitWithMessage(0, "Need the parameter for the DEM file!");

  uint8_t *pIData = nullptr;

  if (argc < 3)
  {
    stbi_load(ppArgv[2], &width, &height, &channels, 3);
    printf("Image Loaded %dx%d\n", width, height);
  }

  // Process the DEM data
  const char *pData = nullptr;

  size_t numPoints = 0;
  udDouble3 *pPoints = nullptr;
  udDouble2 cornerSW;
  udDouble2 cornerNW;
  udDouble2 cornerNE;
  udDouble2 cornerSE;
  udDouble2 elevationLimits;
  udDouble2 resolution;

  int numberOfCols;
  int expectedRows;

  if (udFile_Load(ppArgv[1], &pData) == udR_Success)
  {
    int coordSystem = udStrAtoi(&pData[156]);
    int utmZone = udStrAtoi(&pData[162]);
    int unitsH = udStrAtoi(&pData[529]); // 0=radian;1=feet;2=metre;3=arc-second
    int unitsV = udStrAtoi(&pData[535]); // 1=feet;2=metre

    if (unitsH != 3)
      ExitWithMessage(udE_NotSupported, "Only Arc-Seconds is supported!");

    cornerSW = udDouble2::create(udStrAtof64(&pData[546]), udStrAtof64(&pData[570]));
    cornerNW = udDouble2::create(udStrAtof64(&pData[594]), udStrAtof64(&pData[618]));
    cornerNE = udDouble2::create(udStrAtof64(&pData[642]), udStrAtof64(&pData[666]));
    cornerSE = udDouble2::create(udStrAtof64(&pData[690]), udStrAtof64(&pData[714]));
    elevationLimits = udDouble2::create(udStrAtof64(&pData[738]), udStrAtof64(&pData[762]));
    resolution = udDouble2::create(udStrAtof64(&pData[816]), udStrAtof64(&pData[828]));

    numberOfCols = udStrAtoi(&pData[858]);
    expectedRows = udAbs((int)udRound((cornerNW.y - cornerSW.y) / resolution.y)) + 1;

    numPoints = numberOfCols * expectedRows;
    pPoints = udAllocType(udDouble3, numPoints, udAF_Zero);

    size_t read = 1024;
    int count = 0;

    for (int i = 0; i < numberOfCols; ++i) // Loop over Latitude (South->North)
    {
      int colIndex = udStrAtoi(&pData[read], &count); read += count;
      int rowIndex = udStrAtoi(&pData[read], &count); read += count;

      int mIndex = udStrAtoi(&pData[read], &count); read += count;
      int nIndex = udStrAtoi(&pData[read], &count); read += count;

      double xPos = udStrAtof64(&pData[read], &count); read += count;
      double yPos = udStrAtof64(&pData[read], &count); read += count;

      double elevationDatum = udStrAtof64(&pData[read], &count); read += count;

      double minZ = udStrAtof64(&pData[read], &count); read += count;
      double maxZ = udStrAtof64(&pData[read], &count); read += count;

      double XRun = cornerSW.x + ((cornerSE.x - cornerSW.x) * rowIndex / numberOfCols);
      double yRun = cornerSE.y + ((cornerNE.y - cornerSE.y) * rowIndex / numberOfCols);

      for (int j = 0; j < mIndex; ++j) // Loop over longitude (West->East)
      {
        // 0 < j < 226 (Longitude)
        // 0 < i < 301 (Latitude)
        // pPoints is LongLat order


        int elevation = udStrAtoi(&pData[read], &count); read += count;
        pPoints[i * mIndex + j] = udDouble3::create(xPos / 3600.0, (yPos + resolution.y * j) / 3600.0, elevation / 10);
      }
    }

    udFree(pData);
  }

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

  udConvert_SetOutputFilename(pConvertCtx, "C:/datasets/DEM/dem.uds");

  // Setup the custom item
  udConvertCustomItem item = {};
  DEMConvert itemInfo = {};

  itemInfo.pPoints = pPoints;
  itemInfo.columns = numberOfCols;
  itemInfo.rows = expectedRows;

  itemInfo.pImage = pIData;
  itemInfo.width = width;
  itemInfo.height = height;

  item.pOpen = DEMConvertTest_Open;
  item.pReadPointsFloat = DEMConvertTest_ReadFloat;
  item.pDestroy = DEMConvertTest_Destroy;
  item.pClose = DEMConvertTest_Close;

  item.pData = &itemInfo;

  item.pName = "DEM Item";
  item.boundsKnown = false;
  item.pointCount = ((pIData == nullptr) ? (numberOfCols * expectedRows) : (width * height));
  item.pointCountIsEstimate = true;// false;
  item.sourceProjection = udCSP_SourceLongLat;
  item.sourceResolution = 2.0;

  udAttributeSet_Create(&item.attributes, udSAC_ARGB, 0);

  // Do the actual conversion
  if (udConvert_AddCustomItem(pConvertCtx, &item) != udE_Success)
    ExitWithMessage(1, "Could not add custom convert item!");

  const udConvertInfo *pInfo;
  udConvert_GetInfo(pConvertCtx, &pInfo);
  printf("Detected SRID: %d\n", pInfo->srid);

  // Start the conversion
  if (udConvert_DoConvert(pConvertCtx) != udE_Success)
    ExitWithMessage(1, "Conversion failed!");

  printf("\nOutput %u points\n", itemInfo.pointsTotalOut);

  // Cleanup
  udConvert_DestroyContext(&pConvertCtx);
  udContext_Disconnect(&pContext, false);

  udFree(pPoints);
  stbi_image_free(pIData);

  return 0;
}
