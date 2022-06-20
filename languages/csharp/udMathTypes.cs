
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
    struct udMathDouble2
    {
      double x; //!< The x coordinate
      double y; //!< The y coordinate
    };

    //!
    //! @struct udMathLong2
    //! A 2D geometric vector with 64bit integers
    //! 
    struct udMathLong2
    {
      Int64 x; //!< The x coordinate
      Int64 y; //!< The y coordinate
    };

    //!
    //! @struct udMathULong2
    //! A 2D geometric vector with 64bit unsigned integers
    //! 
    struct udMathULong2
    {
      UInt64 x; //!< The x coordinate
      UInt64 y; //!< The y coordinate
    };

    //!
    //! @struct udMathDouble3
    //! A 3D geometric vector with double precision
    //! 
    struct udMathDouble3
    {
      double x; //!< The x coordinate
      double y; //!< The y coordinate
      double z; //!< The z coordinate
    };

    //!
    //! @struct udMathLong3
    //! A 3D geometric vector with 64bit integers
    //! 
    struct udMathLong3
    {
      Int64 x; //!< The x coordinate
      Int64 y; //!< The y coordinate
      Int64 z; //!< The z coordinate
    };

    //!
    //! @struct udMathULong3
    //! A 3D geometric vector with 64bit unsigned integers
    //! 
    struct udMathULong3
    {
      UInt64 x; //!< The x coordinate
      UInt64 y; //!< The y coordinate
      UInt64 z; //!< The z coordinate
    };

    //!
    //! @struct udMathDouble4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with double precision
    //! 
    struct udMathDouble4
    {
      double x; //!< The x coordinate
      double y; //!< The y coordinate
      double z; //!< The z coordinate
      double w; //!< The w coordinate
    };

    //!
    //! @struct udMathLong4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with 64bit integers
    //! 
    struct udMathLong4
    {
      Int64 x; //!< The x coordinate
      Int64 y; //!< The y coordinate
      Int64 z; //!< The z coordinate
      Int64 w; //!< The w coordinate
    };

    //!
    //! @struct udMathULong4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with 64bit unsigned integers
    //! 
    struct udMathULong4
    {
      UInt64 x; //!< The x coordinate
      UInt64 y; //!< The y coordinate
      UInt64 z; //!< The z coordinate
      UInt64 w; //!< The w coordinate
    };

    //!
    //! @struct udMathDouble4x4
    //! A 4x4 geometric matrix with double precision
    //! 
    struct udMathDouble4x4
    {
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =16)]
      double[] array; //!< The matrix elements storing the 4 values for each axis in this order: x axis, y axis, z axis, t axis
    };
  }
}

