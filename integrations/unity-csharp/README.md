# VDK Unity

This project demonstrates the use of Euclideon Vault developer Kit (VDK) with the Unity Real Time Development Platform. 

```
Language:              C#
Type:                  Integration
Contributor:           Euclideon Vault Development Team <support@euclideon.com>
Organization:          Euclideon, https://euclideon.com/vault
Date:                  2020-02-03
Vault SDK Version:     0.5.0
Toolsets:              Requires Unity >=2019.3
```

## Resources Required
Unity Postprocessing >=v2.3.0

## How to use the sample
<!-- TODO: Explain how this sample can be used and what is required to get it running -->
There are two projects included:

* This is the basic project, including a basic flight camera and collider, this demonstrates the basic concepts required to get simulations and games using VDK running in unity. This is ideal for developers wishing to use VDK with their own projects
* There is also an advanced project: including a driving simulator and third person ragdoll. This demonstrates use of VDK with Unity physics and is good for exploring and testing interactions of unity systems with Unlimited Detail point clouds.

**Both examples require a valid license for Euclideon Vault SDK, trial licenses can be obtained [here](https://zfrmz.com/gwVUru84d60yUedxmLx9/?ref=Unity%20Sample%20Code)** 

### Installation

Both examples are tested with Unity 2019.3.0f6, all steps must be repeated for each package in their respective directories.

1. Download and extract VDK 0.5.0 package from [here](https://earth.vault.euclideon.com) using your license credentials (if you do not have one, free trials are available from [here](https://zfrmz.com/gwVUru84d60yUedxmLx9/?ref=Unity%20Sample%20Code) )
2. Download the Unity VDK examples from [here](https://github.com/Euclideon/vaultsdksamples)
3. Copy the files from _Euclideon_vdk0.5.0/lib/(_your operating system here_)/_ to _vaultsdksamples/integrations/unity-csharp/Assets/VDK in your Unity project
4. Replace the username and password lines from _Assets/VDK/vdkLogin.cs_ to your vault login details 
5. Open Unity Hub, Select Add and specify the  project as the source folder; select the unity version next to the project name; click on the project name
6. Open Window \-> package manager \-> Post Processing \-> update (ensure that postprocessing packages are installed and up to date, \>v2.3.0)
7. Open the sample scene from the project, (Scenes/SampleScene in the project expolorer) press play to fly the camera in the chose scene using game view. Refer to known issues if the game does not run

### Changing UDS model

The UDS can be changed by modifying the path attribute of the Model object in the project hierarchy (by default on the left of the screen) using the inspector (by default on the right)

There is a UDS model included with this example for demonstration purposes, paths to your own model can be pasted into the _path_ field of the US model object
UDS file format developed by Euclideon allowing streamable, unlimited sized 3D datasets requiring only low spec hardware. 

Models can be created from most common photogrammetry and LiDAR point clouds file formats using Euclideon [Vault Client](https://www.euclideon.com/vault/) available [Here](https://earth.vault.euclideon.com) your vault SDK trial license also gives you access to vault client during your trial period.
You can read about the conversion process [here](https://www.euclideon.com/wp-content/uploads/2019/10/2019_10_31-Vault-Conversion-Guide-v1.2.pdf) if you have any questions check the [support knowledge base](https://www.euclideon.com/customerresourcepage/) or email support@euclideon.com

Photogrammetry model of the Gold Coast courtesy of [Aerometrex](https://aerometrex.com.au/)

## Basic Example

This is an example demonstrating how to use VDK with unity, it includes a minimalist example of a flight camera and an attached collider. 
Unlimited detail rendering is implemented as a postprocessing effect that can be applied to cameras in order to display UDS objects.

### Sample Scene Structure
![The Structure of the basic sample scene](./docs/sampleSceneStructure.png "Structure of the Sample Scene")

#### Main Camera

The main view used in the example. It has a flight camera script attached to it to enable user interation. Of importance to this object is 
the implemented post process layer and volume properties which must be included for the camera to view UDS files. 

![Sample UDS camera Settings](./docs/sampleCameraSettings.png "Sample UDS Camera Settings")

### UDS Model

Each of UDS to be loaded in unity is represented as a one of these models.

### vdkLogin

This file contains the login logic for the unity example, including login credentials. GlobalVDKContext contains a ``` vdkContext``` for managing licensing 
and login information between objects, and a ```vdkRenderContext```, enabling the rendering of and caching the UDS model information

### VDKPPER 

_VDKPPES.cs_ contains the implemention of VDK in Unity as a post processing effect. The associated shader is ```vdkShader.shader```

### VDK Collider

This object demonstrates how to achieve physical collisions between Euclideon UDS models and native Unity colliders. Because of the potential scale of UDS objects it is not practical to construct mesh colliders of UDS objects (espeially if these objects are being streamed externally)
The approach taken is to construct a mesh of the UDS object local to a point of interest (for example a player or the potential colliding object using information available from an instance of ```vdkRenderView```. 

Because the information contained in UDS files (especially unfiltered point clouds) can be noisy, we have included functionality to smooth the generated surfaces.

_VDKCollider.cs_ contains the majority of the logic associated with the example collider system.
 
![Collider Object Structure](./docs/colliderStructure.png "Collider Object Structure")

_Because Unity does not allow collisions between a parent and child object, the collider cannot be a direct decendent of an object it is intended to collide with. 
Instead use the follow target parameter to keep the collider within an objects reference frame_

The VDK Colider Script takes the following parameters:

_Follow Target:_ If not none will set the transformation of the collider to match that of the target, useful for meshing locally around 
particular objects. 

_Threshold Follow:_ Determines the rate at which the location of the collider is updated, if true the location of the collider is updated only when the object is _Follow Threshold_ distance from the watcher position

_Watcher Pos:_ The location of the virtual camera in the local frame used to generate the collision mesh. Currently this should be set to 0.5*(ZNear-Zfar) in the direction the cameara is intended to look, this is to prevent unwanted clipping

_width, height:_ the dimensions of the polygon 'sheet' draped over the UDS model in metres, smaller colliders will have a higher polygon density and therefore better collision accuracy at lower computational cost

_Z Near, Z far:_ the locations of the near and far plane along the local z axis for the 'watcher' camera used to generate the collision polygon. 

_Width Pix, Height Pix:_ The number of pixels used to find the position of the vertices in the sheet. Currently this is equal to the number of vertices in the collision mesh. Smoother results may be obtained in future versions by oversampling around the vertex locations and averaging to eliminate noise.
These values have a large impact on frame rate and should be kept as low as possible. Increasing these improves the accuracy of the produced collision mesh.

_Laplacian Smoothing:_ This determines if smoothing should be applied to the collider, this has the effect of removing noise from laser scans at the cost of potentially removing 'sharp' features from the collision mesh.

Smoothing off (note the rougher ground surface due to sensor noise):
![UnSmoothed](./docs/ColliderUnfiltered.png "Unsmoothed Collision Polygon") 

Smoothing on (note that tree branches are no longer captured by the collision model):
![Smoothed](./docs/ColliderFiltered.png "Smoothed Collision Polygon")

As the attached mesh is modified often by this script, baking options for the mesh collider should be turned off for performance reasons

## Known Issues

### Postprocessing bug on initial import
```C#
NullReferenceException: Object reference not set to an instance of an object
UnityEngine.Rendering.PostProcessing.AmbientOcclusion.IsEnabledAndSupported (UnityEngine.Rendering.PostProcessing.PostProcessRenderContext context) (at Library/PackageCache/com.unity.postprocessing@2.3.0/PostProcessing/Runtime/Effects/AmbientOcclusion.cs:182)
UnityEngine.Rendering.PostProcessing.PostProcessLayer.SetLegacyCameraFlags (UnityEngine.Rendering.PostProcessing.PostProcessRenderContext context) (at Library/PackageCache/com.unity.postprocessing@2.3.0/PostProcessing/Runtime/PostProcessLayer.cs:771)
UnityEngine.Rendering.PostProcessing.PostProcessLayer.SetupContext (UnityEngine.Rendering.PostProcessing.PostProcessRenderContext context) (at Library/PackageCache/com.unity.postprocessing@2.3.0/PostProcessing/Runtime/PostProcessLayer.cs:863)
UnityEngine.Rendering.PostProcessing.PostProcessLayer.BuildCommandBuffers () (at Library/PackageCache/com.unity.postprocessing@2.3.0/PostProcessing/Runtime/PostProcessLayer.cs:533)
UnityEngine.Rendering.PostProcessing.PostProcessLayer.OnPreCull () (at Library/PackageCache/com.unity.postprocessing@2.3.0/PostProcessing/Runtime/PostProcessLayer.cs:462)
UnityEngine.GUIUtility:ProcessEvent(Int32, IntPtr)
```

This occurs as a result of a bug in the postprocessing package, the solution is to remove the postprocessing layer from the
camera(s) and re add it with the same settings as provided. 

