## Esri SceneView

<!-- TODO: Write a brief abstract explaining this sample -->
Demonstration of integrating the udSDK JS WASM Module into Esri SceneView using the [External Renderer](https://developers.arcgis.com/javascript/latest/api-reference/esri-views-3d-externalRenderers.html) interface.

<!-- TODO: Fill this section below with metadata about this sample-->
```
Language:              Javascript / HTML
Type:                  Integration
Contributor:           Euclideon Development Team <support@euclideon.com>
Organization:          Euclideon, https://www.euclideon.com
Date:                  2021-10-14
Last Update:		   2021-10-14
udSDK Version:         2.1.0
Toolsets:			   Arcgis Javascript API 4.20
```

## Resources Required
<!-- TODO: Fill this section below with the resources required to do this sample-->
1. A basic webserver (needs to deliver WASM with the correct MIMETYPE).
2. A udStream account
3. An Arcgis Developer account.

## How to use the sample
<!-- TODO: Explain how this sample can be used and what is required to get it running -->
1. Clone the project.
2. Download udSDKJS from [Here](https://www.euclideon.com/wp-content/uploads/2021/06/udSDK_Developer_2.1.zip).
3. Copy downloaded udSDKJS Module to the root directory.
4. Download the Arcgis Javascript API from [Arcgis Development Portal](https://developers.arcgis.com/dashboard/).
5. Copy downloaded Arcgis Javascript API to the root directory.
6. You need to setup a server which has CORS Enabled (COOP & COEP) to solve cross-origin-isolate issue, visit [here](https://web.dev/coop-coep/) for more detail.
7. Login to the login box.
8. Observe the models in the viewer.
  - LiDAR of Melbourne available as [Open Data](https://data.melbourne.vic.gov.au/City-Council/City-of-Melbourne-3D-Point-Cloud-2018/2dqj-9ydd).
9. Observe the building footprint (blue buildings), traffic lights and tree canopies.
10. Click on Gear button on right-hand-side menu, you can disable certain dataset, and adjust udSDK model resolutions.

<!-- End -->
