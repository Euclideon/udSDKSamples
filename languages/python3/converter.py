#!/usr/bin/env python3.7

#######################################################
#  A Vault Client Hello World! program in Python3.7   #
#######################################################

#######################################################
###################### Imports ########################
#######################################################
import vault

from ctypes import c_int, c_float
from os import getcwd
from os.path import dirname, basename
from sys import exit
from PIL import Image
from sys import argv

#######################################################
####################### Setup #########################
#######################################################
cwd = getcwd()
SDKPath = cwd + "\\vaultSDK"

modelFile = cwd + "\\FileToConvert.uds"
outFile = cwd + "\\ConvertedUDS.uds"

appName = "PythonSample_Convert"

serverPath = "https://earth.vault.euclideon.com"
userName = "Username"
userPass = "Password"

if len(argv) >= 3:
  userName = argv[1]
  userPass = argv[2]

if len(argv) >= 4:
  modelFile = argv[3]

#######################################################
######################## Main #########################
#######################################################
if __name__ == "__main__":
    # Load the SDK and fetch symbols
    vault.LoadVaultSDK(SDKPath)

    vaultSDK = vault.vaultSDK

    # Do the thing
    vaultContext = vault.vdkContext()
    convertContext = vault.vdkConvertContext()


    try:
      vaultContext.Connect(serverPath, appName, userName, userPass)
      vaultContext.RequestLicense(vault.vdkLicenseType.Convert)
      vdkError = convertContext.Create(vaultContext)
      vdkError = convertContext.AddItem(modelFile)
      vdkError = convertContext.Output(outFile)
      
      print("Converting...")
      convertContext.DoConvert()
      input("Conversion successful! Press Enter to continue...")
      
      # Exit gracefully
      convertContext.Destroy()
      vaultContext.Disconnect()
    except Exception as err:
      if len(err.args) == 2:
        vaultError = err.args[1]
        if (vaultError == vault.vdkError.ConnectionFailure):
            print("Could not connect to server.")
        elif (vaultError == vault.vdkError.NotAllowed):
            print("Username or Password incorrect.")
        elif (vaultError == vault.vdkError.OutOfSync):
            print("Your clock doesn't match the remote server clock.")
        elif (vaultError == vault.vdkError.SecurityFailure):
            print("Could not open a secure channel to the server.")
        elif (vaultError == vault.vdkError.ServerFailure):
            print("Unable to negotiate with server, please confirm the server address")
        elif (vaultError != vault.vdkError.Success):
            print("Unknown error occurred, please try again later.")
