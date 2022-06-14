using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Scene
  {
    //!
    //! These are the geometry types for nodes
    //!
    enum udSceneGeometryType
    {
      udPGT_None, //!< There is no geometry associated with this node

      udPGT_Point, //!< pCoordinates is a single 3D position
      udPGT_MultiPoint, //!< Array of udPGT_Point, pCoordinates is an array of 3D positions
      udPGT_LineString, //!< pCoordinates is an array of 3D positions forming an open line
      udPGT_MultiLineString, //!< Array of udPGT_LineString; pCoordinates is NULL and children will be present
      udPGT_Polygon, //!< pCoordinates will be a closed linear ring (the outside), there MAY be children that are interior as pChildren udPGT_MultiLineString items, these should be counted as islands of the external ring.
      udPGT_MultiPolygon, //!< pCoordinates is null, children will be udPGT_Polygon (which still may have internal islands)
      udPGT_GeometryCollection, //!< Array of geometries; pCoordinates is NULL and children may be present of any type

      udPGT_Count, //!< Total number of geometry types. Used internally but can be used as an iterator max when displaying different type modes.

      udPGT_Internal, //!< Used internally when calculating other types. Do not use.
    };

    //!
    //! This represents the type of data stored in the node.
    //! @note The `itemtypeStr` in the udSceneNode is a string version. This enum serves to simplify the reading of standard types. The the string in brackets at the end of the comment is the string.
    //!
    public enum udSceneNodeType
    {
      udPNT_Custom, //!< Need to check the itemtypeStr string manually

      udPNT_PointCloud, //!< A Euclideon Unlimited Detail Point Cloud file ("UDS")
      udPNT_PointOfInterest, //!< A point, line or region describing a location of interest ("POI")
      udPNT_Folder, //!< A folder of other nodes ("Folder")
      udPNT_Media, //!< An Image, Movie, Audio file or other media object ("Media")
      udPNT_Viewpoint, //!< An Camera Location & Orientation ("Camera")
      udPNT_VisualisationSettings, //!< Visualisation settings (itensity, map height etc) ("VizSet")
      udPNT_I3S, //!< An Indexed 3d Scene Layer (I3S) or Scene Layer Package (SLPK) dataset ("I3S")
      udPNT_Water, //!< A region describing the location of a body of water ("Water")
      udPNT_ViewShed, //!< A point describing where to generate a view shed from ("ViewMap")
      udPNT_Polygon, //!< A polygon model, usually an OBJ or FBX ("Polygon")
      udPNT_QueryFilter, //!< A query filter to be applied to all PointCloud types in the scene ("QFilter")
      udPNT_Places, //!< A collection of places that can be grouped together at a distance ("Places")
      udPNT_HeightMeasurement, //!< A height measurement object ("MHeight")
      udPNT_GTFS, //!< A General Transit Feed Specification object ("GTFS")
      udPNT_LassoNode, //!< A Lasso Selection Folder ("LNode")
      udPNT_QueryGroup, //!< A Group of Query node being represented as one node ("QGroup")
      udPNT_Count //!< Total number of node types. Used internally but can be used as an iterator max when displaying different type modes.
    };

    //!
    //! @struct udCameraPosition
    //! This represents the camera position in 3D
    //! @note contains x,y,z and heading, pitch  of a camera
    //! 
    struct udCameraPosition
    {
      double x; //!< The x coordiante of the camera
      double y; //!< The y coordiante of the camera
      double z; //!< The z coordiante of the camera

      double heading; //!< The heading of the camera
      double pitch; //!< The pitch of the camera
    };

    //!
    //! @struct udSelectedNode
    //! This represents the selected project node of a user
    //!
    struct udSelectedNode
    {
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =37 )]
      char[] id; //!< The uuid of the selected node
    };

    //!
    //! @struct udAvatarInfo
    //! This represents the avatar info used for collaboration
    //! 
    struct udAvatarInfo
    {
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string url; //!< the url of the avatar
      double offsetX; //!< offset x of the avatar
      double offsetY; //!< offset y of the avatar
      double offsetZ; //!< offset z of the avatar
      double scaleX; //!< scale of the avatar in x
      double scaleY; //!< scale of the avatar in y
      double scaleZ; //!< scale of the avatar in z
      double yaw; //!< The yaw of the avatar
      double pitch; //!< The pitch of the avatar
      double roll; //!< The roll of the avatar
    };

    //!
    //! @struct udMessage
    //! This represents the message sent in project to users
    //! 
    struct udMessage
    {
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pMessageType; //!< The type of the message
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pMessagePayload; //!< The payload of the message

      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pTargetSessionID; //!< The session ID of the message
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pReceivedFromSessionID; //!< The session Id where it's been received
    };

    //!
    //! @struct udUserPosition
    //! This represents the user info used for collaboration
    //! 
    struct udUserPosition
    {
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string userName; //!< The username of this user
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string ID; //!< The uuid of the user
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pSceneSessionID; //!< THe current scene/session ID this user is log in
      double lastUpdated; //!< The time its position has been kast updated

      UInt32 selectedNodesCount; //!< The number of node selected
      IntPtr selectedNodesList; //udSelectedNode //!< The selected nodes

      IntPtr cameraPositionList; //udCameraPosition //!< The list of positions for each camera

      udAvatarInfo avatar; //!< The info on the used avatar by this user
    };
    //!
    //! @struct udSceneNode
    //! Stores the state of a node.
    //! @warning This struct is exposed to avoid having a huge API of accessor functions but it should be treated as read only with the exception of `pUserData`. Making changes to the internal data will cause issues syncronising data
    //!
    struct udSceneNode
    {
      // Node header data
      int isVisible; //!< Non-zero if the node is visible and should be drawn in the scene
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =37 )]
      char[] UUID; //!< Unique identifier for this node "id"
      double lastUpdate; //!< The last time this node was updated in UTC

      udSceneNodeType itemtype; //!< The type of this node, see udSceneNodeType for more information
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
      char[] itemtypeStr; //!< The string representing the type of node. If its a known type during node creation `itemtype` will be set to something other than udPNT_Custom

      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pName; //!< Human readable name of the item
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      string pURI; //!< The address or filename that the resource can be found.

      UInt32 hasBoundingBox; //!< Set to not 0 if this nodes boundingBox item is filled out
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
      double[] boundingBox; //!< The bounds of this model, ordered as [West, South, Floor, East, North, Ceiling]

      // Geometry Info
      udSceneGeometryType geomtype; //!< Indicates what geometry can be found in this model. See the udSceneGeometryType documentation for more information.
      int geomCount; //!< How many geometry items can be found on this model
      IntPtr pCoordinates; // (double*)!< The positions of the geometry of this node (NULL this this node doesn't have points). The format is [X0,Y0,Z0,...Xn,Yn,Zn]

      // Parent node
      IntPtr pParent; //udSceneNode //!< This is the parent item of the current node (NULL if root node)

      // Next nodes
      IntPtr pNextSibling; //udSceneNode //!< This is the next item in the scene (NULL if no further siblings)
      IntPtr pFirstChild; //udSceneNode //!< Some types ("folder", "collection", "polygon"...) have children nodes, NULL if there are no children.

      // Node Data
      IntPtr pUserDataCleanup;//void (*pUserDataCleanup)(struct udSceneNode *pNode, void *pUserData); //!< When a project node is deleted, this function is called first
      IntPtr pUserData; // (void*)!< This is application specific user data. The application should traverse the tree to release these before releasing the udScene
      IntPtr pInternalData; //udInternalProjectNode //!< Internal udSDK specific state for this node
    };

    //!
    //! This represents where the scene was loaded from/saved to most recently and where future calls to udScene_Save will go
    //!
    public enum udSceneLoadSource
    {
      udSceneLoadSource_Memory, //!< The scene source exists in memory; udScene_CreateInMemory, udScene_LoadFromMemory or udScene_SaveToMemory
      udSceneLoadSource_Server, //!< The scene source exists from the server; udScene_CreateInServer, udScene_LoadFromServer or udScene_SaveToServer
      udSceneLoadSource_URI, //!< The scene source exists from a file or URL; udScene_CreateInFile, udScene_LoadFromFile or udScene_SaveToFile

      udSceneLoadSource_Count //!< Total number of source types. Used internally but can be used as an iterator max when displaying different source types.
    };

    //!
    //! @struct udSceneUpdateInfo
    //! This represents the update info given/received to/by udScene_Update
    //! @warning Memory is Freed on next call of udScene_Updte()
    //! 
    struct udSceneUpdateInfo
    {
      UInt32 forceSync; //!< If this is non-zero the sync to the server will happen immediately and the update call will block

      IntPtr pCameraPositions; //udCameraPosition //!< The position of each camera 
      UInt32 count; //!< The lenght of pCameraPositions

      IntPtr pUserList; //udUserPosition //!< The list of position for each user on this project
      UInt32 usersCount; //!< The lenght of pUserList

      IntPtr pSelectedNodesList; //udSelectedNode //!< The list of selected nodes
      UInt32 selectedNodesCount; //!< The length of pSelectedNodesList

      udAvatarInfo avatar; //!< The info required to display the avatar

      IntPtr pReceivedMessages; //udMessage //!< The list of messages
      UInt32 receivedMessagesCount; //!< The length of pReceivedMessages
    };

    public class udScene
    {
      IntPtr pScene;
      udContext context;
      udSceneUpdateInfo updateInfo;
      public udScene(udContext context)
      {
        this.context = context;
      }

      [DllImport("udSDK")]
      private static extern udError udScene_Release(ref IntPtr ppScene);
      ~udScene()
      {
        udScene_Release(ref pScene);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_CreateInMemory(IntPtr pContext, ref IntPtr ppScene, string pName);
      public void CreateInMemory(string name)
      {
        udError error = udScene_CreateInMemory(context.pContext, ref pScene, name);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_CreateInFile(IntPtr pContext, ref IntPtr ppScene, string pName, string pFilename);
      public void CreateInFile(string name, string filename)
      {
        udError error = udScene_CreateInFile(context.pContext, ref pScene, name, filename);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }

      [DllImport("udSDK")]
      private static extern udError udScene_CreateInServer(IntPtr pContext, ref IntPtr ppScene, string pName, string pGroupID);
      public void CreateInServer(string name, string groupID)
      {
        udError error = udScene_CreateInServer(context.pContext, ref pScene, name, groupID);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_LoadFromMemory(IntPtr pContext, ref IntPtr ppScene, string pGeoJSON);
      public void LoadFromMemory(string geoJSONString)
      {
        udError error = udScene_LoadFromMemory(context.pContext, ref pScene, geoJSONString);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_LoadFromFile(IntPtr pContext, ref IntPtr ppScene, string pFilename);
      public void LoadFromFile(string filename)
      {
        udError error = udScene_LoadFromFile(context.pContext, ref pScene, filename);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }

      [DllImport("udSDK")]
      private static extern udError udScene_LoadFromServer(IntPtr pContext, ref IntPtr ppScene, string pSceneUUID, string pGroupID);
      public void LoadFromServer(string sceneUUID, string groupID)
      {
        udError error = udScene_LoadFromServer(context.pContext, ref pScene, sceneUUID, groupID);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_Save(IntPtr pScene);
      public void Save()
      {
        udScene_Save(pScene);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_Update(IntPtr pScene, ref udSceneUpdateInfo pUpdateInfo);
      public void Update()
      {
        udScene_Update(pScene, ref updateInfo);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_SaveToMemory(IntPtr pContext, IntPtr pScene, ref string ppMemory);
      public void SaveToMemory(string name)
      {
        string geoJSON = null;
        udError error = udScene_SaveToMemory(context.pContext, pScene, ref geoJSON);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_SaveToFile(IntPtr pContext, IntPtr pScene, string pPath);
      public void SaveToFile(string filename)
      {
        udError error = udScene_SaveToFile(context.pContext, pScene, filename);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }

      [DllImport("udSDK")]
      private static extern udError udScene_SaveToServer(IntPtr pContext, IntPtr pScene, string pGroupID);
      public void SaveToServer(string name, string groupID)
      {
        udError error = udScene_SaveToServer(context.pContext, pScene, groupID);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_GetProjectRoot(IntPtr pScene, ref IntPtr ppRootNode);
      public SceneNode RootNode
      {
        get
        {
          SceneNode projectRoot = new SceneNode();
          udError error = udScene_GetProjectRoot(pScene, ref projectRoot.pNode);
          if (error != udError.udE_Success)
            throw new UDException(error);
          return projectRoot;
        }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_GetProjectUUID(IntPtr pScene, ref string ppSceneUUID);
      public string UUID
      {
        get
        {
          string ret = null;
          udError error = udScene_GetProjectUUID(pScene, ref ret);
          if(error != udError.udE_Success)
          {
            throw new UDException(error);
          }
          return ret;
        }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_HasUnsavedChanges(IntPtr pScene);
      public bool HasUnsavedChanges
      {
        get
        {
          udError error = udScene_HasUnsavedChanges(pScene);
          if (error == udError.udE_NotFound)
            return false;
          if (error == udError.udE_Success)
            return true;
          throw new UDException(error);
        }
      }

      [DllImport("udSDK")]
      private static extern IntPtr udScene_GetTypeName( udSceneNodeType itemtype); // Might return NULL
      public static string GetTypeName(udSceneNodeType nodeType)
      {
        IntPtr ptr =  udScene_GetTypeName(nodeType);
        if (ptr == IntPtr.Zero)
          return null;
        return Marshal.PtrToStringUTF8(ptr);
      }

      [DllImport("udSDK")]
      private static extern udError udScene_GetLoadSource(IntPtr pScene, ref udSceneLoadSource pSource);
      public udSceneLoadSource LoadSource
      {
        get
        {
          udSceneLoadSource ret = udSceneLoadSource.udSceneLoadSource_Count;
          udError error = udScene_GetLoadSource(pScene, ref ret);
          if(error != udError.udE_Success)
          {
            throw new UDException(error);
          }
          return ret;
          
        }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_DeleteServerProject(IntPtr pContext, string pSceneUUID, string pGroupID);
      public static void DeleteServerProject(udContext context, string sceneUUID, string groupID)
      {
          udError error = udScene_DeleteServerProject(context.pContext, sceneUUID, groupID);
          if(error != udError.udE_Success)
          {
            throw new UDException(error);
          }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_SetLinkShareStatus(IntPtr pContext, string pSceneUUID, UInt32 isSharableToAnyoneWithTheLink, string pGroupID);
      public static void SetLinkShareStatus(udContext context, bool sharableWithAnyoneWithLink, string sceneUUID, string groupID)
      {
          udError error = udScene_SetLinkShareStatus(context.pContext, sceneUUID, System.Convert.ToUInt32(sharableWithAnyoneWithLink), groupID);
          if(error != udError.udE_Success)
          {
            throw new UDException(error);
          }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_GetSessionID(IntPtr pScene, ref IntPtr ppSessionID);
      public string SessionID
      {
        get
        {
          IntPtr ret = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(IntPtr)));
          udError error = udScene_GetSessionID(pScene, ref ret);

          if(error != udError.udE_Success)
          {
            throw new UDException(error);
          }
          string o = Marshal.PtrToStringUTF8(ret);
          Marshal.FreeHGlobal(ret);
          return o;
        }
      }

      [DllImport("udSDK")]
      private static extern udError udScene_QueueMessage(IntPtr pScene, string pTargetSessionID, string pMessageType, string pMessagePayload);
      public void QueueMessage(string targetSessionID, string messageType, string messagePayload)
      {
        udError error = udScene_QueueMessage(pScene, targetSessionID, messageType, messagePayload);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }

      [DllImport("udSDK")]
      private static extern udError udScene_SaveThumbnail(IntPtr pScene, string pImageBase64);
      public void SaveThumbnail(string imageBase64)
      {
        udError error = udScene_SaveThumbnail(pScene, imageBase64);
        if (error != udError.udE_Success)
          throw new UDException(error);

      }
    }

    public class SceneNode
    {
      public udScene scene;
      public IntPtr pNode;
      private udSceneNode node;

      public void Update()
      {
        node = Marshal.PtrToStructure<udSceneNode>(pNode);
      }

    }
  }
}
