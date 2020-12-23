#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "stb_image.h"

#include "udSDKFeatureSamples.h"
#include "udContext.h"
#include "udConvertCustom.h"
#include "udTriangleVoxelizer.h"

#include "udGeoZone.h"
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

  //Voxeliser
  udTriangleVoxelizer *pTriVox;
  int32_t indexX; // could be changed to a single index
  int32_t indexY;
  int8_t triIndex;

  double outputResolution;
};

udError DEMConvertTest_Open(struct udConvertCustomItem * pConvertInput, uint32_t /*everyNth*/, const double /*origin*/[3], double pointResolution, enum udConvertCustomItemFlags /*flags*/)
{
  DEMConvert *pItem = (DEMConvert *)pConvertInput->pData;
  pItem->outputResolution = pointResolution;

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
    double completed = (double)pItem->indexX / ((pItem->columns - 2) * (pItem->rows - 1)) + (double)pItem->indexY / (pItem->rows - 1);
    printf("\rConverting... %.2f%% - X:%5d/%d Y:%5d/%d (%u points)", 100.0 * completed, pItem->indexX, (pItem->columns - 2), pItem->indexY, (pItem->rows - 1), pItem->pointsWritten);

    for (int32_t i = pBuffer->pointCount; pBuffer->pointCount < pBuffer->pointsAllocated && pItem->indexY < pItem->rows - 1; ++i)
    {
      udDouble3 corners[4];
      corners[0] = pItem->pPoints[(pItem->indexX + 0) * pItem->rows + (pItem->indexY + 0)];
      corners[1] = pItem->pPoints[(pItem->indexX + 0) * pItem->rows + (pItem->indexY + 1)];
      corners[2] = pItem->pPoints[(pItem->indexX + 1) * pItem->rows + (pItem->indexY + 0)];
      corners[3] = pItem->pPoints[(pItem->indexX + 1) * pItem->rows + (pItem->indexY + 1)];
      const double p0[3] = { corners[0].x, corners[0].y, corners[0].z };
      const double p1[3] = { corners[1].x, corners[1].y, corners[1].z };
      const double p2[3] = { corners[2].x, corners[2].y, corners[2].z };
      const double p3[3] = { corners[3].x, corners[3].y, corners[3].z };

      udDouble2 p0UV, p1UV, p2UV;
      p0UV = udDouble2::create(double(pItem->indexX) / double(pItem->columns), double(pItem->indexY) / double(pItem->rows));

      if (pItem->triIndex == 0)
      {
        udTriangleVoxelizer_SetTriangle(pItem->pTriVox, p0, p2, p3);
        p1UV = udDouble2::create(double(pItem->indexX + 1) / double(pItem->columns), double(pItem->indexY) / double(pItem->rows));
        p2UV = udDouble2::create(double(pItem->indexX + 1) / double(pItem->columns), double(pItem->indexY + 1) / double(pItem->rows));
      }
      else
      {
        udTriangleVoxelizer_SetTriangle(pItem->pTriVox, p0, p3, p1);
        p1UV = udDouble2::create(double(pItem->indexX + 1) / double(pItem->columns), double(pItem->indexY + 1) / double(pItem->rows));
        p2UV = udDouble2::create(double(pItem->indexX) / double(pItem->columns), double(pItem->indexY + 1) / double(pItem->rows));
      }
      
      double *pPointsPosition = nullptr;
      double *pPointsBWeights = nullptr;
      uint32_t pointsCount = UINT32_MAX;
      uint32_t maxPoints = (pBuffer->pointsAllocated > pBuffer->pointCount) ? pBuffer->pointsAllocated - pBuffer->pointCount : 0;
      while (pointsCount != 0 && maxPoints > 0)
      {
        udTriangleVoxelizer_GetPoints(pItem->pTriVox, &pPointsPosition, &pPointsBWeights, &pointsCount, pBuffer->pointsAllocated - pBuffer->pointCount);
        for (uint32_t point = 0; point < pointsCount; ++point)
        {
          udDouble2 UV = udClamp(p0UV * pPointsBWeights[point * 3 + 0] + p1UV * pPointsBWeights[point * 3 + 1] + p2UV * pPointsBWeights[point * 3 + 2], udDouble2::zero(), udDouble2::one());
          int imageIndex = int(UV.x * (pItem->width - 1.0) + int((1.0 - UV.y) * (pItem->height - 1.0)) * double(pItem->width));

          pBuffer->pAttributes[pBuffer->pointCount * pBuffer->attributeStride + 0] = pItem->pImage[imageIndex * 3 + 2];
          pBuffer->pAttributes[pBuffer->pointCount * pBuffer->attributeStride + 1] = pItem->pImage[imageIndex * 3 + 1];
          pBuffer->pAttributes[pBuffer->pointCount * pBuffer->attributeStride + 2] = pItem->pImage[imageIndex * 3 + 0];

          pBuffer->pPositions[pBuffer->pointCount * 3 + 0] = pPointsPosition[point * 3 + 0];
          pBuffer->pPositions[pBuffer->pointCount * 3 + 1] = pPointsPosition[point * 3 + 1];
          pBuffer->pPositions[pBuffer->pointCount * 3 + 2] = pPointsPosition[point * 3 + 2];

          ++pItem->pointsTotalOut;
          ++pItem->pointsWritten;
          ++pBuffer->pointCount;
        }
        maxPoints = (pBuffer->pointsAllocated > pBuffer->pointCount) ? pBuffer->pointsAllocated - pBuffer->pointCount : 0;
      }

      if (pBuffer->pointsAllocated - pBuffer->pointCount != 0)
      {
        pItem->triIndex = pItem->triIndex == 0 ? 1 : 0; // next poly
        if (pItem->triIndex == 0 && pItem->indexX < pItem->columns - 2) // next square
        {
          ++pItem->indexX;
        }
        else if (pItem->triIndex == 0) // new line
        {
          ++pItem->indexY;
          pItem->indexX = 0;
        }
      }
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

  if (argc < 4)
  {
    pIData = stbi_load(ppArgv[2], &width, &height, &channels, 3);
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
  udDouble2 resolution = {0, 0};

  double gridRes = 1.0;
  udTriangleVoxelizer *pTriVox;

  int numberOfCols = 0;
  int expectedRows = 0;

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

  // Setup the custom item
  udConvertCustomItem item = {};
  DEMConvert itemInfo = {};

  // Convert to Cartesian
  int srid;
  udGeoZone_FindSRID(&srid, pPoints[0], true);
  item.srid = srid;

  udGeoZone zone;
  udGeoZone_SetFromSRID(&zone, srid);
  for (int i = 0; i < numberOfCols * expectedRows; ++i)
    pPoints[i] = udGeoZone_LatLongToCartesian(zone, pPoints[i], true);

  itemInfo.pPoints = pPoints;
  itemInfo.columns = numberOfCols;
  itemInfo.rows = expectedRows;

  itemInfo.pImage = pIData;
  itemInfo.width = width;
  itemInfo.height = height;

  if (pIData != nullptr)
    gridRes = udMin(30.0 * udMag2(cornerSE - cornerNE) / height, 30.0 * udMag2(cornerNE - cornerNW) / width);
  else
    gridRes = 10.0;

  printf("Converting at %.3fm resolution\n", gridRes);

  udTriangleVoxelizer_Create(&pTriVox, gridRes);

  itemInfo.indexX = 0;
  itemInfo.indexY = 0;
  itemInfo.triIndex = 0;
  itemInfo.pTriVox = pTriVox;

  item.pOpen = DEMConvertTest_Open;
  item.pReadPointsFloat = DEMConvertTest_ReadFloat;
  item.pDestroy = DEMConvertTest_Destroy;
  item.pClose = DEMConvertTest_Close;

  item.pData = &itemInfo;

  item.pName = ppArgv[1];
  item.boundsKnown = false;
  item.pointCount = ((pIData == nullptr) ? (numberOfCols * expectedRows) : (width * height));
  item.pointCountIsEstimate = true;// false;
  item.sourceProjection = udCSP_SourceCartesian;
  item.sourceResolution = gridRes;

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
  udTriangleVoxelizer_Destroy(&pTriVox);
  udConvert_DestroyContext(&pConvertCtx);
  udContext_Disconnect(&pContext, false);

  udFree(pPoints);
  stbi_image_free(pIData);

  return 0;
}
