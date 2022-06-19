/*

# Using udSDKJS

 * First, initiate the library by calling `udSDKJS_RegisterShared()`. After this call the other functions will be registered for use.
 * Next, use one of the Connection functions to connect to either udCloud or the (legacy) udStream server.
 * When udSDKJS is no longer required call `udSDKJS_DestroyShared()` to clean up shared memory.

# Notes

 * Error codes returned by some methods can be interpreted with the udSDKJS_GetErrorString() function. See udError.h for desciptions of each error.
 * 3D points are handled as objects, in the format:
        point = { x : value, y: value, z: value }

 # API listing categories:

  * Connection
  * UDS Management
  * Rendering
  * Project Management
  * Utility

*/

"use strict";

//---------------------------------------------------------------------------------------------
// Connection
//---------------------------------------------------------------------------------------------

//!
//! Initialises the library and establishes a (legacy) connection to a Euclideon udServer.
//! The server address can be set with udSDKJS_SetServerAddress().
//!
//!   errorCode udSDKJS_CreateShared(email, password, applicationName)
//!
//! @param email {string} The email address of the user connecting to the Euclideon udServer.
//! @param password {string} The password of the user connecting to the Euclideon udServer.
//! @param applicationName {string} The name of the application connecting to the Euclideon udServer.
//! @return {number} An error code based on the result of the connection creation.
//! @note The application should call udSDKJS_DestroyShared to disconnect from the server.
//!
let udSDKJS_CreateShared;

//!
//! Initialises the library and establishes a connection to Euclideon udCloud.
//!
//!   errorCode udSDKJS_CreateFrom_udCloud(applicationName)
//!
//! @param applicationName {string} The name of the application connecting to Euclideon udCloud.
//! @return {Promise} The created promise.
//! @note The application should call udSDKJS_DestroyShared to disconnect from the server.
//! @note This function will open a tab for the user to sign in to udCloud.
//!
function udSDKJS_CreateFrom_udCloud(applicationName) {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'v');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_CreateFrom_udCloudInternal(applicationName, onSuccessPtr, onFailurePtr);
  });
}

//!
//! Initialises the library and establishes a (legacy) domain connection to a Euclideon udServer.
//! The server address can be set with udSDKJS_SetServerAddress().
//!
//!   errorCode udSDKJS_CreateSharedFromDomain(applicationName)
//!
//! @param applicationName {string} The name of the application connecting to the Euclideon udServer.
//! @return {number} An error code based on the result of the connection creation.
//! @note The application should call udSDKJS_DestroyShared to disconnect from the server.
//!
let udSDKJS_CreateSharedFromDomain;

//!
//! Initialises the library and establishes a connection to Euclideon udCloud.
//! Will only have access to publicly shared scenes on udCloud.
//!
//!   Promise udSDKJS_CreateSharedFrom_udCloud(applicationName)
//!
//! @param applicationName {string} The name of the application connecting to Euclideon udCloud.
//! @return {Promise} The created promise.
//! @note The application should call udSDKJS_DestroyShared to disconnect from the server.
//!
function udSDKJS_CreateSharedFrom_udCloud(applicationName) {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'v');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_CreateSharedFrom_udCloudInternal(applicationName, onSuccessPtr, onFailurePtr);
  });
}

//!
//! Uninitialises the library and disconects from either udCloud or udServer.
//!
//!   Promise udSDKJS_DestroyShared()
//!
//! @return {Promise} The created Promise.
//!
function udSDKJS_DestroyShared(applicationName) {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'v');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_DestroySharedInternal(onSuccessPtr, onFailurePtr);
  });
}

//!
//! (legacy) Sets the server address to connect to before connecting with udSDKJS_CreateShared() or udSDKJS_CreateSharedFromDomain().
//!
//!   errorCode udSDKJS_SetServerAddress(address)
//!
//! @param address {string} The server address.
//! @return An error code based on the result of setting the server address.
//!
let udSDKJS_SetServerAddress;


// Derecated
function udSDKJS_Login(username, password, application) {
  udSDKJS_RegisterShared();
  return udSDKJS_CreateShared(username, password, application)
}

// Derecated
function udSDKJS_Domain(application) {
  udSDKJS_RegisterShared();
  return udSDKJS_CreateSharedFromDomain(application)
}

//---------------------------------------------------------------------------------------------
// UDS management
//---------------------------------------------------------------------------------------------

//!
//! Loads a UDS model from `modelLocation`.
//!
//!   handle udSDKJS_LoadModel(modelLocation)
//!
//! @param modelLocation {string} The location to load the model from. This should be a HTTP or HTTPS URL.
//! @return {number} A handle to a model at the provided URL. This will be 0 if the model failed to load.
//! @warning This funtion does not handle loading multiple models at the same time. Only load one model at a time.
//!
function udSDKJS_LoadModel(modelLocation) {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'vi');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_LoadModelInternal(modelLocation, onSuccessPtr, onFailurePtr);
  });
}

//!
//! Unloads a UDS model.
//!
//!   errorCode udSDKJS_ReleaseModel(handle);
//!
//! @param handle {number} A handle to the previously loaded model.
//! @return {number} An error code based on the result of releasing the model.
//!
let udSDKJS_ReleaseModel;

//!
//! Retrieves the header data from a previously loaded UDS model.
//!
//!   object udSDKJS_GetHeaderData(handle)
//!
//! @param handle {number} A handle to the previously loaded model.
//! @return {object} A JavaScript object containing the model header data.
//!
function udSDKJS_GetHeaderData(model) {
  // This is a wrapper to return an object rather than a string
  return JSON.parse(udSDKJS_GetHeaderDataInternal(model));
}

//---------------------------------------------------------------------------------------------
// Rendering
//---------------------------------------------------------------------------------------------

//!
//! Sets new color and depth buffers as well as sets the size of them. If creating these in JavaScript, these
//! must be allocated with Module._malloc().
//!
//!   errorCode udSDKJS_ResizeScene(width, height, colourBuffer, depthBuffer)
//!
//! @param width {number} The new window width.
//! @param height {number} The new window height.
//! @param colourBuffer {number} A pointer to the new colour buffer (0 to have udSDKJS create it).
//! @param depthBuffer {number} A pointer to the depth buffer (0 to have udSDKJS create it).
//! @return An error code based on the result of resizing the scene.
//!
let udSDKJS_ResizeScene;

//!
//! Sets the 4x4 matrix for a given matrix type.
//!
//!   errorCode udSDKJS_SetMatrix(matrixType, a0, a1 ... a15)
//!
//! @param matrixType {string} The matrix type to set. Can be "view", "projection", "camera" or "viewport".
//! @param a0, a1 ... a15 {number} The 16 elements of the matrix.
//! @return {number} An error code based on the result of setting the matrix.
//!
let udSDKJS_SetMatrix;

//!
//! Sets the 4x4 matrix for a given model.
//!
//!   errorCode udSDKJS_SetModelMatrix(modelHandle, matrix)
//!
//! @param modelHandle {number} A handle to a previously loaded model.
//! @param matrix {array} An array of 16 values representing the transform matrix.
//! @return {number} An error code based on the result of setting the matrix.
//!
function udSDKJS_SetModelMatrix (modelHandle, matrix) {
  let ptr = Module._malloc(16 * 8);
  for (let i = 0; i < 16; i++) {
    Module.setValue(ptr + (i * 8), matrix[i], 'double');
  }
  let code = udSDKJS_SetModelMatrixInternal(modelHandle, ptr);
  Module._free(ptr);
  return code;
}

//!
//! Adds the model to the render queue.
//!
//!   slotID udSDKJS_RenderQueueAddModel(handle, zOffset, targetzone)
//!
//! @param handle {number} A handle to a previously loaded model.
//! @param zOffset {number} A height offset to apply to the model before rendering.
//! @param targetzone {number} The EPSG code of the geozone to render to. Additional values include:
//!                   0 : Local space
//!                  -1 : ESRI ECEF
//!                  -2 : MapBox
//!                  -3 : Remove translation
//!                  -4 : Unit cube
//! @return {number} The slot ID it was added to; negative values indicate failure.
//!
let udSDKJS_RenderQueueAddModel;

//!
//! Adds the model to the render queue with a custom transformation matrix.
//!
//!   slotID udSDKJS_RenderQueueAddModelWithMatrix(handle, matrix)
//!
//! @param handle {number} A handle to a previously loaded model.
//! @param matrix {array} An array of 16 values representing the transform matrix.
//! @return {number} The slot ID it was added to; negative values indicate failure.
//!
function udSDKJS_RenderQueueAddModelWithMatrix(handle, matrix) {
  let ptr = Module._malloc(16 * 8);
  for (let i = 0; i < 16; i++) {
    Module.setValue(ptr + (i * 8), matrix[i], 'double');
  }
  let slotID = udSDKJS_RenderQueueAddModelWithMatrixInternal(handle, ptr);
  Module._free(ptr);
  return slotID;
}

//!
//! Remove an item from the render queue.
//!
//!   void udSDKJS_RenderQueueRemoveItem(slotID)
//!
//! @param slotID {number} The slot ID of the model to remove.
//!
let udSDKJS_RenderQueueRemoveItem

//!
//! Clears the render queue.
//!
//!   errorCode udSDKJS_RenderQueueClear()
//!
//! @return {number} An error code based on the result of clearing the render queue.
//!
let udSDKJS_RenderQueueClear;

//!
//! Render the model queue.
//!
//!   errorCode udSDKJS_RenderQueue()
//!
//! @return {number} An error code based on rendering the queue.
//!
let udSDKJS_RenderQueue;

//!
//! Set the coordinates of a point to pick the next render call.
//!
//!   errorCode udSDKJS_SetPick(x, y)
//!
//! @param x {number} x coordinate of the screen.
//! @param y {number} y coordinate of the screen.
//! @return {number} An error code based on setting the pick.
//!
let udSDKJS_SetPick;

//!
//! Get the pick pick data from the last render.
//!
//!   pickData udSDKJS_GetPickData()
//!
//! @return {object} An object containing the pick data, in the format
//!                  {
//!                    hit: (bool)
//!                    isHighestLOD: (bool)
//!                    slotID: (number)
//!                    point: { x: (number), y: (number), z: (number) }
//!                    voxelData : {}
//!                  }
//!
function udSDKJS_GetPickData(getVoxelData = false) {
  let offset = udSDKJS_GetPickDataInternal();

  let hit = getValue(offset + 8, "i32") == 0 ? false : true;

  if (!hit) {
    return { hit: false };
  }

  let voxelObj = null;
  if (getVoxelData) {
    const bufSize = 1024;
    let ptr = Module._malloc(bufSize);
    let code = udSDKJS_GetVoxelDataInternal(ptr, bufSize);
    if (code != 0) {
      console.log("Error retrieving voxel data. Code: " + udSDKJS_GetErrorString(code));
    }
    else {
      voxelObj = JSON.parse(Module.UTF8ToString(ptr));
    }
    Module._free(ptr);
  }

  return {
    hit: true,
    isHighestLOD: getValue(offset + 12, "i32") == 0 ? false : true,
    slotID: udSDKJS_GetPickedModelIDInternal(),
    point: {
      x: getValue(offset + 28, "double"),
      y: getValue(offset + 36, "double"),
      z: getValue(offset + 42, "double"),
    },
    voxelData: voxelObj
  };
}

//!
//! Returns the internal colour buffer if created using `0` in `udSDKJS_ResizeScene()`
//!
//!   buffer udSDKJS_GetColourBuffer()
//!
//! @return {number} A pointer to the internal colour buffer.
//!
let udSDKJS_GetColourBuffer;

//!
//! Returns the internal depth buffer if created using `0` in `udSDKJS_ResizeScene()`
//!
//!   buffer udSDKJS_GetDepthBuffer()
//!
//! @return {number} A pointer to the internal depth buffer.
//!
let udSDKJS_GetDepthBuffer;

//!
//! Sets the item in slotID (returned from udSDKJS_RenderQueueAddModel()) to render colour - the default setting
//!
//!   errorCode udSDKJS_RenderQueueItem_SetColour(slotID)
//!
//! @param slotID {number} The slot ID of the model in the render queue.
//! @return {number} An error code based on the result of setting the render mode.
//!
let udSDKJS_RenderQueueItem_SetColour;

//!
//! Sets the item in slotID (returned from udSDKJS_RenderQueueAddModel()) to render intensity
//!
//!   errorCode udSDKJS_RenderQueueItem_SetIntensity(slotID, maxIntensity)
//!
//! @param slotID {number} The slot ID of the model in the render queue.
//! @param maxIntensity {number} Sets the maximum intensity. Values above this will be rendered white.
//! @return {number} An error code based on the result of setting the render mode.
//!
let udSDKJS_RenderQueueItem_SetIntensity;

//!
//! Sets the item in slotID (returned from udSDKJS_RenderQueueAddModel()) to render classification
//!
//!   errorCode udSDKJS_RenderQueueItem_SetClassification(slotID)
//!
//! @param slotID {number} The slot ID of the model in the render queue.
//! @return {number} An error code based on the result of setting the render mode.
//!
let udSDKJS_RenderQueueItem_SetClassification;

//!
//! Applies a highlight shader to the model
//!
//!   errorCode udSDKJS_RenderQueueItem_ToggleHighlight(slotID, isOn)
//!
//! @param slotID {number} The slot ID of the model in the render queue.
//! @param isOn {bool} true turn highight on, false off.
//! @return {number} An error code based on the result of setting the highlight.
//!
function udSDKJS_RenderQueueItem_ToggleHighlight(slotID, isOn) {
  udSDKJS_RenderQueueItem_ToggleHighlightInternal(slotID, isOn ? 1 : 0);
}

//!
//! Applies a highlight shader to the model
//!
//!   errorCode udSDKJS_RenderQueueItem_SetHighlightColour(red, green, blue, mix)
//!
//! @param red {number} The amount of red in the highlight colour: [0 - 255]
//! @param green {number} The amount of green in the highlight colour: [0 - 255]
//! @param blue {number} The amount of blue in the highlight colour: [0 - 255]
//! @param mix {number} The strength of the highlight colour: [0 - 1]
//! @return {number} An error code based on the result of setting the render mode.
//!
let udSDKJS_RenderQueueItem_SetHighlightColour;

//!
//! Applies a grey scale shader to the model
//!
//!   errorCode udSDKJS_RenderQueueItem_ToggleGreyScale(slotID, isOn)
//!
//! @param slotID {number} The slot ID of the model in the render queue.
//! @param isOn {bool} true to turn grey scale on, false off.
//! @return {number} An error code based on the result of setting the grey scale.
//!
function udSDKJS_RenderQueueItem_ToggleGreyScale(slotID, isOn) {
  udSDKJS_RenderQueueItem_ToggleGreyScaleInternal(slotID, isOn ? 1 : 0);
}

//---------------------------------------------------------------------------------------------
// Project Management
//---------------------------------------------------------------------------------------------

//!
//! Load a udServer project (legacy), or a Euclideon udCloud scene. 
//!
//!   errorCode udSDKJS_ServerProjectLoad(projectUUID, groupUUID)
//!
//! @param projectUUID {number} The ID for the udCloud scene or udServer project (legacy) that is being requested.
//! @param groupUUID {number} The ID for the workspace/project for udCloud projects (null for udServer projects).
//! @return {Promise} The created Promise.
//! @note Only one project can be loaded at any one time, and must be released with udSDKJS_ServerProjectRelease().
//!
function udSDKJS_ServerProjectLoad(projectUUID, groupUUID) {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'v');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_ServerProjectLoadInternal(projectUUID, groupUUID, onSuccessPtr, onFailurePtr);
  });
}

//!
//! Release the currently loaded project.
//!
//!   errorCode udSDKJS_ServerProjectRelease()
//!
//! @return {number} An error code based on the result of releasing the project.
//!
let udSDKJS_ServerProjectRelease;

//!
//! Export a project to where it was loaded from.
//!
//!   errorCode udSDKJS_ServerProjectSave()
//!
//! @return {number} An error code based on the result of saving the project.
//! @note {number} You can only save projects if you have connected with udSDKJS_CreateShared() or udSDKJS_CreateFrom_udCloud().
//!
function udSDKJS_ServerProjectSave() {
  return new Promise(function (onSuccess, onFailure) {
    let onSuccessPtr = Module.addFunction(onSuccess, 'v');
    let onFailurePtr = Module.addFunction(onFailure, 'vi');
    udSDKJS_ServerProjectSaveInternal(onSuccessPtr, onFailurePtr);
  });
}

//!
//! Move a node to reorder within the current parent or move to a different parent.
//!
//!   errorCode udSDKJS_ProjectNodeMoveChild(currentParent, newParent, node, insertBeforeChild)
//!
//! @param currentParent {udSDKJS_ProjectNode} The current parent of node.
//! @param newParent {udSDKJS_ProjectNode} The intended new parent of node.
//! @param node {udSDKJS_ProjectNode} The node to move.
//! @param insertBeforeChild {udSDKJS_ProjectNode} The node that will be after the node after the move. Set as 0 to be the last child of newParent.
//! @return {number} A udError value based on the result of the move.
//!
function udSDKJS_ProjectNodeMoveChild(currentParent, newParent, node, insertBeforeChild) {
  return udSDKJS_ProjectNodeMoveChildInternal(currentParent.rawPointer, newParent.rawPointer, node.rawPointer, insertBeforeChild.rawPointer);
}

//!
//! Remove a node from the project.
//!
//!   errorCode udSDKJS_ProjectNodeRemoveChild(parent, node)
//!
//! @param parentNode {udSDKJS_ProjectNode} The parent of the node to be removed.
//! @param node {udSDKJS_ProjectNode} The node to remove from the project.
//! @return {number} A udError value based on the result of removing the node.
//!
function udSDKJS_ProjectNodeRemoveChild(parent, node) {
  return udSDKJS_ProjectNodeRemoveChildInternal(parent.rawPointer, node.rawPointer);
}

//!
//! Get the root node of the currently loaded project.
//!
//!   node udSDKJS_GetProjectRoot()
//!
//! @return {udSDKJS_ProjectNode} The root node object, or null if failed to get root node.
//!
function udSDKJS_GetProjectRoot() {
  let ptr = udSDKJS_GetProjectRootInternal();
  return ptr != 0 ? new udSDKJS_ProjectNode(ptr) : null;
}

//!
//! Create a node in the currently loaded project.
//!
//!   node udSDKJS_GetProjectRoot(parent, type, name, URI)
//!
//! @param parentNode {udSDKJS_ProjectNode} The parent of the node to be created.
//! @param type {string} The node type. See '# Node types' section for listing.
//! @param name {string} A human readable name for the item. If this item is NULL it will attempt to generate a name from the pURI or the pType strings.
//! @param URI {string} The URL, filename or other URI containing where to find this item. These should be absolute paths where applicable (preferably HTTPS) to ensure maximum interop with other packages. Can be null.
//! @return {udSDKJS_ProjectNode} The root node object, or null if failed to create the node.
//!
function udSDKJS_ProjectNodeCreate(parent, type, name, URI = null) {
  let ptr = null;
  if (URI == null)
    ptr = udSDKJS_ProjectNodeCreateNoURIInternal(parent.rawPointer, type, name);
  else
    ptr = udSDKJS_ProjectNodeCreateInternal(parent.rawPointer, type, name, URI);
  return ptr != 0 ? new udSDKJS_ProjectNode(ptr) : null;
}

//!
//! Helper class for managing project nodes.
//! 
class udSDKJS_ProjectNode {
  #projectNodePtr = 0;
  constructor(projectNodePtr) {
    this.#projectNodePtr = projectNodePtr;
  }

  // Getter and setters

  //!
  //! (Internal use) Get the raw pointer to the udPorjectNode.
  //!
  //! @return {number} The raw pointer to the node in memory.
  //!
  get rawPointer() {
    return this.#projectNodePtr;
  }

  //!
  //! Get the node visibliity flag.
  //!
  //! @return {bool} true if the node is visible and should be drawn in the scene
  //!
  get isVisible() {
    return udSDKJS_ProjectNode_GetVisibilityInternal(this.#projectNodePtr) == 0 ? false : true;
  }

  //!
  //! Set the node visibliity flag.
  //!
  //! @param isVisible {bool} true if the node is visible and should be drawn in the scene
  //!
  set isVisible(isVisible) {
    udSDKJS_ProjectNode_SetVisibilityInternal(this.#projectNodePtr, isVisible ? 1 : 0);
  }

  //!
  //! Get the node UUID.
  //!
  //! @return {string} The ID as a string
  //!
  get UUID() {
    return udSDKJS_ProjectNode_GetUUIDInternal(this.#projectNodePtr);
  }

  //!
  //! Get the last time this node was updated.
  //!
  //! @return {number} UTC time
  //!
  get lastUpdate() {
    return udSDKJS_ProjectNode_GetLastUpdateInternal(this.#projectNodePtr);
  }

  //!
  //! Get the type of this node.
  //!
  //! @return {number} The id of the node type, see udSceneNodeType for more information.
  //!
  get itemtype() {
    return udSDKJS_ProjectNode_GetItemTypeInternal(this.#projectNodePtr);
  }

  //!
  //! Get the type of this node.
  //!
  //! @return {string} The id of the node type, see udSceneNodeType for more information.
  //!
  get itemtypeStr() {
    return udSDKJS_ProjectNode_GetItemTypeStrInternal(this.#projectNodePtr);
  }

  //!
  //! Get the type of this node.
  //!
  //! @return {string} The id of the node type, see udSceneNodeType for more information.
  //!
  get name() {
    return udSDKJS_ProjectNode_GetNameInternal(this.#projectNodePtr);
  }

  //!
  //! Set the name of this node.
  //!
  //! @param name {string} The node name
  //!
  set name(name) {
    udSDKJS_ProjectNode_SetNameInternal(this.#projectNodePtr, name);
  }

  //!
  //! Get the URI of this node.
  //!
  //! @return {string} The node URI
  //!
  get uri() {
    return udSDKJS_ProjectNode_GetURIInternal(this.#projectNodePtr);
  }

  //!
  //! Set the name of this node.
  //!
  //! @param name {string} The new node URI
  //!
  set uri(uri) {
    udSDKJS_ProjectNode_SetURIInternal(this.#projectNodePtr, uri);
  }

  //!
  //! Query if the node has a bounding box
  //!
  //! @return {bool} true if the node has a bounding box, false otherwise
  //!
  get hasBoundingBox() {
    return udSDKJS_ProjectNode_GetHasBoundingBoxInternal(this.#projectNodePtr) == 0 ? false : true;
  }

  //!
  //! Retrieve the node bounding box.
  //!
  //! @return {object} null if the node does not have a bounding box, otherwise will return the following object:
  //!                  { minPoint: { x, y, z },
  //!                    maxPoint: { x, y, z } }
  //!
  get boundingBox() {
    if (!this.hasBoundingBox)
      return null;

    let offset = udSDKJS_ProjectNode_GetBoundingBoxInternal(this.#projectNodePtr);
    return {
      minPoint: { x: getValue(offset, "double"), y: getValue(offset + 8, "double"), z: getValue(offset + 16, "double") },
      maxPoint: { x: getValue(offset + 24, "double"), y: getValue(offset + 32, "double"), z: getValue(offset + 48, "double") }
    };
  }

  //!
  //! Retrieve the geometry type of the node
  //!
  //! @return {number} A code representing the geometry type. See udScene.h to interpret this code.
  //!
  get geomtype() {
    return udSDKJS_ProjectNode_GetGeomTypeInternal(this.#projectNodePtr);
  }

  //!
  //! How many geometry items can be found on this node
  //!
  //! @return {number} The number of 3D points associated with the model
  //!
  get geomCount() {
    return udSDKJS_ProjectNode_GetGeomCountInternal(this.#projectNodePtr);
  }

  //!
  //! Get the geometry coordinates associated with the node
  //!
  //! @return {object} An array of {x, y, z} coordinates
  //!
  get coordinates() {
    let offset = udSDKJS_ProjectNode_GetCoordinatesInternal(this.#projectNodePtr);
    let ary = [];
    let count = udSDKJS_ProjectNode_GetGeomCountInternal(this.#projectNodePtr);
    for (let i = 0; i < count; i++) {
      ary.push({ x: getValue(offset, "double"), y: getValue(offset + 8, "double"), z: getValue(offset + 16, "double") });
      offset = offset + 24;
    }
    return ary;
  }

  //!
  //! Set the geometry coordinates for this node.
  //!
  //! @param coordArray {object} An array of {x, y, z} coordinates
  //! @param geomType {string} Geometry type. Options are "Point", "MultiPoint", "LineString", "MultiLineString", "Polygon".
  //!                          For more information, see udScene.h : udSceneGeometryType
  //!
  SetGeometry(coordArray, geomType) {
    let len = coordArray.length;
    let ptr = Module._malloc(len * 3 * 8);
    let ptrBegin = ptr;
    for (let i = 0; i < len; i++) {
      Module.setValue(ptr, coordArray[i].x, 'double');
      ptr = ptr + 8;
      Module.setValue(ptr, coordArray[i].y, 'double');
      ptr = ptr + 8;
      Module.setValue(ptr, coordArray[i].z, 'double');
      ptr = ptr + 8;
    }
    let code = udSDKJS_ProjectNodeSetGeometryInternal(this.#projectNodePtr, geomType, len, ptrBegin);
    Module._free(ptrBegin);
    return code;
  }

  //!
  //! Get the node parent.
  //!
  //! @return {udSDKJS_ProjectNode} null if this node is the project root, parent node otherwise
  //!
  get parent() {
    let ptr = udSDKJS_ProjectNode_GetParentInternal(this.#projectNodePtr);
    return ptr != 0 ? new udSDKJS_ProjectNode(ptr) : null;
  }

  //!
  //! Get the next sibling of this node.
  //!
  //! @return {udSDKJS_ProjectNode} null if there is no next sibling, sibling node otherwise
  //!
  get nextSibling() {
    let ptr = udSDKJS_ProjectNode_GetNextSiblingInternal(this.#projectNodePtr);
    return ptr != 0 ? new udSDKJS_ProjectNode(ptr) : null;
  }

  //!
  //! Get the first child of this node.
  //!
  //! @return {udSDKJS_ProjectNode} null if there are no children, child node otherwise
  //!
  get firstChild() {
    let ptr = udSDKJS_ProjectNode_GetFirstChildInternal(this.#projectNodePtr);
    return ptr != 0 ? new udSDKJS_ProjectNode(ptr) : null;
  }

  //!
  //! NOT IMPLEMENTED. When a project node is deleted, this function is called first
  //!
  get userDataCleanupCallback() {
    // TODO: Create setter for this to support collab projects
    return udSDKJS_ProjectNode_GetUserDataCleanupCallbackInternal(this.#projectNodePtr);
  }

  //!
  //! Get a raw pointer to the user data associated with this node.
  //!
  //! @return {number} Pointer to the data.
  //!
  get userData() {
    return udSDKJS_ProjectNode_GetUserDataInternal(this.#projectNodePtr);
  }

  //!
  //! Get a metadata item of the node as an integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {number} The value to return if the metadata item isn't in the node or if it isn't of an integer type.
  //! @return {number} The metadata value
  //!
  GetMetadataInt(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataIntInternal(this.#projectNodePtr, key, defaultValue);
  }

  //!
  //! Set a metadata item of the node as an integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {number} The value to write to the metadata key
  //!
  SetMetadataInt(key, value) {
    udSDKJS_ProjectNode_SetMetadataIntInternal(this.#projectNodePtr, key, value);
  }

  //!
  //! Get a metadata item of the node as an unsigned integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {number} The value to return if the metadata item isn't in the node or if it isn't of an unsigned integer type.
  //! @return {number} The metadata value
  //!
  GetMetadataUint(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataUintInternal(this.#projectNodePtr, key, defaultValue);
  }

  //!
  //! Set a metadata item of the node as an unsigned integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {number} The value to write to the metadata key
  //!
  SetMetadataUint(key, value) {
    udSDKJS_ProjectNode_SetMetadataUintInternal(this.#projectNodePtr, key, value);
  }

  //!
  //! Get a metadata item of the node as a 64-bit integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {number} The value to return if the metadata item isn't in the node or if it isn't of a 64-bit integer type.
  //! @return {number} The metadata value
  //!
  GetMetadataInt64(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataInt64Internal(this.#projectNodePtr, key, defaultValue);
  }

  //!
  //! Set a metadata item of the node as a 64-bit integer.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {number} The value to write to the metadata key
  //!
  SetMetadataInt64(key, value) {
    udSDKJS_ProjectNode_SetMetadataInt64Internal(this.#projectNodePtr, key, value);
  }

  //!
  //! Get a metadata item of the node as a 64-bit floating point number.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {number} The value to return if the metadata item isn't in the node or if it isn't of a 64-bit floating point number.
  //! @return {number} The metadata value
  //!
  GetMetadataDouble(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataDoubleInternal(this.#projectNodePtr, key, defaultValue);
  }

  //!
  //! Set a metadata item of the node as a double.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {number} The value to write to the metadata key
  //!
  SetMetadataDouble(key, value) {
    udSDKJS_ProjectNode_SetMetadataDoubleInternal(this.#projectNodePtr, key, value);
  }

  //!
  //! Get a metadata item of the node as a boolean.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {bool} The value to return if the metadata item isn't in the node or if it isn't of a boolean type.
  //! @return {number} The metadata value
  //!
  GetMetadataBool(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataBoolInternal(this.#projectNodePtr, key, defaultValue ? 1 : 0) == 0 ? false : true;
  }

  //!
  //! Set a metadata item of the node as a bool.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {bool} The value to write to the metadata key
  //!
  SetMetadataBool(key, value) {
    udSDKJS_ProjectNode_SetMetadataBoolInternal(this.#projectNodePtr, key, value ? 1 : 0);
  }

  //!
  //! Get a metadata item of the node as a string.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param defaultValue {string} The value to return if the metadata item isn't in the node or if it isn't of a string type.
  //! @return {number} The metadata value
  //!
  GetMetadataString(key, defaultValue) {
    return udSDKJS_ProjectNode_GetMetadataStringInternal(this.#projectNodePtr, key, defaultValue);
  }

  //!
  //! Set a metadata item of the node from a string.
  //!
  //! @param key {string} The name of the metadata key.
  //! @param value {string} The value to write to the metadata key
  //!
  SetMetadataString(key, value) {
    udSDKJS_ProjectNode_SetMetadataStringInternal(this.#projectNodePtr, key, value);
  }

  //!
  //! Iteration helper.
  //!
  //! let rootNode = udSDKJS_GetProjectRoot();
  //! let count = 0;
  //! rootNode.forEachChild((child) => { count++; });
  //!
  forEachChild(callback) {
    var child = this.firstChild;
    while (child != null) {
      callback(child);
      child = child.nextSibling;
    }
  }

  //!
  //! Iteration helpers
  //!
  //! let rootNode = udSDKJS_GetProjectRoot();
  //! let count = 0;
  //! for (child of rootNode.children) {
  //!   count++;
  //! }
  //!
  *children() {
    var child = this.firstChild;
    while (child != null) {
      yield child;
      child = child.nextSibling;
    }
  }
}

//---------------------------------------------------------------------------------------------
// Utility
//---------------------------------------------------------------------------------------------

//!
//! Get the name and descripton from an error code.
//!
//! errorString udSDKJS_GetErrorString(code)
//!
//! @param code {number} The error code.
//! @return {string} A string representing the type of error.
//!
function udSDKJS_GetErrorString(code) {
  if (code > 0)
    return "Not an error code";
  return udSDKJS_GetErrorStringInternal(-code);
}

//---------------------------------------------------------------------------------------------
// Internal functions
//---------------------------------------------------------------------------------------------

let udSDKJS_GetPickedModelIDInternal;
let udSDKJS_RenderQueueItem_ToggleHighlightInternal;
let udSDKJS_RenderQueueItem_ToggleGreyScaleInternal;
let udSDKJS_GetVoxelDataInternal;
let udSDKJS_ProjectNodeCreateNoURIInternal;
let udSDKJS_SetModelMatrixInternal;
let udSDKJS_LoadModelInternal;
let udSDKJS_ServerProjectSaveInternal;
let udSDKJS_DestroySharedInternal;
let udSDKJS_ServerProjectLoadInternal;
let udSDKJS_RenderQueueAddModelWithMatrixInternal;
let udSDKJS_CreateFrom_udCloudInternal;
let udSDKJS_CreateSharedFrom_udCloudInternal
let udSDKJS_GetErrorStringInternal;
let udSDKJS_GetHeaderDataInternal;
let udSDKJS_GetPickDataInternal;
let udSDKJS_GetProjectRootInternal;
let udSDKJS_ProjectNodeCreateInternal;
let udSDKJS_ProjectNodeMoveChildInternal;
let udSDKJS_ProjectNodeRemoveChildInternal;
let udSDKJS_ProjectNodeSetGeometryInternal;
let udSDKJS_ProjectNode_GetVisibilityInternal;
let udSDKJS_ProjectNode_SetVisibilityInternal;
let udSDKJS_ProjectNode_GetUUIDInternal;
let udSDKJS_ProjectNode_GetLastUpdateInternal;
let udSDKJS_ProjectNode_GetItemTypeInternal;
let udSDKJS_ProjectNode_GetItemTypeStrInternal;
let udSDKJS_ProjectNode_GetNameInternal;
let udSDKJS_ProjectNode_SetNameInternal;
let udSDKJS_ProjectNode_GetURIInternal;
let udSDKJS_ProjectNode_SetURIInternal;
let udSDKJS_ProjectNode_GetHasBoundingBoxInternal;
let udSDKJS_ProjectNode_GetBoundingBoxInternal;
let udSDKJS_ProjectNode_GetGeomTypeInternal;
let udSDKJS_ProjectNode_GetGeomCountInternal;
let udSDKJS_ProjectNode_GetCoordinatesInternal;
let udSDKJS_ProjectNode_GetParentInternal;
let udSDKJS_ProjectNode_GetNextSiblingInternal;
let udSDKJS_ProjectNode_GetFirstChildInternal;
let udSDKJS_ProjectNode_GetUserDataCleanupCallbackInternal;
let udSDKJS_ProjectNode_GetUserDataInternal;
let udSDKJS_ProjectNode_GetMetadataIntInternal;
let udSDKJS_ProjectNode_SetMetadataIntInternal;
let udSDKJS_ProjectNode_GetMetadataUintInternal;
let udSDKJS_ProjectNode_SetMetadataUintInternal;
let udSDKJS_ProjectNode_GetMetadataInt64Internal;
let udSDKJS_ProjectNode_SetMetadataInt64Internal;
let udSDKJS_ProjectNode_GetMetadataDoubleInternal;
let udSDKJS_ProjectNode_SetMetadataDoubleInternal;
let udSDKJS_ProjectNode_GetMetadataBoolInternal;
let udSDKJS_ProjectNode_SetMetadataBoolInternal;
let udSDKJS_ProjectNode_GetMetadataStringInternal;
let udSDKJS_ProjectNode_SetMetadataStringInternal;

function udSDKJS_RegisterShared()
{
  udSDKJS_GetErrorStringInternal = Module.cwrap('udSDKJS_GetErrorString', 'string', ['number']);
  udSDKJS_CreateSharedFrom_udCloudInternal = Module.cwrap('udSDKJS_CreateSharedFrom_udCloud', '', ['string', 'number', 'number']);
  udSDKJS_CreateShared = Module.cwrap('udSDKJS_CreateShared', 'number', ['string', 'string', 'string'])
  udSDKJS_CreateSharedFromDomain = Module.cwrap('udSDKJS_CreateSharedFromDomain', 'number', ['string'])
  udSDKJS_CreateFrom_udCloudInternal = Module.cwrap('udSDKJS_CreateFrom_udCloud', 'number', ['string', 'number', 'number']);

  udSDKJS_DestroySharedInternal = Module.cwrap('udSDKJS_DestroyShared', '', ['number', 'number'])

  udSDKJS_SetServerAddress = Module.cwrap('udSDKJS_SetServerAddress', 'number', ['string'])

  udSDKJS_GetProjectRootInternal = Module.cwrap('udSDKJS_GetProjectRoot', 'number', []);
  udSDKJS_ServerProjectLoadInternal = Module.cwrap('udSDKJS_ServerProjectLoad', '', ['string', 'string', 'number', 'number']);
  udSDKJS_ServerProjectRelease = Module.cwrap('udSDKJS_ServerProjectRelease', 'number', []);
  udSDKJS_ServerProjectSaveInternal = Module.cwrap('udSDKJS_ServerProjectSave', '', ['number', 'number']);
  udSDKJS_ProjectNodeCreateNoURIInternal = Module.cwrap('udSDKJS_ProjectNodeCreateNoURI', 'number', ['number', 'string', 'string']);
  udSDKJS_ProjectNodeCreateInternal = Module.cwrap('udSDKJS_ProjectNodeCreate', 'number', ['number', 'string', 'string', 'string']);
  udSDKJS_ProjectNodeMoveChildInternal = Module.cwrap('udSDKJS_ProjectNodeMoveChild', 'number', ['number', 'number', 'number', 'number']);
  udSDKJS_ProjectNodeRemoveChildInternal = Module.cwrap('udSDKJS_ProjectNodeRemoveChild', 'number', ['number', 'number']);
  udSDKJS_ProjectNodeSetGeometryInternal = Module.cwrap('udSDKJS_ProjectNodeSetGeometry', 'number', ['number', 'string', 'number', 'number']);

  udSDKJS_ProjectNode_GetVisibilityInternal= Module.cwrap('udSDKJS_ProjectNode_GetVisibility', 'number', ['number']);
  udSDKJS_ProjectNode_SetVisibilityInternal= Module.cwrap('udSDKJS_ProjectNode_SetVisibility', 'number', ['number', 'number']);
  udSDKJS_ProjectNode_GetUUIDInternal= Module.cwrap('udSDKJS_ProjectNode_GetUUID', 'string', ['number']);
  udSDKJS_ProjectNode_GetLastUpdateInternal= Module.cwrap('udSDKJS_ProjectNode_GetLastUpdate', 'number', ['number']);
  udSDKJS_ProjectNode_GetItemTypeInternal= Module.cwrap('udSDKJS_ProjectNode_GetItemType', 'number', ['number']);
  udSDKJS_ProjectNode_GetItemTypeStrInternal= Module.cwrap('udSDKJS_ProjectNode_GetItemTypeStr', 'string', ['number']);
  udSDKJS_ProjectNode_GetNameInternal= Module.cwrap('udSDKJS_ProjectNode_GetName', 'string', ['number']);
  udSDKJS_ProjectNode_SetNameInternal= Module.cwrap('udSDKJS_ProjectNode_SetName', 'int', ['number', 'string']);
  udSDKJS_ProjectNode_GetURIInternal= Module.cwrap('udSDKJS_ProjectNode_GetURI', 'string', ['number']);
  udSDKJS_ProjectNode_SetURIInternal= Module.cwrap('udSDKJS_ProjectNode_SetURI', 'int', ['number', 'string']);
  udSDKJS_ProjectNode_GetHasBoundingBoxInternal= Module.cwrap('udSDKJS_ProjectNode_GetHasBoundingBox', 'number', ['number']);
  udSDKJS_ProjectNode_GetBoundingBoxInternal= Module.cwrap('udSDKJS_ProjectNode_GetBoundingBox', 'number', ['number']);
  udSDKJS_ProjectNode_GetGeomTypeInternal= Module.cwrap('udSDKJS_ProjectNode_GetGeomType', 'number', ['number']);
  udSDKJS_ProjectNode_GetGeomCountInternal= Module.cwrap('udSDKJS_ProjectNode_GetGeomCount', 'number', ['number']);
  udSDKJS_ProjectNode_GetCoordinatesInternal= Module.cwrap('udSDKJS_ProjectNode_GetCoordinates', 'number', ['number']);
  udSDKJS_ProjectNode_GetParentInternal= Module.cwrap('udSDKJS_ProjectNode_GetParent', 'number', ['number']);
  udSDKJS_ProjectNode_GetNextSiblingInternal= Module.cwrap('udSDKJS_ProjectNode_GetNextSibling', 'number', ['number']);
  udSDKJS_ProjectNode_GetFirstChildInternal= Module.cwrap('udSDKJS_ProjectNode_GetFirstChild', 'number', ['number']);
  udSDKJS_ProjectNode_GetUserDataCleanupCallbackInternal= Module.cwrap('udSDKJS_ProjectNode_GetUserDataCleanupCallback', 'number', ['number']);
  udSDKJS_ProjectNode_GetUserDataInternal= Module.cwrap('udSDKJS_ProjectNode_GetUserData', 'number', ['number']);

  udSDKJS_ProjectNode_GetMetadataIntInternal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataInt', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_SetMetadataIntInternal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataInt', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_GetMetadataUintInternal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataUint', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_SetMetadataUintInternal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataUint', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_GetMetadataInt64Internal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataInt64', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_SetMetadataInt64Internal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataInt64', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_GetMetadataDoubleInternal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataDouble', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_SetMetadataDoubleInternal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataDouble', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_GetMetadataBoolInternal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataBool', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_SetMetadataBoolInternal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataBool', 'number', ['number', 'string', 'number']);
  udSDKJS_ProjectNode_GetMetadataStringInternal= Module.cwrap('udSDKJS_ProjectNode_GetMetadataString', 'string', ['number', 'string', 'string']);
  udSDKJS_ProjectNode_SetMetadataStringInternal= Module.cwrap('udSDKJS_ProjectNode_SetMetadataString', 'number', ['number', 'string', 'string']);

  udSDKJS_LoadModelInternal = Module.cwrap('udSDKJS_LoadModel', '', ['string', 'number', 'number']);
  udSDKJS_RenderQueueAddModelWithMatrixInternal = Module.cwrap('udSDKJS_RenderQueueAddModelWithMatrix', 'number', ['number', 'number']);
  udSDKJS_ReleaseModel = Module.cwrap('udSDKJS_ReleaseModel', 'number', ['number']);
  udSDKJS_RenderQueueRemoveItem = Module.cwrap('udSDKJS_RenderQueueRemoveItem', '', ['number']);

  udSDKJS_GetHeaderDataInternal= Module.cwrap('udSDKJS_GetHeaderData', 'string', ['number']);

  udSDKJS_GetPickedModelIDInternal = Module.cwrap('udSDKJS_GetPickedModelID', 'number', []);
  udSDKJS_SetPick = Module.cwrap('udSDKJS_SetPick', 'number', ['number', 'number']);
  udSDKJS_GetPickDataInternal = Module.cwrap('udSDKJS_GetPickData', 'number', []);
  udSDKJS_GetVoxelDataInternal = Module.cwrap('udSDKJS_GetVoxelData', 'number', ['number', 'number']);
  udSDKJS_ResizeScene = Module.cwrap('udSDKJS_ResizeScene', 'number', ['number', 'number', 'number', 'number']);
  udSDKJS_SetMatrix = Module.cwrap('udSDKJS_SetMatrix', 'number', ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']);
  udSDKJS_SetModelMatrixInternal = Module.cwrap('udSDKJS_SetModelMatrix', 'number', ['number', 'number']);

  udSDKJS_RenderQueueAddModel = Module.cwrap('udSDKJS_RenderQueueAddModel', 'number', ['number', 'number', 'number']); // Model, Elevation, Zone (-1 for Esri ECEF, 0 for Local)
  udSDKJS_RenderQueueClear = Module.cwrap('udSDKJS_RenderQueueClear', 'number', []);
  udSDKJS_RenderQueue = Module.cwrap('udSDKJS_RenderQueue', 'number', []);

  udSDKJS_GetColourBuffer = Module.cwrap('udSDKJS_GetColourBuffer', 'array', []);
  udSDKJS_GetDepthBuffer = Module.cwrap('udSDKJS_GetDepthBuffer', 'array', []);

  udSDKJS_RenderQueueItem_SetColour = Module.cwrap('udSDKJS_RenderQueueItem_SetColour', 'number', ['number']);
  udSDKJS_RenderQueueItem_SetIntensity = Module.cwrap('udSDKJS_RenderQueueItem_SetIntensity', 'number', ['number', 'number']);
  udSDKJS_RenderQueueItem_SetClassification = Module.cwrap('udSDKJS_RenderQueueItem_SetClassification', 'number', ['number']);
  udSDKJS_RenderQueueItem_SetHighlightColour = Module.cwrap('udSDKJS_RenderQueueItem_SetHighlightColour', 'number', ['number', 'number', 'number', 'number']);
  udSDKJS_RenderQueueItem_ToggleGreyScaleInternal = Module.cwrap('udSDKJS_RenderQueueItem_ToggleGreyScale', 'number', ['number', 'number']);
  udSDKJS_RenderQueueItem_ToggleHighlightInternal = Module.cwrap('udSDKJS_RenderQueueItem_ToggleHighlight', 'number', ['number', 'number']);
}
