## Exclude Streamed Attributes From Render

<!-- TODO: Write a brief abstract explaining this sample -->
UDS files may be used to store many different properties (attributes) per point. This introduces an overhead that may be mitigated by only requesting the attributes we wish to display at render time.
This sample demonstrates loading a UDS file for rendering with some attribute streams disabled. 

<!-- TODO: Fill this section below with metadata about this sample-->
```
Language:              (C-Style) C++
Type:                  Feature Example
Contributor:           Euclideon Development Team <support@euclideon.com>
Organization:          Euclideon, https://euclideon.com
Date:                  2022-04-27
udSDK Version:         2.1.0
Toolsets:              C++11 compiler
```

## Resources Required
<!-- TODO: Fill this section below with the resources required to do this sample-->
- `HistogramTest.uds` (Provided as sample data)
- `stb_image_write.h` (Provided in `external`)

## How to use the sample
<!-- TODO: Explain how this sample can be used and what is required to get it running -->
1. Create the project in the sample root directory (see instructions there for more information)
2. Set User information in the `udSDKFeatureSamples.h` file (if updating a sample, be careful not to push these back upstream)
3. Run the sample and observe a UDS is output from the data provided in CustomConvertTest_ReadFloat

<!-- End -->
