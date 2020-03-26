#!/usr/bin/env python3.7

#######################################################
#  A Vault Client Hello World! program in Python3.7   #
#######################################################

"""
converter.py

General purpose command line converter for Euclideon Vault using python
Requires Vault SDK shared libraries (.so or .dll)
Converts any point cloud format supported by Vault SDK to UDS format 

By default will produce one UDS for each model provided
If no models are provided the program will attempt to convert the sample file located
at ../../samplefiles/DirCube.uds

--merge produces a single uds from the input files 

output files are stored in:
    ./convertedUDS/[inputName].uds for individual mode
    ./mergedUDS/[firstInputName].uds for merge mode
    
    
Usage:
    converter username password [models] [--merge]
"""

import vault
import os
from os.path import abspath
from sys import exit
from sys import argv

#######################################################
####################### Setup #########################
#######################################################

# Load the SDK and fetch symbols
SDKPath = abspath("./vaultSDK") #this is where we will look for the dll first
vault.LoadVaultSDK(SDKPath)
vaultSDK = vault.vaultSDK

appName = "PythonSample_Convert"

#some default values; these should be overwritten by argv
modelFiles = [abspath("../../samplefiles/DirCube.uds")]
outFile = abspath("./ConvertedUDS.uds")
serverPath = "https://earth.vault.euclideon.com"
userName = "Username"
userPass = "Password"

vaultContext = vault.vdkContext()
convertContext = vault.vdkConvertContext()

def vault_login():
    """
    to vault server and requests a license

    Returns
    -------
    None.

    """
    try:
        vaultContext.Connect(serverPath, appName, userName, userPass)
        vaultContext.RequestLicense(vault.vdkLicenseType.Convert)
        convertContext.Create(vaultContext)
    except vault.VdkException as err:
        err.printout()
        exit()

def vault_logout():
        # Exit gracefully
      convertContext.Destroy()
      vaultContext.Disconnect()
  
def convert_model(modelFiles, outFile):
    """
    performs a conversion of a list of input files to the output UDS at path outfile

    Parameters
    ----------
    modelFiles : List[string]
        List of paths to files to be converted
    outFile : string
        path to the output file to be written

    Returns
    -------
    None.

    """

    try:
      
      formattedInputNames = ""
      for modelFile in modelFiles:
          vdkError = convertContext.AddItem(modelFile)
          formattedInputNames += "\t {}\n".format(modelFile)
      vdkError = convertContext.Output(outFile)
      
      print("Converting files:\n {} to {}".format(formattedInputNames,outFile))
      convertContext.DoConvert()
      #input("Conversion successful! Press Enter to continue...")
      print("done")
      
      
    except vault.VdkException as err:
      err.printout();
    

#######################################################
######################## Main #########################
#######################################################
if __name__ == "__main__":
    if len(argv)<3:
        print("Usage: {} VaultUserName VaultPassword [--merge] [inputFiles]".format(argv[0]))
    try:
        argv.remove("--merge")
        merge = True
    except ValueError:
        merge = False
    
    #mass convert, single directory, individual output files
    if len(argv) >= 3:
        userName = argv[1]
        userPass = argv[2]

    if len(argv) >= 4:
        modelFiles = argv[3:]
    else:
        print("No model specified, falling back to example uds at {}".format(modelFiles[0]))
        
    vault_login()
    
    if merge:
        outFile = abspath("./mergedUDS/"+os.path.basename(modelFiles[0])+".uds")
        convert_model(modelFiles,outFile)
    else:
        for modelFile in modelFiles:
            outFile = os.path.splitext(modelFile)[0]
            outFile = abspath("./convertedUDS/"+os.path.basename(outFile)+".uds")
            convert_model([modelFile], outFile)
    
    vault_logout()