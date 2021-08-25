const udsModels = [
  // Melbourne Model
  { key: "Melbourne", url: "https://models.euclideon.com/Melbourne_75mm.uds", eval: [145.0, -1] },
  // 50mm multibeam dataset 2018
  { key: "Bendigo", url: "https://cesiumdemo.blob.core.windows.net/models/Bendigo_nonPrimitive_HR.uds", eval: [280.0, -1] },
  // ...add more uds models here...
];

// Global variables.
var userChosen = null;
var showModel = true;

/**
 * Check whether user has model rendered.
 * @returns boolean
 */
const isLoaded = () => {
  return (userChosen !== null);
}

/**
* Unload all UDS Models.
*/
const unloadUDSModels = () => {
  udSDKJS_RenderQueueClear(0);
}

/**
 * pickModel returns which model to load on demand
 * @param {String} modelName 
 * @returns 
 */
const getModel = (modelName) => {
  for (var i = 0; i < udsModels.length; i++) {
    if (modelName === udsModels[i].key) {
      userChosen = udsModels[i].key;
      return udsModels[i];
    }
  }
  return null;
}

/**
 * Load an UDS Model, pass an object and calling UDSDK for rendering.
 * @param {*} model 
 * @returns 
 */
const loadUDSModel = async (model) => {
  if (model === null)
    return false;
  // Do not load the model if the model is hidden by user.
  if (showModel === false)
    return false;
  let loadModel = await udSDKJS_LoadModel(model.url);
  udSDKJS_RenderQueueAddModel(loadModel, model.eval[0], model.eval[1]);
  console.log("Model Loaded: ", model.key);
};

/**
 * Load one UDS Model Only, pass an object and calling UDSDK for rendering, unload the previous one.
 * @param {*} model 
 * @returns 
 */
const unload_LoadUDSModel = async (modelName) => {
  // prevent infinity reload if the user is already in the location.
  if (userChosen === modelName)
    return false;

  var model = getModel(modelName);
  if (model === null)
    return false;

  unloadUDSModels();
  // Do not load the model if the model is hidden by user.
  if (showModel === false)
    return false;
  let loadModel = await udSDKJS_LoadModel(model.url);
  udSDKJS_RenderQueueAddModel(loadModel, model.eval[0], model.eval[1]);
  console.log("Model Loaded: ", model.key);
};

/**
 * Load all UDS Models in the list.
 */
const loadUDSModels = () => {
  let t0 = performance.now();
  // Loop replace of hard code
  udsModels.map((model) => {
    loadUDSModel(model);
  });
  let t1 = performance.now();
  console.log("Loaded " + udsModels.length + " Models took " + (t1 - t0).toFixed(2) + " ms.")
}