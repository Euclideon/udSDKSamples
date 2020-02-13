using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

namespace Vault
{
    public static class UDUtilities
    {

        public static double[] GetUDMatrix(Matrix4x4 unityMat)
        {
            double[] udMat =
            {
                unityMat.m00,
                unityMat.m10,
                unityMat.m20,
                unityMat.m30,

                unityMat.m01,
                unityMat.m11,
                unityMat.m21,
                unityMat.m31,

                unityMat.m02,
                unityMat.m12,
                unityMat.m22,
                unityMat.m32,

                unityMat.m03,
                unityMat.m13,
                unityMat.m23,
                unityMat.m33
            };

         return udMat;
        }

        /*
         * attempts to load and returns all loaded UDS models in the scene
         */
        public static vdkRenderInstance[] getUDSInstances()
        {
            GameObject[] objects = GameObject.FindGameObjectsWithTag("UDSModel");
            int count = 0;
            vdkRenderInstance[] modelArray = new vdkRenderInstance[objects.Length];
            for (int i = 0; i < objects.Length; ++i)
            {
                Component component = objects[i].GetComponent("UDSModel");
                UDSModel model = component as UDSModel;

                if (!model.isLoaded)
                    model.LoadModel();

                if (model.isLoaded)
                {
                    modelArray[count].pointCloud = model.udModel.pModel;
                    modelArray[count].worldMatrix = UDUtilities.GetUDMatrix(model.pivotTranslation * model.modelScale * objects[i].transform.localToWorldMatrix * model.pivotTranslation.inverse);
                    count++;
                }
            }
            return modelArray.Where(m => (m.pointCloud != System.IntPtr.Zero)).ToArray();
        }
    }
}

