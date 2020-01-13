using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

using Vault;

namespace VDKWinForms
{
  public partial class ViewForm : Form
  {
    vdkContext vaultCtx;
    vdkRenderContext renderCtx;
    vdkRenderView renderView;
    uint vdkWidth;
    uint vdkHeight;
    uint[] vdkColorBuffer;
    float[] vdkDepthBuffer;

    float time;
    public Bitmap bmp;
    bool closeWhenPossible = false;
    
    public ViewForm()
    {
      InitializeComponent();

      vaultCtx = new vdkContext();

      LoginBox box = new LoginBox(ref vaultCtx);
      box.ShowDialog();

      if (box.GetLoginInfo())
      {
        renderCtx = new vdkRenderContext();
        renderView = new vdkRenderView();

        vdkWidth = (uint)panel1.Width;
        vdkHeight = (uint)panel1.Height;

        vdkColorBuffer = new uint[vdkWidth * vdkHeight];
        vdkDepthBuffer = new float[vdkWidth * vdkHeight];

        renderCtx.Create(vaultCtx);
        renderView.Create(vaultCtx, renderCtx, vdkWidth, vdkHeight);
        renderView.SetTargets(ref vdkColorBuffer, 0, ref vdkDepthBuffer);

        bmp = new Bitmap((int)vdkWidth, (int)vdkHeight, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
      }
      else
      {
        closeWhenPossible = true;
      }
    }

    private void panel1_Paint(object sender, PaintEventArgs e)
    {
      if (closeWhenPossible)
      {
        Close();
        return;
      }

      vdkRenderInstance[] renderInstances = new vdkRenderInstance[listBox1.Items.Count];

      time += 0.01f;
      double angle = -Math.PI / 3.0;

      double[] matrix = new double[16];
      matrix[0] = Math.Cos(time) * Math.Sin(angle);
      matrix[1] =-Math.Sin(time) * Math.Sin(angle);
      matrix[2] = Math.Cos(angle);

      matrix[4] = Math.Sin(time) * Math.Sin(angle);
      matrix[5] = Math.Cos(time) * Math.Sin(angle);
      matrix[6] = Math.Cos(angle);

      matrix[8] = 0;
      matrix[9] = 0;
      matrix[10] = Math.Sin(angle);

      matrix[12] = 0;
      matrix[13] = 1;
      matrix[14] = -0.25;

      matrix[15] = 1.0;

      for (int i = 0; i < listBox1.Items.Count; ++i)
      {
        renderInstances[i].pointCloud = (listBox1.Items[i] as PointCloud).pointCloud.pModel;
        renderInstances[i].worldMatrix = matrix;//(listBox1.Items[i] as PointCloud).matrix;
      }
      
      double[] cameraMatrix = {
          1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0,0,0,1
        };

      renderView.SetMatrix(Vault.RenderViewMatrix.Camera, cameraMatrix);
      
      renderCtx.Render(renderView, renderInstances, renderInstances.Length);

      // Copy it to the bitmap
      BitmapData bData = bmp.LockBits(new Rectangle(0, 0, (int)vdkWidth, (int)vdkHeight), ImageLockMode.WriteOnly, bmp.PixelFormat);
      System.Runtime.InteropServices.Marshal.Copy((int[])(object)vdkColorBuffer, 0, bData.Scan0, (int)(vdkWidth*vdkHeight));
      bmp.UnlockBits(bData);
      
      // Draw the bitmap
      e.Graphics.DrawImage(bmp, 0, 0);

      panel1.Invalidate();
    }

    private void listBox1_DragDrop(object sender, DragEventArgs e)
    {
      string[] s = (string[])e.Data.GetData(DataFormats.FileDrop, false);
      int i;
      for (i = 0; i < s.Length; i++)
      {
        PointCloud pc = new PointCloud(s[i], vaultCtx);

        listBox1.Items.Add(pc);
      }
    }

    private void listBox1_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
    {
      if (e.Data.GetDataPresent(DataFormats.FileDrop))
        e.Effect = DragDropEffects.All;
      else
        e.Effect = DragDropEffects.None;
    }

    private void timer1_Tick(object sender, EventArgs e)
    {
      panel1.Invalidate();
    }
  }

  class PointCloud
  {
    public vdkPointCloud pointCloud;
    public double[] matrix;
    string filename;

    public PointCloud(string filename, vdkContext vaultCtx)
    {
      this.filename = filename;

      vdkPointCloudHeader header = new vdkPointCloudHeader();

      pointCloud = new vdkPointCloud();
      pointCloud.Load(vaultCtx, filename, ref header);

      matrix = header.storedMatrix;
    }

    public override string ToString()
    {
      return filename;
    }
  }
}
