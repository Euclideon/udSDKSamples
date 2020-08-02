from ctypes import *
from enum import IntEnum, unique
import platform
import os

class udException(Exception):
    def printout(this):
        udSDKError = this.args[1]
        if (udSDKError == udError.ConnectionFailure):
            print("Could not connect to server.")
        elif (udSDKError == udError.AuthFailure):
            print("Username or Password incorrect.")
        elif (udSDKError == udError.OutOfSync):
            print("Your clock doesn't match the remote server clock.")
        elif (udSDKError == udError.SecurityFailure):
            print("Could not open a secure channel to the server.")
        elif (udSDKError == udError.ServerFailure):
            print("Unable to negotiate with server, please confirm the server address")
        elif (udSDKError != udError.Success):
            print("Error {}: {}; please consult udSDK SDK documentation".format(this.args[1],this.args[0]))

def LoadudSDK(SDKPath):
  global udSDKDLL
  try:
      udSDKDLL = CDLL(SDKPath)
  except OSError:
    print("No local udSDK shared object/dll found in current working directory, trying path in udSDK environment variable...")
    SDKPath = os.environ.get("UDSDK_HOME")
    if SDKPath==None:
        raise FileNotFoundError("Environment variable UDSDK_HOME not set, please refer to udSDK SDK documentation")
        
    if platform.system() == 'Windows':
        SDKPath +="/lib/win_x64/udSDK"

    #TODO Add support for these paths:
    #elif platform.system() == "Linux":
    #    print("Platform not supported")
        
    #elif platform.system() == "Darwin":
    #    print("Platform not supported"
    else:
        print("Platform {} not supported by this sample".format(platform.system()))
        exit()
    print("Using udSDK SDK shared object located at {}".format(SDKPath))
    udSDKDLL = CDLL(SDKPath)

@unique
class udError(IntEnum):
  Success = 0 # Indicates the operation was successful

  Failure = 1 # A catch-all value that is rarely used, internally the below values are favored
  InvalidParameter = 2 # One or more parameters is not of the expected format
  InvalidConfiguration = 3 # Something in the request is not correctly configured or has conflicting settings
  InvalidLicense = 4 # The required license isn't available or has expired
  SessionExpired = 5 # The udServer has terminated your session

  NotAllowed = 6 # The requested operation is not allowed (usually this is because the operation isn't allowed in the current state)
  NotSupported = 7 # This functionality has not yet been implemented (usually some combination of inputs isn't compatible yet)
  NotFound = 8 # The requested item wasn't found or isn't currently available
  NotInitialized = 9 # The request can't be processed because an object hasn't been configured yet

  ConnectionFailure = 10 # There was a connection failure
  MemoryAllocationFailure = 11 # udSDK wasn't able to allocate enough memory for the requested feature
  ServerFailure = 12 # The server reported an error trying to fufil the request
  AuthFailure = 13 # The provided credentials were declined (usually email or password issue)
  SecurityFailure = 14 # There was an issue somewhere in the security system- usually creating or verifying of digital signatures or cryptographic key pairs
  OutOfSync = 15 # There is an inconsistency between the internal udSDK state and something external. This is usually because of a time difference between the local machine and a remote server

  ProxyError = 16 # There was some issue with the provided proxy information (either a proxy is in the way or the provided proxy info wasn't correct)
  ProxyAuthRequired = 17 # A proxy has requested authentication

  OpenFailure = 18 # A requested resource was unable to be opened
  ReadFailure = 19 # A requested resourse was unable to be read
  WriteFailure = 20 # A requested resource was unable to be written
  ParseError = 21 # A requested resource or input was unable to be parsed
  ImageParseError = 22 # An image was unable to be parsed. This is usually an indication of either a corrupt or unsupported image format

  Pending = 23 # A requested operation is pending.
  TooManyRequests = 24 # This functionality is currently being rate limited or has exhausted a shared resource. Trying again later may be successful
  Cancelled = 25 # The requested operation was cancelled (usually by the user)
  Timeout = 26 # The requested operation timed out. Trying again later may be successful
  OutstandingReferences = 27 # The requested operation failed because there are still references to this object
  ExceededAllowedLimit = 28 # The requested operation failed because it would exceed the allowed limits (generally used for exceeding server limits like number of projects)

  Count = 29 # Internally used to verify return values

def _HandleReturnValue(retVal):
  if retVal != udError.Success:
    err = udError(retVal)
    raise udException(err.name, err.value)

@unique
class udRenderTargetMatrix(IntEnum):
  Camera = 0     # The local to world-space transform of the camera (View is implicitly set as the inverse)
  View = 1       # The view-space transform for the model (does not need to be set explicitly)
  Projection = 2 # The projection matrix (default is 60 degree LH)
  Viewport = 3   # Viewport scaling matrix (default width and height of viewport)
  Count = 4

class udAttributeSet(Structure):
  _fields_ = [("standardContent", c_uint64),
              ("count", c_uint32),
              ("allocated", c_uint32),
              ("pDescriptors", c_void_p)
              ]

class udPointCloudHeader(Structure):
  _fields_ = [("scaledRange", c_double),
              ("unitMeterScale", c_double),
              ("totalLODLayers", c_uint32),
              ("convertedResolution", c_double),
              ("storedMatrix", c_double * 16),
              ("attributes", udAttributeSet),
              ("baseOffset", c_double * 3),
              ("pivot", c_double * 3),
              ("boundingBoxCenter", c_double * 3),
              ("boundingBoxExtents", c_double * 3)
              ]

class udRenderInstance(Structure):
    _fields_ = [("pPointCloud", c_void_p),
                ("matrix", c_double * 16),
                ("pFilter", c_void_p),
                ("pVoxelShader", c_void_p),
                ("pVoxelUserData", c_void_p)
                ]

class udContext:
  def __init__(self):
    print("LOADING!")
    self.udContext_Connect = getattr(udSDKDLL, "udContext_Connect")
    self.udContext_Disconnect = getattr(udSDKDLL, "udContext_Disconnect")
    self.context = c_void_p(0)

  def Connect(self, url, applicationName, username, password):
    _HandleReturnValue(self.udContext_Connect(byref(self.context), url.encode('utf8'), applicationName.encode('utf8'), username.encode('utf8'), password.encode('utf8')))

  def Disconnect(self):
    _HandleReturnValue(self.udContext_Disconnect(byref(self.context)))

class udRenderContext:
  def __init__(self):
    self.udRenderContext_Create = getattr(udSDKDLL, "udRenderContext_Create")
    self.udRenderContext_Destroy = getattr(udSDKDLL, "udRenderContext_Destroy")
    self.udRenderContext_Render = getattr(udSDKDLL, "udRenderContext_Render")
    self.renderer = c_void_p(0)

  def Create(self, context):
    _HandleReturnValue(self.udRenderContext_Create(context.context, byref(self.renderer)))

  def Destroy(self):
    _HandleReturnValue(self.udRenderContext_Destroy(byref(self.renderer)))

  def Render(self, renderTarget, models):
    renderInstances = (udRenderInstance * len(models))()
    for i, model in enumerate(models):
      renderInstances[i].pPointCloud = model.model
      renderInstances[i].matrix = model.header.storedMatrix
    _HandleReturnValue(self.udRenderContext_Render(self.renderer, renderTarget.renderTarget, renderInstances, len(models), c_void_p(0)))

class udRenderTarget:
  def __init__(self):
    self.udRenderTarget_Create = getattr(udSDKDLL, "udRenderTarget_Create")
    self.udRenderTarget_Destroy = getattr(udSDKDLL, "udRenderTarget_Destroy")
    self.udRenderTarget_SetTargets = getattr(udSDKDLL, "udRenderTarget_SetTargets")
    self.udRenderTarget_SetMatrix = getattr(udSDKDLL, "udRenderTarget_SetMatrix")
    self.renderTarget = c_void_p(0)

  def Create(self, context, udSDKRenderer, width, height):
    _HandleReturnValue(self.udRenderTarget_Create(context.context, byref(self.renderTarget), udSDKRenderer.renderer, width, height))

  def Destroy(self):
    _HandleReturnValue(self.udRenderTarget_Destroy(byref(self.renderTarget)))

  def SetTargets(self, colorBuffer, clearColor, depthBuffer):
    _HandleReturnValue(self.udRenderTarget_SetTargets(self.renderTarget, byref(colorBuffer), clearColor, byref(depthBuffer)))

  def SetMatrix(self, matrixType, matrix):
    cMatrix = (c_double * 16)(*matrix)
    _HandleReturnValue(self.udRenderTarget_SetMatrix(self.renderTarget, matrixType, byref(cMatrix)))

class udPointCloud:
  def __init__(self):
    self.udPointCloud_Load = getattr(udSDKDLL, "udPointCloud_Load")
    self.udPointCloud_Unload = getattr(udSDKDLL, "udPointCloud_Unload")
    self.udPointCloud_GetMetadata = getattr(udSDKDLL, "udPointCloud_GetMetadata")
    self.model = c_void_p(0)
    self.header = udPointCloudHeader()

  def Load(self, context, modelLocation):
    _HandleReturnValue(self.udPointCloud_Load(context.context, byref(self.model), modelLocation.encode('utf8'), byref(self.header)))

  def Unload(self):
    _HandleReturnValue(self.udPointCloud_Unload(byref(self.model)))

  def GetMetadata(self):
    pMetadata = c_char_p(0)
    _HandleReturnValue(self.udPointCloud_GetMetadata(self.model, byref(pMetadata)))
    return pMetadata.value.decode('utf8')

class udConvertContext:
  def __init__(self):
      self.udConvert_CreateContext = getattr(udSDKDLL, "udConvert_CreateContext")
      self.udConvert_DestroyContext  = getattr(udSDKDLL, "udConvert_DestroyContext")
      self.udConvert_SetOutputFilename = getattr(udSDKDLL, "udConvert_SetOutputFilename")
      self.udConvert_AddItem = getattr(udSDKDLL, "udConvert_AddItem")
      self.udConvert_DoConvert = getattr(udSDKDLL, "udConvert_DoConvert")
      self.convertContext = c_void_p(0)

  def Create(self, context):
      _HandleReturnValue(self.udConvert_CreateContext(context.context, byref(self.convertContext)))
  def Destroy(self):
      _HandleReturnValue(self.udConvert_DestroyContext(byref(self.convertContext)))
  def Output(self, fileName):
      _HandleReturnValue(self.udConvert_SetOutputFilename(self.convertContext, fileName.encode('utf8')))
  def AddItem(self, modelName):
      _HandleReturnValue(self.udConvert_AddItem(self.convertContext, modelName.encode('utf8')))
  def DoConvert(self):
      _HandleReturnValue(self.udConvert_DoConvert(self.convertContext))
