/*

# Using udSDKJS

Call `udSDKJS_Login` to use a username and password to authenticate or `udSDKJS_Domain` from a registered domain.

After that call the other functions will be registered for use.

Note: The server defaults to "https://udstream.euclideon.com" but can be changed using `udSDKJS_SetServerAddress(URL)`
Note: When udSDKJS is no longer required call `udSDKJS_DestroyShared` to clean up shared memory.

`udSDKJS_LoadModel(URL)` returns a handle to a model at the provided URL
`udSDKJS_ReleaseModel(handle)` returns 0 on success, release the handle previously gathered using `udSDKJS_LoadModel`

`udSDKJS_GetHeaderData(handle)` returns an object with the loaded models header data (including metadata and attribute information)

`udSDKJS_RenderQueueAddModel(handle, zOffset, targetzone)` adds the model to the render queue with the given zOffset and target zone. Return value is the slot id it was added to; negative values indicate failure
`udSDKJS_RenderQueueClear()` clears the current render queue
`udSDKJS_RenderQueue()` does the render to the provided buffers from `udSDKJS_ResizeScene` using the matrices from `udSDKJS_SetMatrix`

`udSDKJS_ResizeScene(width, height, colour buffer, depth buffer)` sets new color and depth buffers (or 0 to have udSDKJS create them) as well as sets the size of them
`udSDKJS_SetMatrix(matrix type, a0, a1 ... a15)` sets the 4x4 matrix for the given matrix type: "view", "projection", "camera", "viewport"

`udSDKJS_GetColourBuffer()` and `udSDKJS_GetDepthBuffer()` return the internal buffers if created using `0` in `udSDKJS_ResizeScene`

`udSDKJS_RenderQueueItem_SetColour(int slotID)` sets the item in slotID (returned from udSDKJS_RenderQueueAddModel) back to colour mode- the default setting (negative return if failed)
`udSDKJS_RenderQueueItem_SetIntensity(int slotID, int maxIntensity)` sets the item in slotID (returned from udSDKJS_RenderQueueAddModel) to intensity mode  (negative return if failed)
`udSDKJS_RenderQueueItem_SetClassification(int slotID)` sets the item in slotID (returned from udSDKJS_RenderQueueAddModel) to classification mode (negative return if failed)

*/

function udSDKJS_RegisterShared()
{
	udSDKJS_CreateShared = Module.cwrap('udSDKJS_CreateShared', 'number', ['string', 'string', 'string'])
	udSDKJS_CreateSharedFromDomain = Module.cwrap('udSDKJS_CreateSharedFromDomain', 'number', ['string'])
	udSDKJS_DestroyShared = Module.cwrap('udSDKJS_DestroyShared', 'number', [])

	udSDKJS_SetServerAddress = Module.cwrap('udSDKJS_SetServerAddress', 'number', ['string'])

	udSDKJS_LoadModel = Module.cwrap('udSDKJS_LoadModel', 'number', ['string']);
	udSDKJS_ReleaseModel = Module.cwrap('udSDKJS_ReleaseModel', 'number', ['number']);

	udSDKJS_GetHeaderDataI = Module.cwrap('udSDKJS_GetHeaderData', 'string', ['number']);

	udSDKJS_ResizeScene = Module.cwrap('udSDKJS_ResizeScene', 'number', ['number', 'number', 'number', 'number']);
	udSDKJS_SetMatrix = Module.cwrap('udSDKJS_SetMatrix', 'number', ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']);

	udSDKJS_RenderQueueAddModel = Module.cwrap('udSDKJS_RenderQueueAddModel', 'number', ['number', 'number', 'number']); // Model, Elevation, Zone (-1 for Esri ECEF, 0 for Local)
	udSDKJS_RenderQueueClear = Module.cwrap('udSDKJS_RenderQueueClear', 'number', []);
	udSDKJS_RenderQueue = Module.cwrap('udSDKJS_RenderQueue', 'number', []);

	udSDKJS_GetColourBuffer = Module.cwrap('udSDKJS_GetColourBuffer', 'array', []);
	udSDKJS_GetDepthBuffer = Module.cwrap('udSDKJS_GetDepthBuffer', 'array', []);

	udSDKJS_RenderQueueItem_SetColour = Module.cwrap('udSDKJS_RenderQueueItem_SetColour', 'number', ['number']);
	udSDKJS_RenderQueueItem_SetIntensity = Module.cwrap('udSDKJS_RenderQueueItem_SetIntensity', 'number', ['number', 'number']);
	udSDKJS_RenderQueueItem_SetClassification = Module.cwrap('udSDKJS_RenderQueueItem_SetClassification', 'number', ['number']);
}

function udSDKJS_GetHeaderData(model)
{
	// This is a wrapper to return an object rather than a string
	return JSON.parse(udSDKJS_GetHeaderDataI(model));
}

function udSDKJS_Login(username, password, application)
{
	return udSDKJS_CreateShared(username, password, application)
}

function udSDKJS_Domain(application)
{
	return udSDKJS_CreateSharedFromDomain(application)
}
