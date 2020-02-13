using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class flipBuggy : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
    if (Input.GetKeyUp(KeyCode.F))
    {
      GameObject buggy = GameObject.FindWithTag("demoRoot");
      float oldYrot = buggy.transform.localEulerAngles.y;
      buggy.transform.localEulerAngles = new Vector3(0,oldYrot,0);
    }

    }
}
