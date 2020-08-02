using System;
using System.Windows.Forms;

using udSDK;

namespace VDKWinForms
{
  public partial class LoginBox : Form
  {
    private udContext vaultCtx = null;
    private bool loginSuccess = false;

    public LoginBox(ref udContext vaultCtx)
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
