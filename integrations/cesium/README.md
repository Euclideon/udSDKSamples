## CesiumJS

<!-- TODO: Write a brief abstract explaining this sample -->
Demonstration of integrating the udSDK JS WASM Module into Cesium JS.

<!-- TODO: Fill this section below with metadata about this sample-->
```
Language:              Javascript / HTML
Type:                  Integration
Contributor:           Euclideon Development Team <support@euclideon.com>
Organization:          Euclideon, https://euclideon.com
Date:                  2020-08-02
udSDK Version:         2.0.0
Toolsets:
```

## Resources Required
<!-- TODO: Fill this section below with the resources required to do this sample-->
1. A basic webserver (needs to deliver WASM with the correct MIMETYPE).
2. A Euclideon udStream Account

## How to use the sample
<!-- TODO: Explain how this sample can be used and what is required to get it running -->
1. Put the following files in your web server:
  a. The CesiumEuclideonIntegration.html file from this repository
  b. The contents of the udSDK JS/WASM module into the "euclideon" folder.
  c. The Cesium JS package into the "Cesium" folder
2. Load the CesiumEuclideonIntegration.html file in your brower.
3. Login to the login box.
4. Observe the models in the viewer.
  - Photogrammetry model of the Brisbane and Gold Coast regions courtesy of [Aerometrex](https://aerometrex.com.au/).
  - LiDAR of Melbourne available as [Open Data](https://data.melbourne.vic.gov.au/City-Council/City-of-Melbourne-3D-Point-Cloud-2018/2dqj-9ydd).
  - LiDAR of Vancouver available as [Open Data](https://opendata.vancouver.ca/pages/home/).

## How this Integration works
<!-- TODO: Explain the design steps used in this sample -->

1. Start with the basic sandbox Cesium scene
1. Add a Screen Space quad based on the [custom primitive example](https://github.com/cesiumlab/cesium-custom-primitive)
   1. In the shader, add 2 texture/sampler sets; one will be the colour buffer, one will be the depth buffer
   1. Set up the custom primitive to upload a random colour every frame to test upload support
1. Add the udSDKJS Module
   1. Add login box to the sceen
   1. Identify the geospatial zone used in Cesium (default is EPSG:4978)
1. In the custom primitive:
   1. Render the UDS files in the Update function
   1. Upload the colour and depth buffer
   1. Resize the textures when the screen changes (update both the GPU buffer and the udSDKJS buffers)

## Notes & Limitations
1. Due to format limitations in WebGL (not present in WebGL2) the Depth buffer has to be uploaded as RGBA8 and unpacked as floating point value in the shader.
1. The current integration does not handle:
   1. The buffers getting destroyed if the WebGL context goes out of scope
   1. There is no alert to the user if a UDS file does not load correctly.
   1. There is a known issue in udSDK-JS the prevents some models from being correctly rotated in ECEF mode.


<!-- End -->
