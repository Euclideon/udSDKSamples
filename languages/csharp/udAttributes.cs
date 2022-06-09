using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Attributes
  {
    public enum StandardAttribute
    {
      GPSTime, //!< udATI_float64 
      PrimitiveID, //!< udATI_uint32  
      ARGB, //!< udATI_color32 
      Normal, //!< udATI_normal32
      Red, //!< Legacy 16bit Red channel
      Green, //!< Legacy 16bit Green channel
      Blue, //!< Legacy 16bit Blue channel
      Intensity, //!< udATI_uint16  
      NIR, //!< udATI_uint16  
      ScanAngle, //!< udATI_uint16  
      PointSourceID, //!< udATI_uint16  
      Classification, //!< udATI_uint8   
      ReturnNumber, //!< udATI_uint8   
      NumberOfReturns, //!< udATI_uint8   
      ClassificationFlags, //!< udATI_uint8   
      ScannerChannel, //!< udATI_uint8   
      ScanDirection, //!< udATI_uint8   
      EdgeOfFlightLine, //!< udATI_uint8   
      ScanAngleRank, //!< udATI_uint8   
      LasUserData, //!< Specific LAS User data field (udATI_uint8)

      Count, //!< Count helper value to iterate this enum
      AllAttributes = Count, //!< Internal sentinal value used by some functions to indicate getting start of interleaved attributes
      First = 0, //!< Generally used to initialise an attribute value for use in loops
    };

    public enum StandardAttributeContent
    {
      udSAC_None = (0),
      udSAC_GPSTime = (1 << StandardAttribute.GPSTime),
      udSAC_PrimitiveID = (1 << StandardAttribute.PrimitiveID),
      udSAC_ARGB = (1 << StandardAttribute.ARGB),
      udSAC_Normal = (1 << StandardAttribute.Normal),
      udSAC_Red = (1 << StandardAttribute.Red),
      udSAC_Green = (1 << StandardAttribute.Green),
      udSAC_Blue = (1 << StandardAttribute.Blue),
      udSAC_Intensity = (1 << StandardAttribute.Intensity),
      udSAC_NIR = (1 << StandardAttribute.NIR),
      udSAC_ScanAngle = (1 << StandardAttribute.ScanAngle),
      udSAC_PointSourceID = (1 << StandardAttribute.PointSourceID),
      udSAC_Classification = (1 << StandardAttribute.Classification),
      udSAC_ReturnNumber = (1 << StandardAttribute.ReturnNumber),
      udSAC_NumberOfReturns = (1 << StandardAttribute.NumberOfReturns),
      udSAC_ClassificationFlags = (1 << StandardAttribute.ClassificationFlags),
      udSAC_ScannerChannel = (1 << StandardAttribute.ScannerChannel),
      udSAC_ScanDirection = (1 << StandardAttribute.ScanDirection),
      udSAC_EdgeOfFlightLine = (1 << StandardAttribute.EdgeOfFlightLine),
      udSAC_ScanAngleRank = (1 << StandardAttribute.ScanAngleRank),
      udSAC_LasUserData = (1 << StandardAttribute.LasUserData),

      udSAC_AllAttributes = (1 << StandardAttribute.AllAttributes) - 1,
      udSAC_64BitAttributes = udSAC_GPSTime,
      udSAC_32BitAttributes = udSAC_PrimitiveID + udSAC_ARGB + udSAC_Normal,
      udSAC_16BitAttributes = udSAC_Intensity + udSAC_NIR + udSAC_ScanAngle + udSAC_PointSourceID,
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct udAttributeSet
    {
      public StandardAttributeContent standardContent; //!< Which standard attributes are available (used to optimize lookups internally), they still appear in the descriptors
      public uint count; //!< How many udAttributeDescriptor objects are used in pDescriptors
      public uint allocated; //!< How many udAttributeDescriptor objects are allocated to be used in pDescriptors
      public IntPtr pDescriptors; //!< this contains the actual information on the attributes
    };
  }
}
