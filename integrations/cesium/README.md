## Esri SceneView

<!-- TODO: Write a brief abstract explaining this sample -->
Demonstration of integrating the Vault SDK JS WASM Module into Cesium JS.

<!-- TODO: Fill this section below with metadata about this sample-->
```
Language:              Javascript / HTML
Type:                  Integration
Contributor:           Euclideon Vault Development Team <support@euclideon.com>
Organization:          Euclideon, https://euclideon.com/vault
Date:                  2020-03-02
Vault SDK Version:     0.6.0
Toolsets:
```

## Resources Required
<!-- TODO: Fill this section below with the resources required to do this sample-->
1. A basic webserver (needs to deliver WASM with the correct MIMETYPE).
2. Access to a Euclideon Vault Rendering license.

## How to use the sample
<!-- TODO: Explain how this sample can be used and what is required to get it running -->
1. Put the following files in your web server:
  a. The CesiumEuclideonIntegration.html file from this repository
  b. The contents of the Vault SDK JS/WASM module into the "euclideon" folder.
  c. The Cesium JS package into the "Cesium" folder
2. Load the CesiumEuclideonIntegration.html file in your brower.
3. Login to the login box.
4. Observe the models in the viewer.
  - Photogrammetry model of the Brisbane and Gold Coast regions courtesy of [Aerometrex](https://aerometrex.com.au/).
  - LiDAR of Melbourne available as [Open Data](https://data.melbourne.vic.gov.au/City-Council/City-of-Melbourne-3D-Point-Cloud-2018/).
  - LiDAR of Vancouver available as [Open Data](https://opendata.vancouver.ca/pages/home/).

<!-- End -->
