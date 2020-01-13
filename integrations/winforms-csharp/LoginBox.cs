using System;
using System.Windows.Forms;

using Vault;

namespace VDKWinForms
{
  public partial class LoginBox : Form
  {
    private Vault.vdkContext vaultCtx = null;
    private bool loginSuccess = false;

    public LoginBox(ref Vault.vdkContext vaultCtx)
    {
      InitializeComponent();

      this.vaultCtx = vaultCtx;
    }

    private void button1_Click(object sender, EventArgs e)
    {
      try
      {
        vaultCtx.Connect(servernameTB.Text, "WinForms Sample", usernameTB.Text, passwordTB.Text);
        loginSuccess = true;
        this.Close();
      }
      catch(Exception exc)
      {
        errorLBL.Text = exc.Message;
      }
    }

    public bool GetLoginInfo()
    {
      return loginSuccess;
    }
  }
}
