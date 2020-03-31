import vault

from ctypes import c_int, c_float
import os
import platform
from os.path import dirname, basename, abspath
from sys import exit
from PIL import Image
from sys import argv

# Load the SDK and fetch symbols
SDKPath = abspath("./vaultSDK")
vault.LoadVaultSDK(SDKPath)
vaultSDK = vault.vaultSDK
    

modelFile = abspath("../../samplefiles/DirCube.uds")
outFile = abspath("./tmp.png")

appName = "PythonSample"
serverPath = "https://earth.vault.euclideon.com"
userName = "Username"
userPass = "Password"



width = 1280
height = 720
#array of 32 bit ARGB pixels:
colourBuffer = (c_int * width * height)()
#float depths, z' is normalized between 0 and 1
depthBuffer = (c_float * width * height)()

#the camera matrix using left handed GL convention (i.e. last row is translation)
cameraMatrix = [1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,-5,0,1]

if __name__ == "__main__":
    
    if len(argv) >= 3:
        userName = argv[1]
        userPass = argv[2]

    if len(argv) >= 4:
        modelFile = abspath(argv[3])

    # Do the thing
    vaultContext = vault.vdkContext()
    vaultRenderer = vault.vdkRenderContext()
    vaultRenderView = vault.vdkRenderView()
    vaultModel = vault.vdkPointCloud()

    try:
      #initialize
      vaultContext.Connect(serverPath, appName, userName, userPass)
      vaultContext.RequestLicense(vault.vdkLicenseType.Render)
      vaultRenderer.Create(vaultContext)
      vaultRenderView.Create(vaultContext, vaultRenderer, width, height)
      vaultModel.Load(vaultContext, modelFile)
      vaultRenderView.SetTargets(colourBuffer, 0, depthBuffer)
      vaultRenderView.SetMatrix(vault.vdkRenderViewMatrix.Camera, cameraMatrix)

      models = [vaultModel]

      for x in range(10):
        vaultRenderer.Render(vaultRenderView, models)

      Image.frombuffer("RGBA", (width, height), colourBuffer, "raw", "RGBA", 0, 1).save(outFile)
      print("{0} written to the build directory.\nPress enter to exit.\n".format(basename(outFile)))

      # Exit gracefully
      vaultModel.Unload()
      vaultRenderView.Destroy()
      vaultRenderer.Destroy()
      vaultContext.Disconnect()
    except vault.VdkException as err:
      err.printout()
