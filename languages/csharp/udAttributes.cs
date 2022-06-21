using System;
using System.Runtime.InteropServices;

namespace Euclideon.udSDK
{
  namespace Attributes
  {
    public enum StandardAttribute
    {
      GPSTime, //!< udATI_float64 
      PrimitiveID, //!< udATI_uint32  
      ARGB, //!< udATI_color32 
      Normal, //!< udATI_normal32
      Red, //!< Legacy 16bit Red channel
      Green, //!< Legacy 16bit Green channel
      Blue, //!< Legacy 16bit Blue channel
      Intensity, //!< udATI_uint16  
      NIR, //!< udATI_uint16  
      ScanAngle, //!< udATI_uint16  
      PointSourceID, //!< udATI_uint16  
      Classification, //!< udATI_uint8   
      ReturnNumber, //!< udATI_uint8   
      NumberOfReturns, //!< udATI_uint8   
      ClassificationFlags, //!< udATI_uint8   
      ScannerChannel, //!< udATI_uint8   
      ScanDirection, //!< udATI_uint8   
      EdgeOfFlightLine, //!< udATI_uint8   
      ScanAngleRank, //!< udATI_uint8   
      LasUserData, //!< Specific LAS User data field (udATI_uint8)

      Count, //!< Count helper value to iterate this enum
      AllAttributes = Count, //!< Internal sentinal value used by some functions to indicate getting start of interleaved attributes
      First = 0, //!< Generally used to initialise an attribute value for use in loops
    };

    public enum StandardAttributeContent
    {
      udSAC_None = (0),
      udSAC_GPSTime = (1 << StandardAttribute.GPSTime),
      udSAC_PrimitiveID = (1 << StandardAttribute.PrimitiveID),
      udSAC_ARGB = (1 << StandardAttribute.ARGB),
      udSAC_Normal = (1 << StandardAttribute.Normal),
      udSAC_Red = (1 << StandardAttribute.Red),
      udSAC_Green = (1 << StandardAttribute.Green),
      udSAC_Blue = (1 << StandardAttribute.Blue),
      udSAC_Intensity = (1 << StandardAttribute.Intensity),
      udSAC_NIR = (1 << StandardAttribute.NIR),
      udSAC_ScanAngle = (1 << StandardAttribute.ScanAngle),
      udSAC_PointSourceID = (1 << StandardAttribute.PointSourceID),
      udSAC_Classification = (1 << StandardAttribute.Classification),
      udSAC_ReturnNumber = (1 << StandardAttribute.ReturnNumber),
      udSAC_NumberOfReturns = (1 << StandardAttribute.NumberOfReturns),
      udSAC_ClassificationFlags = (1 << StandardAttribute.ClassificationFlags),
      udSAC_ScannerChannel = (1 << StandardAttribute.ScannerChannel),
      udSAC_ScanDirection = (1 << StandardAttribute.ScanDirection),
      udSAC_EdgeOfFlightLine = (1 << StandardAttribute.EdgeOfFlightLine),
      udSAC_ScanAngleRank = (1 << StandardAttribute.ScanAngleRank),
      udSAC_LasUserData = (1 << StandardAttribute.LasUserData),

      udSAC_AllAttributes = (1 << StandardAttribute.AllAttributes) - 1,
      udSAC_64BitAttributes = udSAC_GPSTime,
      udSAC_32BitAttributes = udSAC_PrimitiveID + udSAC_ARGB + udSAC_Normal,
      udSAC_16BitAttributes = udSAC_Intensity + udSAC_NIR + udSAC_ScanAngle + udSAC_PointSourceID,
    };

    //!
    //! These are the various options for how an attribute is calculated when merging voxels while generating LODs
    //!
    public enum udAttributeBlendType
    {
      udABT_Mean, //!< This blend type merges nearby voxels together and finds the mean value for the attribute on those nodes
      udABT_FirstChild, //!< This blend type selects the value from one of the nodes and uses that
      udABT_NoLOD, //!< This blend type has no information in LODs and is only stored in the highest detail level

      udABT_Count //!< Total number of blend types. Used internally but can be used as an iterator max when checking attribute blend modes.
    };

    //!
    //! These are the types that could be contained in attributes
    //!
    public enum udAttributeTypeInfo
    {
      udATI_Invalid = 0,
      udATI_SizeMask = 0x000ff,  // Lower 8 bits define the size in bytes - currently the actual maximum is 32
      udATI_SizeShift = 0,
      udATI_ComponentCountMask = 0x0ff00,  // Next 8 bits define the number of components, component size is size/componentCount
      udATI_ComponentCountShift = 8,
      udATI_Signed = 0x10000,  // Set if type is signed (used in blend functions)
      udATI_Float = 0x20000,  // Set if floating point type (signed should always be set)
      udATI_Color = 0x40000,  // Set if type is de-quantized from a color
      udATI_Normal = 0x80000,  // Set if type is encoded normal (32 bit = 16:15:1)

      // Some keys to define standard types
      udATI_uint8 = 1,
      udATI_uint16 = 2,
      udATI_uint32 = 4,
      udATI_uint64 = 8,
      udATI_int8 = 1 | udATI_Signed,
      udATI_int16 = 2 | udATI_Signed,
      udATI_int32 = 4 | udATI_Signed,
      udATI_int64 = 8 | udATI_Signed,
      udATI_float32 = 4 | udATI_Signed | udATI_Float,
      udATI_float64 = 8 | udATI_Signed | udATI_Float,
      udATI_color32 = 4 | udATI_Color,
      udATI_normal32 = 4 | udATI_Normal,
      udATI_vec3f32 = 12 | 0x300 | udATI_Signed | udATI_Float
    };

    //!
    //! @struct udAttributeDescriptor
    //! Describes the contents of an attribute stream including its size, type and how it gets blended in LOD layers
    //!
    [StructLayout(LayoutKind.Sequential)]
    public struct udAttributeDescriptor
    {
      public udAttributeTypeInfo typeInfo; //!< This contains information about the type
      public udAttributeBlendType blendType; //!< Which blend type this attribute is using
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
      char[] _name; //!< Name of the attibute
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
      char[] _prefix; //!< Value prefix for display
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
      char[] _suffix; //!< Value suffix for display

      public Type GetValueType()
      {
        switch (typeInfo)
        {
          case udAttributeTypeInfo.udATI_uint8:
            return typeof(byte);
          case udAttributeTypeInfo.udATI_uint16:
            return typeof(UInt16);
          case udAttributeTypeInfo.udATI_uint32:
            return typeof(UInt32);
          case udAttributeTypeInfo.udATI_uint64:
            return typeof(UInt64);
          case udAttributeTypeInfo.udATI_int8:
            return typeof(sbyte);
          case udAttributeTypeInfo.udATI_int16:
            return typeof(Int16);
          case udAttributeTypeInfo.udATI_int32:
            return typeof(Int32);
          case udAttributeTypeInfo.udATI_int64:
            return typeof(Int64);
          case udAttributeTypeInfo.udATI_float32:
            return typeof(float);
          case udAttributeTypeInfo.udATI_float64:
            return typeof(double);
          case udAttributeTypeInfo.udATI_color32:
            return typeof(UInt32);
          case udAttributeTypeInfo.udATI_normal32:
            return typeof(UInt32);
          case udAttributeTypeInfo.udATI_vec3f32: //fallthrough
          default:
            throw new NotImplementedException();
        }
      }

      private static void StringIntoCharBufferUTF8(ref char[] buffer,  string value)
      {
        System.Text.Encoding encoding = new System.Text.UTF8Encoding();
        char[] st = encoding.GetChars(encoding.GetBytes(value));
        Array.Copy(st, buffer, st.Length);
      }

      public string name
      {
        get
        {
          _name ??= new char[64];
          return new string(_name);
        }
        set
        {
          _name ??= new char[64];
          StringIntoCharBufferUTF8(ref _name, value);
        }
      }

      public string prefix
      {
        get
        {
          _prefix ??= new char[16];
          return new string(_prefix);
        }
        set
        {
          _prefix ??= new char[16];
          StringIntoCharBufferUTF8(ref _prefix, value);
        }
      }

      public string suffix
      {
        get
        {
          _suffix ??= new char[16];
          return new string(_suffix);
        }
        set
        {
          _suffix ??= new char[16];
          StringIntoCharBufferUTF8(ref _suffix, value);
        }
      }

      static public bool operator==(udAttributeDescriptor lhs, udAttributeDescriptor rhs)
      {
        return lhs.blendType == rhs.blendType && lhs.typeInfo == lhs.typeInfo && (new string(lhs._name) == new string(lhs._name)) && (new string(lhs._prefix) == new string(lhs._prefix)) && (new string(lhs._suffix) == new string(lhs._suffix));
      }
      static public bool operator!=(udAttributeDescriptor lhs, udAttributeDescriptor rhs) { return !(lhs == rhs); }
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct udAttributeSetInternal
    {
      public StandardAttributeContent standardContent; //!< Which standard attributes are available (used to optimize lookups internally), they still appear in the descriptors
      public uint count; //!< How many udAttributeDescriptor objects are used in pDescriptors
      public uint allocated; //!< How many udAttributeDescriptor objects are allocated to be used in pDescriptors
      public IntPtr pDescriptors; //!< this contains the actual information on the attributes

      private int IndexOf(string name)
      {
        for(int i=0; i< count; ++i)
        {
          if (this[i].name.TrimEnd('\0') == name)
            return i;
        }
        return -1;
      }

      public UInt32 ByteOffsetOf(StandardAttribute i)
      {
        IntPtr intPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(UInt32)));
        IntPtr selfPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(udAttributeSetInternal)));
        Marshal.StructureToPtr(this, selfPtr, false);
        udError code = udAttributeSet_GetOffsetOfStandardAttribute(selfPtr, i, intPtr);
        if(code != udError.udE_Success)
          throw new UDException(code);
        UInt32 offset = Marshal.PtrToStructure<UInt32>(intPtr);
        Marshal.FreeHGlobal(intPtr);
        Marshal.FreeHGlobal(selfPtr);
        return offset;
      }

      public UInt32 ByteOffsetOf(string i)
      {
        IntPtr intPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(UInt32)));
        IntPtr selfPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(udAttributeSetInternal)));
        Marshal.StructureToPtr(this, selfPtr, false);
        udError code = udAttributeSet_GetOffsetOfNamedAttribute(selfPtr, i, intPtr);
        if(code != udError.udE_Success)
          throw new UDException(code);
        UInt32 offset = Marshal.PtrToStructure<UInt32>(intPtr);
        Marshal.FreeHGlobal(intPtr);
        Marshal.FreeHGlobal(selfPtr);
        return offset;
      }

      public udAttributeDescriptor this[int i]
      {
        get
        {
          if (Math.Abs(i) > count)
            throw new IndexOutOfRangeException();

          if (i < 0)
            i = (int) count - i;

          IntPtr ret = new IntPtr(pDescriptors.ToInt64() + i * Marshal.SizeOf(typeof(udAttributeDescriptor)));
          return Marshal.PtrToStructure<udAttributeDescriptor>(ret);
        }
        set
        {
          if (Math.Abs(i) > count)
            throw new IndexOutOfRangeException();

          if (i < 0)
            i = (int) count - i;

          IntPtr ret = new IntPtr(pDescriptors.ToInt64() + i * Marshal.SizeOf(typeof(udAttributeDescriptor)));
          Marshal.StructureToPtr(value, ret, false);
        }
      }

      public udAttributeDescriptor this[string i]
      {
        get
        {
          IntPtr intPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(udAttributeDescriptor)));
          IntPtr selfPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(udAttributeSetInternal)));
          Marshal.StructureToPtr(this, selfPtr, false);
          udError code = udAttributeSet_GetDescriptorOfNamedAttribute(selfPtr, i, intPtr);
          if(code != udError.udE_Success)
            throw new UDException(code);
          Marshal.FreeHGlobal(selfPtr);
          udAttributeDescriptor ret =  Marshal.PtrToStructure<udAttributeDescriptor>(intPtr);
          Marshal.FreeHGlobal(intPtr);
          return ret;
        }
        set
        {
          int descriptorIndex = IndexOf(i);
          if(descriptorIndex == -1)
          {
            if(count < allocated)
            {
              // add to set
              descriptorIndex = (int) count;
              ++count;
            }
            else
            {
              throw new OverflowException("Set is full: cannot add new member");
            }
          }
          value.name = i;
          this[descriptorIndex] = value;
        }
      }

      [DllImport("udSDK")]
      private static extern udError udAttributeSet_GetOffsetOfStandardAttribute(IntPtr /*udAttributeSet * */pAttributeSet, StandardAttribute attribute, IntPtr pOffset);

      [DllImport("udSDK")]
      private static extern udError udAttributeSet_GetOffsetOfNamedAttribute(IntPtr /*udAttributeSet * */pAttributeSet, [MarshalAs(UnmanagedType.LPUTF8Str)] string pName,  IntPtr pOffset);

      [DllImport("udSDK")]
      private static extern udError udAttributeSet_GetDescriptorOfNamedAttribute(IntPtr /*udAttributeSet * */pAttributeSet, [MarshalAs(UnmanagedType.LPUTF8Str)] string pName, IntPtr /*udAttributeDescriptor * */pDescriptor);

      [DllImport("udSDK")]
      private static extern udError udAttribute_GetDescriptorOfStandardAttribute(StandardAttribute attribute, IntPtr /*udAttributeDescriptor * */pDescriptor);
    }
    public class AttributeSet
    {
      public IntPtr pAttributeSet;
      bool manuallyCreated;
      public AttributeSet(StandardAttributeContent standardContent, UInt32 additionalContent)
      {
        udError code = udAttributeSet_Create(pAttributeSet, standardContent, additionalContent);
        if(code != udError.udE_Success)
          throw new UDException(code);
        manuallyCreated = true;
      }

      ~AttributeSet()
      {
        if (manuallyCreated)
        {
          udError code = udAttributeSet_Destroy(pAttributeSet);
          if(code != udError.udE_Success)
            throw new UDException(code);
        }
      }

      //this is a read only copy of the internal structure:
      udAttributeSetInternal InternalAttributeSet
      {
        get { return Marshal.PtrToStructure<udAttributeSetInternal>(pAttributeSet); }
      }

      public static udAttributeSetInternal From_Pointer(IntPtr ptr)
      {
        return Marshal.PtrToStructure<udAttributeSetInternal>(ptr);
      }

      [DllImport("udSDK")]
      private static extern udError udAttributeSet_Create(IntPtr /*udAttributeSet * */pAttributeSet, StandardAttributeContent content, UInt32 additionalCustomAttributes);

      [DllImport("udSDK")]
      private static extern udError udAttributeSet_Destroy(IntPtr /*udAttributeSet * */pAttributeSet);
         
    };
  }
}

