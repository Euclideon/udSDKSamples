using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Vault;
namespace Vault
{

    public static class GlobalVDKContext
    {
        public static bool isCreated = false;
        public static vdkContext vContext = new vdkContext();
        public static vdkRenderContext renderer = new vdkRenderContext();
        public static string vaultServer = "https://earth.vault.euclideon.com";
        public static string vaultUsername = "";
        public static string vaultPassword = "";
        public static void Login()
        {
            if (!GlobalVDKContext.isCreated)
            {
                try
                {
                    Debug.Log("Attempting to resume Euclideon Vault session");
                    vContext.Try_Resume(vaultServer, "Unity", vaultUsername, true);
                    vContext.RequestLicense(LicenseType.Render);
                    isCreated = true;
                    Debug.Log("Resume Succeeded");
                }
                catch (System.Exception e)
                {
                    Debug.Log(e.ToString() + "Logging in to Euclideon Vault server");
                    GlobalVDKContext.vContext.Connect(vaultServer, "Unity", vaultUsername, vaultPassword);
                    vContext.RequestLicense(LicenseType.Render);
                    GlobalVDKContext.isCreated = true;
                    Debug.Log("Logged in!");
                }
            }
            renderer.Create(vContext);
        }
    }
}

