using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using Vault;

public class loadAllUDSInDirectory : MonoBehaviour
{
  public string path;
  // Start is called before the first frame update
  void Start()
  {
    string[] files = Directory.GetFiles(path);
    Vector3 rootBaseOffset = new Vector3();
    for (int i= 0; i < files.Length; ++i)
    {
      string file = files[i];
      //skip non uds files
      if (!file.Substring(file.Length - 4).Equals(".uds"))
        continue;

      GameObject udModel = new GameObject(file);
      udModel.transform.SetParent(this.transform);
      udModel.AddComponent<UDSModel>();
      UDSModel model = udModel.GetComponent<UDSModel>();
      model.path = file;
      try
      {
        model.LoadModel();
      }
      catch
      {
        Debug.LogError("load model failed: "+file);
        continue;
      }
      Vector3 baseOffset = new Vector3((float) model.header.baseOffset[0], (float) model.header.baseOffset[1], (float) model.header.baseOffset[2]);

      if (i == 0)
        rootBaseOffset = baseOffset;
      model.transform.localPosition = baseOffset - rootBaseOffset;
      model.transform.localScale = new Vector3((float)model.header.scaledRange, (float)model.header.scaledRange, (float)model.header.scaledRange);
      model.transform.localRotation = Quaternion.Euler(-90, 0, 0);
      udModel.tag = "UDSModel";
    }

  }

  // Update is called once per frame
  void Update()
  {
  }
}
