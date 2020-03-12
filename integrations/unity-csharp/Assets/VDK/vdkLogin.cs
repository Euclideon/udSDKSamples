using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using Vault;
namespace Vault
{

    public static class GlobalVDKContext
    {
        public static bool isCreated = false;
        public static vdkContext vContext = new vdkContext();
        public static vdkRenderContext renderer = new vdkRenderContext();
        public static string vaultServer = "https://earth.vault.euclideon.com";
        public static string vaultUsername = ""; // leave empty
        
        public static string vaultPassword = ""; // leave empty

        // These strings exist to ensure during development no typo or error is ever set regarding the saving/loading/reading of 
        // .. usernames and passwords.
        public static string SavedUsernameKey = "VDKUsername";
        public static string SavedPasswordKey = "VDKPassword";
        public static void Login()
        {
            // Using editor prefs unfortunatly
            vaultUsername = EditorPrefs.GetString(SavedUsernameKey);
            vaultPassword = EditorPrefs.GetString(SavedPasswordKey);

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

