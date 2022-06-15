using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Scene
  {
    //!
    //! These are the geometry types for nodes
    //!
    public enum udSceneGeometryType
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
      public int isVisible; //!< Non-zero if the node is visible and should be drawn in the scene
      [MarshalAs(UnmanagedType.ByValArray, SizeConst =37 )]
      public char[] UUID; //!< Unique identifier for this node "id"
      public double lastUpdate; //!< The last time this node was updated in UTC

      public udSceneNodeType itemtype; //!< The type of this node, see udSceneNodeType for more information
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
      public char[] itemtypeStr; //!< The string representing the type of node. If its a known type during node creation `itemtype` will be set to something other than udPNT_Custom

      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pName; //!< Human readable name of the item
      [MarshalAs(UnmanagedType.LPUTF8Str)]
      public string pURI; //!< The address or filename that the resource can be found.

      public UInt32 hasBoundingBox; //!< Set to not 0 if this nodes boundingBox item is filled out
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
      public double[] boundingBox; //!< The bounds of this model, ordered as [West, South, Floor, East, North, Ceiling]

      // Geometry Info
      public udSceneGeometryType geomtype; //!< Indicates what geometry can be found in this model. See the udSceneGeometryType documentation for more information.
      public int geomCount; //!< How many geometry items can be found on this model
      public IntPtr pCoordinates; // (double*)!< The positions of the geometry of this node (NULL this this node doesn't have points). The format is [X0,Y0,Z0,...Xn,Yn,Zn]

      // Parent node
      public IntPtr pParent; //udSceneNode //!< This is the parent item of the current node (NULL if root node)

      // Next nodes
      public IntPtr pNextSibling; //udSceneNode //!< This is the next item in the scene (NULL if no further siblings)
      public IntPtr pFirstChild; //udSceneNode //!< Some types ("folder", "collection", "polygon"...) have children nodes, NULL if there are no children.

      // Node Data
      public IntPtr pUserDataCleanup;//void (*pUserDataCleanup)(struct udSceneNode *pNode, void *pUserData); //!< When a project node is deleted, this function is called first
      public IntPtr pUserData; // (void*)!< This is application specific user data. The application should traverse the tree to release these before releasing the udScene
      public IntPtr pInternalData; //udInternalProjectNode //!< Internal udSDK specific state for this node
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
    public struct udSceneUpdateInfo
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
      public IntPtr pScene;
      udContext context;
      public udSceneUpdateInfo updateInfo;
      public udScene(udContext context)
      {
        this.context = context;
        updateInfo = new udSceneUpdateInfo();
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
          SceneNode projectRoot = new SceneNode(this, IntPtr.Zero);
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

      public SceneNode(udScene scene, IntPtr pNode)
      {
        this.scene = scene;
        this.pNode = pNode;
      }

      private udSceneNode InternalNode
      {
        get
        {
          return Marshal.PtrToStructure<udSceneNode>(pNode);
        }
      }

      public double LastUpdate
      {
        get
        {
          return InternalNode.lastUpdate;
        }
      }

      public bool HasBoundingBox
      {
        get { return System.Convert.ToBoolean(InternalNode.hasBoundingBox); }
      }

      public SceneNode FirstChild
      {
        get
        {
          udSceneNode node = InternalNode;
          if (node.pFirstChild != IntPtr.Zero)
            return new SceneNode(this.scene, InternalNode.pFirstChild);
          else
            return null;
        }
      }

      public SceneNode NextSibling
      {
        get
        {
          udSceneNode node = InternalNode;
          if (node.pNextSibling != IntPtr.Zero)
            return new SceneNode(this.scene, InternalNode.pNextSibling);
          else
            return null;
        }
      }

      public SceneNode Parent
      {
        get
        {
          udSceneNode node = InternalNode;
          if (node.pParent != IntPtr.Zero)
            return new SceneNode(this.scene, InternalNode.pParent);
          else
            return null;
        }
      }

      [DllImport("udSDK")]
      public static extern udError udSceneNode_Create(IntPtr pScene, ref IntPtr ppNode, IntPtr pParent, string pType, string pName, string pURI, IntPtr pUserData);
      public SceneNode(SceneNode parent, string type, string name, string URI, IntPtr? userData=null)
      {
        this.scene = parent.scene;
        udError error = udSceneNode_Create(scene.pScene, ref pNode, parent.pNode, type, name, URI, userData ?? IntPtr.Zero);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }
      
      [DllImport("udSDK")]
      private static extern udError udSceneNode_MoveChild(IntPtr pScene, IntPtr pCurrentParent, IntPtr pNewParent, IntPtr pNode, IntPtr pInsertBeforeChild);
      public void Move(SceneNode newParent, SceneNode beforeNode)
      {
        udSceneNode node = InternalNode;

        udError error = udSceneNode_MoveChild(scene.pScene, node.pParent, newParent.pNode, pNode, beforeNode.pNode);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_RemoveChild(IntPtr pScene, IntPtr pParentNode, IntPtr pNode);
      public void Remove(SceneNode newParent, SceneNode beforeNode)
      {
        udSceneNode node = InternalNode;

        udError error = udSceneNode_RemoveChild(scene.pScene, node.pParent, pNode);
        if (error != udError.udE_Success)
          throw new UDException(error);
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetName(IntPtr pScene, IntPtr pNode, string pNodeName);
      public string Name
      {
        get
        {
          return InternalNode.pName;
        }
        set
        {
          udError error = udSceneNode_SetName(scene.pScene, pNode, value);
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetVisibility(IntPtr pNode, int visibility);
      public bool Visibility
      {
        get
        {
          return System.Convert.ToBoolean(InternalNode.isVisible);
        }
        set
        {
          udError error = udSceneNode_SetVisibility(pNode, System.Convert.ToInt32(value));
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetURI(IntPtr pScene, IntPtr pNode, string pNodeURI);
      public string URI
      {
        get
        {
          return InternalNode.pURI;
        }
        set
        {
          udError error = udSceneNode_SetURI(scene.pScene, pNode, value);
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetBoundingBox(IntPtr pScene, IntPtr pNode, IntPtr boundingBox);
      public double[] BoundingBox
      {
        get
        {
          return InternalNode.boundingBox;
        }
        set
        {
          IntPtr set = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(double)) *6);
          Marshal.StructureToPtr<double[]>(value, set, false);
          udError error = udSceneNode_SetBoundingBox(scene.pScene, pNode, set);
          Marshal.FreeHGlobal(set);
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }

      public udSceneNodeType ItemType { get { return InternalNode.itemtype; } }
      public string ItemTypeStr { get { return new string(InternalNode.itemtypeStr); } }

      private udSceneGeometryType? changeType = null;
      public udSceneGeometryType GeometryType
      {
        get { return InternalNode.geomtype; }
        set { changeType = value; Geometry = Geometry; changeType = null; }
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetGeometry(IntPtr pScene, IntPtr pNode, udSceneGeometryType nodeType, int geometryCount, [In, Out] double[] pCoordinates);
      public double[] Geometry
      {
        get
        {
          udSceneNode node = InternalNode;
          double[] ret = new double[node.geomCount *3];
          Marshal.Copy(node.pCoordinates, ret, 0, node.geomCount * 3);
          return ret;
        }
        set
        {
          udError error = udSceneNode_SetGeometry(scene.pScene, pNode, changeType?? InternalNode.geomtype, value.Length/3, value);
          if (error != udError.udE_Success)
            throw new UDException(error);
        }
      }
      public abstract class Metadata<T>
      {
        protected SceneNode node;
        public Metadata(SceneNode node)
        {
          this.node = node;
        }
        public abstract T this[string key] { get; set; }
      }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataInt(IntPtr pNode, string pMetadataKey, ref Int32 pInt, Int32 defaultValue);
      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataInt(IntPtr pNode, string pMetadataKey, Int32 iValue);
      public class MetadataIntCls :Metadata<Int32?>
      {
        public MetadataIntCls(SceneNode node) : base(node) { }
        override public Int32? this[string key]
        {
          get
          {
            Int32 retVal = 0;
            udError error = udSceneNode_GetMetadataInt(node.pNode, key, ref retVal, 0);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return retVal;
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataInt(node.pNode, key, value?? 0);
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<Int32?> MetadataAsInt { get { return new MetadataIntCls(this); } }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataUint(IntPtr pNode, string pMetadataKey, ref UInt32 pInt, UInt32 defaultValue);

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataUint(IntPtr pNode, string pMetadataKey, UInt32 iValue);
      public class MetadataUIntCls : Metadata<UInt32?>
      {
        public MetadataUIntCls(SceneNode node) : base(node) { }
        override public UInt32? this[string key]
        {
          get
          {
            UInt32 retVal = 0;
            udError error = udSceneNode_GetMetadataUint(node.pNode, key, ref retVal, 0);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return retVal;
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataUint(node.pNode, key, value?? 0);
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<UInt32?> MetadataAsUInt { get { return new MetadataUIntCls(this); } }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataInt64(IntPtr pNode, string pMetadataKey, ref Int64 pInt64, Int64 defaultValue);
      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataInt64(IntPtr pNode, string pMetadataKey, Int64 doubleValue);
      public class MetadataInt64Cls : Metadata<Int64?>
      {
        public MetadataInt64Cls(SceneNode node) : base(node) { }
        override public Int64? this[string key]
        {
          get
          {
            Int64 retVal = 0;
            udError error = udSceneNode_GetMetadataInt64(node.pNode, key, ref retVal, 0);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return retVal;
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataInt64(node.pNode, key, value?? 0);
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<Int64?> MetadataAsInt64 { get { return new MetadataInt64Cls(this); } }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataDouble(IntPtr pNode, string pMetadataKey, ref double pDouble, double defaultValue);

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataDouble(IntPtr pNode, string pMetadataKey, double doubleValue);
      public class MetadataDoubleCls : Metadata<double?>
      {
        public MetadataDoubleCls(SceneNode node) : base(node) { }
        override public double? this[string key]
        {
          get
          {
            double retVal = 0;
            udError error = udSceneNode_GetMetadataDouble(node.pNode, key, ref retVal, 0);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return retVal;
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataDouble(node.pNode, key, value?? 0);
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<double?> MetadataAsDouble { get { return new MetadataDoubleCls(this); } }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataBool(IntPtr pNode, string pMetadataKey, ref UInt32 pBool, UInt32 defaultValue);

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataBool(IntPtr pNode, string pMetadataKey, UInt32 boolValue);
      public class MetadataBoolCls : Metadata<bool?>
      {
        public MetadataBoolCls(SceneNode node) : base(node) { }
        override public bool? this[string key]
        {
          get
          {
            UInt32 retVal = 0;
            udError error = udSceneNode_GetMetadataBool(node.pNode, key, ref retVal, 0);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return System.Convert.ToBoolean(retVal);
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataBool(node.pNode, key, System.Convert.ToUInt32(value?? false));
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<bool?> MetadataAsBool { get { return new MetadataBoolCls(this); } }

      [DllImport("udSDK")]
      private static extern udError udSceneNode_GetMetadataString(IntPtr pNode, string pMetadataKey, ref IntPtr ppString, string pDefaultValue);

      [DllImport("udSDK")]
      private static extern udError udSceneNode_SetMetadataString(IntPtr pNode, string pMetadataKey, string pString);
      public class MetadataStringCls : Metadata<string>
      {
        public MetadataStringCls(SceneNode node) : base(node) { }
        override public string this[string key]
        {
          get
          {
            IntPtr intPtr = IntPtr.Zero;
            udError error = udSceneNode_GetMetadataString(node.pNode, key, ref intPtr, "");
            string val = Marshal.PtrToStringUTF8(intPtr);
            if (error == udError.udE_NotFound)
              return null;
            else
            {
              if (error != udError.udE_Success)
                throw new UDException(error);
              return val;
            }
          }
          set
          {
            udError error = udSceneNode_SetMetadataString(node.pNode, key, value?? "");
            if (error != udError.udE_Success)
              throw new UDException(error);
          }
        }
      }

      public Metadata<string> MetadataAsString { get { return new MetadataStringCls(this); } }
    }
  }
}
