using UnityEngine;
using System;
using UnityEditor;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;
using Vault;




namespace vdk
{



    public class VDKUserInfoEditor : EditorWindow
    {
        [MenuItem("VDK/Set user Info")]

        public static void ShowWindow()
        {
            EditorWindow.GetWindow(typeof(VDKUserInput));
        }
    }

    public class VDKUserInput : EditorWindow
    {
        // Show window logic
        private static void ShowWindow()
        {
            var window = GetWindow<VDKUserInput>();
            window.titleContent = new GUIContent("VDK User Info");
            window.Show();

        }

        // Load details
        private void Awake()
        {
            LoadUserInfo();
        }

        private void LoadUserInfo()
        {
            usernameEntry = EditorPrefs.GetString(GlobalVDKContext.SavedUsernameKey);
            passwordEntry = EditorPrefs.GetString(GlobalVDKContext.SavedPasswordKey);

            CommitUserInfo();
        }

        private string PrefsUsername()
        {
            return EditorPrefs.GetString(GlobalVDKContext.SavedUsernameKey);
        }

        // Strings used to store the entered info
        private string usernameEntry;
        private string passwordEntry;
        
        private string SuccessMessage ="";

        private void OnGUI()
        {
            // Current input related event 
            Event e = Event.current;

            // Centered layer style for headers
            var CenteredBold = GUI.skin.GetStyle("Label");
            CenteredBold.alignment = TextAnchor.MiddleCenter;
            CenteredBold.fontStyle = FontStyle.Bold;

            // Centered bottom layerstyle for clear button
            var BottomButton = GUI.skin.GetStyle("Button");
            BottomButton.alignment = TextAnchor.LowerCenter;

            // Drawing the GUI elements
            EditorGUILayout.LabelField("VDK Save user Info", CenteredBold);
            EditorGUILayout.LabelField("Once your details are saved, hit play to run your scene"); 

            // Username and password
            EditorGUILayout.LabelField("Username:");
            usernameEntry = EditorGUILayout.TextField(usernameEntry);
            EditorGUILayout.LabelField("Password:");
            passwordEntry = EditorGUILayout.PasswordField(passwordEntry);

            bool pressed = GUILayout.Button("Save User Info");

            EditorGUILayout.LabelField("*WARNING* these details are saved to your computer in plaintext");

            // Radio button for saving to the hard disk or not
            // saveToDisk = GUILayout.Toggle(saveToDisk, "Remember my details", RadioButtonCentered);

            bool clearInfo = GUILayout.Button("Remove saved info", BottomButton);

            // This GUILable is just for visual confirmation the user has done something
            // .. and really should hook directly into Vault API authentication for a true retrun success/error message
            EditorGUILayout.LabelField(SuccessMessage); 


            // Commit the user info in several ways.
            if (pressed || e.keyCode == KeyCode.Return || e.keyCode == KeyCode.KeypadEnter)
            {
                // Save user info
                CommitUserInfo();

                // Idealy the VDK would begin rendering here in the scene view, but I'm unsure if .Login() was built for this purpose
                // GlobalVDKContext.Login();

                SuccessMessage = "User successfully saved!";
            }

            // wipe the users info
            if (clearInfo == true)
            {
                RemoveUserInfo();
            }
        }

        // Called from ONGUI when the user commits info
        private void CommitUserInfo()
        {
            EditorPrefs.SetString(GlobalVDKContext.SavedUsernameKey, usernameEntry);
            EditorPrefs.SetString(GlobalVDKContext.SavedPasswordKey, passwordEntry);
        }

        // Totally wipe user information from the system. Called from ONGUI when button is pressed
        private void RemoveUserInfo()
        {
            usernameEntry = "";
            passwordEntry = "";

            // Clear editor prefs 
            EditorPrefs.SetString(GlobalVDKContext.SavedUsernameKey, "");
            EditorPrefs.SetString(GlobalVDKContext.SavedPasswordKey, "");

            // Player prefs isn't saved, but on the off chance it was during development make sure to clear it
            PlayerPrefs.SetString(GlobalVDKContext.SavedUsernameKey, "");
            PlayerPrefs.SetString(GlobalVDKContext.SavedPasswordKey, "");
            
        }
    }

}