import vault

from ctypes import c_int, c_float
import os
import platform
from os import getcwd
from os.path import dirname, basename, abspath
from sys import exit
from PIL import Image
from sys import argv

cwd = getcwd()

#Try to determine the location of the SDK shared library, 
#if the environment does not include one we fall back to looking in the 
#currrent working directory
SDKPath = os.environ.get("VAULTSDK_HOME")
if SDKPath and platform.system() == 'Windows':
    SDKPath +="/lib/win_x64/vaultSDK"
    #TODO add checks for other operating systems
else:#we assume that the .dll or .so file is in the current working directory
    SDKPath = abspath("./vaultSDK")
    

modelFile = abspath("../../samplefiles/DirCube.uds")
outFile = abspath("./tmp.png")

appName = "PythonSample"
serverPath = "https://earth.vault.euclideon.com"
userName = "Username"
userPass = "Password"

if len(argv) >= 3:
  userName = argv[1]
  userPass = argv[2]

if len(argv) >= 4:
  modelFile = abspath(argv[3])

width = 1280
height = 720

colourBuffer = (c_int * width * height)()
depthBuffer = (c_float * width * height)()

cameraMatrix = [1,0,0,0,0,1,0,0,0,0,1,0,0,-5,0,1]

if __name__ == "__main__":
    # Load the SDK and fetch symbols
    vault.LoadVaultSDK(SDKPath)

    vaultSDK = vault.vaultSDK

    # Do the thing
    vaultContext = vault.vdkContext()
    vaultRenderer = vault.vdkRenderContext()
    vaultRenderView = vault.vdkRenderView()
    vaultModel = vault.vdkPointCloud()

    try:
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
    except Exception as err:
      if len(err.args) == 2:
        vaultError = err.args[1]
        if (vaultError == vault.vdkError.ConnectionFailure):
            print("Could not connect to server.")
        elif (vaultError == vault.vdkError.AuthFailure):
            print("Username or Password incorrect.")
        elif (vaultError == vault.vdkError.OutOfSync):
            print("Your clock doesn't match the remote server clock.")
        elif (vaultError == vault.vdkError.SecurityFailure):
            print("Could not open a secure channel to the server.")
        elif (vaultError == vault.vdkError.ServerFailure):
            print("Unable to negotiate with server, please confirm the server address")
        elif (vaultError != vault.vdkError.Success):
            print("Error: ", vault.vdkError(vaultError).name)
