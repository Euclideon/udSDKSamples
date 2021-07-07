const udsModels = [
  // great barrier reef 100m
  { key: "Great Barrier Reef", url: "https://esrigeoaus.blob.core.windows.net/demodata/Great-Barrier-Reef-100m-From-GeoTIFF-crop.uds", eval: [0.0, -1] },
  // 50mm multibeam dataset 2018
  { key: "AusSeabed Marine Data", url: "https://esrigeoaus.blob.core.windows.net/demodata/All%20Oz%2050m%202018_coloured.uds", eval: [0.0, -1] },
  // 3rd data
  { key: "AusSeabed Marine Data", url: "https://esrigeoaus.blob.core.windows.net/demodata/All%20Oz%2050m%202018_coloured.uds", eval: [0.0, -1] },
  // ...add more uds models here...
];

// Global variables.
var userChosen = null;

/**
 * Check whether user has model rendered.
 * @returns boolean
 */
const isLoaded = () => {
  return (userChosen != null);
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