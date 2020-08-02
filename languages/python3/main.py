import udSDK

from ctypes import c_int, c_float
import os
import platform
from os.path import dirname, basename, abspath
from sys import exit
from PIL import Image
from sys import argv

# Load the SDK and fetch symbols
SDKPath = abspath("./udSDK")
udSDK.LoadudSDK(SDKPath)
udSDKDLL = udSDK.udSDKDLL
    

modelFile = abspath("../../samplefiles/DirCube.uds")
outFile = abspath("./tmp.png")

appName = "PythonSample"
serverPath = "https://udstream.euclideon.com"
userName = ""
userPass = ""



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
    udContext = udSDK.udContext()
    udRenderer = udSDK.udRenderContext()
    udRenderTarget = udSDK.udRenderTarget()
    udModel = udSDK.udPointCloud()

    try:
      #initialize
      udContext.Connect(serverPath, appName, userName, userPass)
      udRenderer.Create(udContext)
      udRenderTarget.Create(udContext, udRenderer, width, height)
      udModel.Load(udContext, modelFile)
      udRenderTarget.SetTargets(colourBuffer, 0, depthBuffer)
      udRenderTarget.SetMatrix(udSDK.udRenderTargetMatrix.Camera, cameraMatrix)

      models = [udModel]

      for x in range(10):
        udRenderer.Render(udRenderTarget, models)

      Image.frombuffer("RGBA", (width, height), colourBuffer, "raw", "RGBA", 0, 1).save(outFile)
      print("{0} written to the build directory.\nPress enter to exit.\n".format(basename(outFile)))

      # Exit gracefully
      udModel.Unload()
      udRenderTarget.Destroy()
      udRenderer.Destroy()
      udContext.Disconnect()
    except udSDK.udException as err:
      err.printout()
