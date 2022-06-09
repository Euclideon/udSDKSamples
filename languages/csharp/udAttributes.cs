using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Attributes
  {
    public enum StandardAttribute
    {
      GPSTime,
      ARGB,
      Normal,
      Intensity,
      NIR,
      ScanAngle,
      PointSourceID,
      Classification,
      ReturnNumber,
      NumberOfReturns,
      ClassificationFlags,
      ScannerChannel,
      ScanDirection,
      EdgeOfFlightLine,
      ScanAngleRank,
      LASUserData,

      Count
    };

    public enum StandardAttributeContent
    {
      None = (0),
      GPSTime = (1 << StandardAttribute.GPSTime),
      ARGB = (1 << StandardAttribute.ARGB),
      Normal = (1 << StandardAttribute.Normal),
      Intensity = (1 << StandardAttribute.Intensity),
      NIR = (1 << StandardAttribute.NIR),
      ScanAngle = (1 << StandardAttribute.ScanAngle),
      PointSourceID = (1 << StandardAttribute.PointSourceID),
      Classification = (1 << StandardAttribute.Classification),
      ReturnNumber = (1 << StandardAttribute.ReturnNumber),
      NumberOfReturns = (1 << StandardAttribute.NumberOfReturns),
      ClassificationFlags = (1 << StandardAttribute.ClassificationFlags),
      ScannerChannel = (1 << StandardAttribute.ScannerChannel),
      ScanDirection = (1 << StandardAttribute.ScanDirection),
      EdgeOfFlightLine = (1 << StandardAttribute.EdgeOfFlightLine),
      ScanAngleRank = (1 << StandardAttribute.ScanAngleRank),
      LasUserData = (1 << StandardAttribute.LASUserData),
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct udAttributeSet
    {
      public Attributes.StandardAttributeContent standardContent; //!< Which standard attributes are available (used to optimize lookups internally), they still appear in the descriptors
      public uint count; //!< How many udAttributeDescriptor objects are used in pDescriptors
      public uint allocated; //!< How many udAttributeDescriptor objects are allocated to be used in pDescriptors
      public IntPtr pDescriptors; //!< this contains the actual information on the attributes
    };
  }
}
