
using System;
using System.Runtime.InteropServices;
namespace Euclideon.udSDK
{
  namespace MathTypes
  {
    //!
    //! @struct udMathDouble2
    //! A 2D geometric vector with double precision
    //! 
    public struct udMathDouble2
    {
      public double x; //!< The x coordinate
      public double y; //!< The y coordinate
    };

    //!
    //! @struct udMathLong2
    //! A 2D geometric vector with 64bit integers
    //! 
    public struct udMathLong2
    {
      public Int64 x; //!< The x coordinate
      public Int64 y; //!< The y coordinate
    };

    //!
    //! @struct udMathULong2
    //! A 2D geometric vector with 64bit unsigned integers
    //! 
    public struct udMathULong2
    {
      public UInt64 x; //!< The x coordinate
      public UInt64 y; //!< The y coordinate
    };

    //!
    //! @struct udMathDouble3
    //! A 3D geometric vector with double precision
    //! 
    public struct udMathDouble3
    {
      public double x; //!< The x coordinate
      public double y; //!< The y coordinate
      public double z; //!< The z coordinate
    };

    //!
    //! @struct udMathLong3
    //! A 3D geometric vector with 64bit integers
    //! 
    public struct udMathLong3
    {
      public Int64 x; //!< The x coordinate
      public Int64 y; //!< The y coordinate
      public Int64 z; //!< The z coordinate
    };

    //!
    //! @struct udMathULong3
    //! A 3D geometric vector with 64bit unsigned integers
    //! 
    public struct udMathULong3
    {
      public UInt64 x; //!< The x coordinate
      public UInt64 y; //!< The y coordinate
      public UInt64 z; //!< The z coordinate
    };

    //!
    //! @struct udMathDouble4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with double precision
    //! 
    public struct udMathDouble4
    {
      public double x; //!< The x coordinate
      public double y; //!< The y coordinate
      public double z; //!< The z coordinate
      public double w; //!< The w coordinate
    };

    //!
    //! @struct udMathLong4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with 64bit integers
    //! 
    public struct udMathLong4
    {
      public Int64 x; //!< The x coordinate
      public Int64 y; //!< The y coordinate
      public Int64 z; //!< The z coordinate
      public Int64 w; //!< The w coordinate
    };

    //!
    //! @struct udMathULong4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with 64bit unsigned integers
    //! 
    public struct udMathULong4
    {
      public UInt64 x; //!< The x coordinate
      public UInt64 y; //!< The y coordinate
      public UInt64 z; //!< The z coordinate
      public UInt64 w; //!< The w coordinate
    };

    //!
    //! @struct udMathDouble4x4
    //! A 4x4 geometric matrix with double precision
    //! 
    public struct udMathDouble4x4
    {
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =16)]
      public double[] array; //!< The matrix elements storing the 4 values for each axis in this order: x axis, y axis, z axis, t axis
    };
  }
}

