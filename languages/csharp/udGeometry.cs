using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Geometry
  {

    //! @typedef udGeometryDouble2
    //! A 2D geometric vector with double precision
    //! 
   using udGeometryDouble2 = MathTypes.udMathDouble2 ;

    //!
    //! @typedef udGeometryDouble3
    //! A 3D geometric vector with double precision
    //! 
    using udGeometryDouble3 = MathTypes.udMathDouble3;

    //!
    //! @typedef udGeometryDouble4
    //! A 4D geometric vector, or 3D vector for homogeneous coordinates with double precision
    //! 
    using udGeometryDouble4 = MathTypes.udMathDouble4;

    //!
    //! @typedef udGeometryDouble4x4
    //! A 4x4 geometric matrix with double precision
    //! 
    using udGeometryDouble4x4 = MathTypes.udMathDouble4x4;

    //!
    //!
    //! The currently supported geometry types
    //! 
    enum udGeometryType
    {
      udGT_Inverse, //!< An inversion filter; flips the udGeometryTestResult of the child udGeometry node
      udGT_CircleXY, //!< A 2D Circle with an infinite Z value
      udGT_RectangleXY, //!< A 2D Rectangle with an infinite Z value
      udGT_PolygonXY, //!< A 2D Polygon with rotation (quaternion) to define the up of the polygon
      udGT_PolygonPerspective, //!< A 2D polygon with a perspective projection to the screen plane
      udGT_Cylinder, //!< @deprecated A radius out of a line which caps immediately at the end of the line
      udGT_Capsule, //!< A line with a radius from the line; forming hemispherical caps at the end of the line
      udGT_Sphere, //!< A radius from a point
      udGT_HalfSpace, //!< A binary space partition allowing 1 side of a plane
      udGT_AABB, //!< An axis aligned box; Use with caution. OBB while less performant correctly handles transforms
      udGT_OBB, //!< An oriented bounding box using half size and orientation
      udGT_CSG, //!< A constructed solid geometry that uses a udGeometryCSGOperation to join to child udGeometry nodes

      udGT_Count //!< Count helper value to iterate this enum
    };

    //!
    //! @struct udGeometryVoxelNode
    //! The geometric representation of a Node in a Unlimited Detail Model.
    //! 
    struct udGeometryVoxelNode
    {
      udGeometryDouble3 minPos; //!< The Bottom, Left, Front corner of the voxel (closest to the origin)
      double childSize; //!< The half size of the voxel (which is the same size as this voxels children)
    };

    //!
    //! @struct udGeometryCircleXY
    //! The geometric representation of a Circle.
    //! 
    struct udGeometryCircleXY
    {
      udGeometryDouble2 centre; //!< The centre of the circle
      double radius; //!< The radius of the circle
    };

    //!
    //! @struct udGeometryRectangleXY
    //! The geometric representation of a Rectangle.
    //! 
    struct udGeometryRectangleXY
    {
      udGeometryDouble2 minPoint; //!< The lowest point of the rectangle
      udGeometryDouble2 maxPoint; //!<The highest point of the rectangle
    };

    //!
    //! @struct udGeometryPolygonXYZ
    //! The geometric representation of a Polygon.
    //! 
    struct udGeometryPolygonXYZ
    {
      UInt32 pointCount; //!< THe number of points defining the polygon
      //udGeometryDouble3* pointsList; //!< The list of points defining the polygon
      IntPtr pointslist;
      udGeometryDouble4 rotationQuat; //!< The rotation of the polygon
    };

    //!
    //! @struct udGeometryPolygonPerspective
    //! The geometric representation of a Polygon with a perspective projection.
    //! 
    struct udGeometryPolygonPerspective
    {
      UInt32 pointCount; //!< The number of points defining the polygon
      //udGeometryDouble3* pointsList; //!< The list of points defining the polygon
      IntPtr pointsList;
      udGeometryDouble4 rotationQuat; //!< The rotation of the polygon
      udGeometryDouble4x4 worldToScreen; //!< The matrix to project from World space to Screen space
      udGeometryDouble4x4 projectionMatrix; //!< The matrix to project the points of the polygon
      udGeometryDouble4x4 cameraMatrix; //!< The camera matrix
      udGeometryDouble3 normRight; //!< The normal on the right of the plane
      udGeometryDouble3 normLeft; //!< The normal on the left of the plane
      udGeometryDouble3 normTop; //!< The normal on the top of the plane
      udGeometryDouble3 normBottom; //!< The normal on the bottom of the plane
      double nearPlane; //!< the near plane distance
      double farPlane; //!< The far plane distance
    };

    //!
    //! @struct udGeometryCylinder
    //! The geometric representation of a cylinder
    //! @deprecated This object has edge cases that do not resolve correctly and is highly non-performant
    //! 
    struct udGeometryCylinder
    {
      udGeometryDouble3 point1; //!< The point at one end of the line
      udGeometryDouble3 point2; //!< The point at the other end of the line
      double radius; //!< The radius around the line

      // Derived values
      udGeometryDouble3 axisVector; //!< The vector of the line
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =2)]
      udGeometryDouble4[] planes; //!< The two planes for the caps
    };

    //!
    //! @struct udGeometryCapsule
    //! Stores the properties of a geometric capsule
    //! 
    struct udGeometryCapsule
    {
      udGeometryDouble3 point1; //!< One end of the line
      udGeometryDouble3 point2; //!< The other end of the line
      double radius; //!< The radius around the line

      // Derived values
      udGeometryDouble3 axisVector; //!< The vector of the line
      double length; //!< The length of the line
    };

    //!
    //! @struct udGeometrySphere
    //! Stores the properties of a geometric sphere
    //! 
    struct udGeometrySphere
    {
      udGeometryDouble3 center; //!< The center of the sphere
      double radius; //!< The radius of the sphere
    };

    //!
    //! @struct udGeometryHalfSpace
    //! Stores the properties of a geometric half space
    //! 
    struct udGeometryHalfSpace
    {
      udGeometryDouble4 plane; //!< The parameters to define the plane (normal XYZ and offset from origin)
    };

    //!
    //! @struct udGeometryAABB
    //! Stores the properties of a geometric axis aligned bounding box
    //!
    struct udGeometryAABB
    {
      udGeometryDouble3 center; //!< The point at the center of the AABB
      udGeometryDouble3 extents; //!< The half space size of the AABB
    };

    //!
    //! @struct udGeometryOBB
    //! Stores the properties of a geometric axis aligned bounding box (extending AABB)
    //! 
    struct udGeometryOBB
    {
      udGeometryDouble3 center; //!< The point at the center of the AABB
      udGeometryDouble3 extents; //!< The half space size of the AABB
      udGeometryDouble4x4 rotationMatrix; //!< The transform that represents the rotation
    };

    //!
    //! The Constructive Solid Geometry operations
    //! 
    enum udGeometryCSGOperation
    {
      udGCSGO_Union = 0,   //!< A union CSG operation; any point matching the one or the other geometry (OR operation)
      udGCSGO_Difference,  //!< A subtractive CSG operation; any point in the first geometry but not matching the second geometry (XOR operation)
      udGCSGO_Intersection //!< An intersection CSG operation; any point that matches both geometries (AND operation)
    };

    //!
    //! @struct udGeometryInverse
    //! Stores the properties of an inversed udGeometry node
    //! 
    struct udGeometryInverse
    {
      IntPtr /*udGeometry * */pInverseOf; //!< The inverse geometry
      int owns; //!< If non-zero pInverseOf is owned by this need (and will need to be cleaned up)
    };

    //!
    //! @struct udGeometryCSG
    //! Stores the properties of a CSG udGeometry node
    //! 
    struct udGeometryCSG
    {
      IntPtr /*udGeometry * */pFirst; //!< The first geometry
      IntPtr /*udGeometry * */pSecond; //!< The second geometry
      udGeometryCSGOperation operation; //!< The operation applied to the 2 gemetries
      int owns; //!< non zero if it owns both children
    };

    //!
    //! The results of a geometry test
    //! 
    enum udGeometryTestResult
    {
      udGTR_CompletelyOutside = 0, //!< The node is totally outside of the geometry (and no further tests are required)
      udGTR_CompletelyInside = 1, //!< The node is totally inside of the geometry (and no further tests are required)
      udGTR_PartiallyOverlap = 2 //!< The node is overlapping the boundary of the geoetry and further tests will be required to place the voxel inside or outside 
    };
    public class udGeometry
    {
      public IntPtr pGeometry;

      [DllImport("udSDK")]
      private static extern void udGeometry_InitInverse( ref udGeometry pGeometry,  ref udGeometry pSource);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitCircleXY( ref udGeometry pGeometry, udGeometryDouble2 centre, double radius);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitRectangleXY( ref udGeometry pGeometry, udGeometryDouble2 point1, udGeometryDouble2 point2);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitPolygonXY( ref udGeometry pGeometry,IntPtr /*udGeometryDouble3 * */ pXYCoords, UInt32 count, udGeometryDouble4 rotationQuat);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitPolygonPerspective( ref udGeometry pGeometry, IntPtr /* udGeometryDouble2 * */pXYCoords, UInt32 count, udGeometryDouble4x4 projectionMatrix, udGeometryDouble4x4 cameraMatrix, double nearPlaneOffset, double farPlaneOffset);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitCSG( ref udGeometry pGeometry,  ref udGeometry pGeometry1,  ref udGeometry pGeometry2, udGeometryCSGOperation function);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitHalfSpace( ref udGeometry pGeometry, udGeometryDouble3 point, udGeometryDouble3 normal);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitCylinderFromEndPoints( ref udGeometry pGeometry, udGeometryDouble3 point1, udGeometryDouble3 point2, double radius);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitCylinderFromCenterAndHeight( ref udGeometry pGeometry, udGeometryDouble3 centre, double radius, double halfHeight, udGeometryDouble3 yawPitchRoll);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitCapsule( ref udGeometry pGeometry, udGeometryDouble3 point1, udGeometryDouble3 point2, double radius);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitSphere( ref udGeometry pGeometry, udGeometryDouble3 center, double radius);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitAABBFromMinMax( ref udGeometry pGeometry, udGeometryDouble3 point1, udGeometryDouble3 point2);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitAABBFromCentreExtents( ref udGeometry pGeometry, udGeometryDouble3 centre, udGeometryDouble3 extents);

      [DllImport("udSDK")]
      private static extern void udGeometry_InitOBB( ref udGeometry pGeometry, udGeometryDouble3 centerPoint, udGeometryDouble3 extents, udGeometryDouble3 rotations);

      [DllImport("udSDK")]
      private static extern void udGeometry_Deinit( ref udGeometry pGeometry);

      [DllImport("udSDK")]
      private static extern udError udGeometry_Create(ref IntPtr /*udGeometry ** */ppGeometry);

      [DllImport("udSDK")]
      private static extern void udGeometry_Destroy(ref IntPtr /* udGeometry ** */ppGeometry);
      
    }
  }
}
