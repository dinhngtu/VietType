namespace VietTypeConfig {
    partial class Form1 {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.cbDefaultEnabled = new System.Windows.Forms.CheckBox();
            this.settingsBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.cbOaUy = new System.Windows.Forms.CheckBox();
            this.cbAcceptDd = new System.Windows.Forms.CheckBox();
            this.cbBackspaceInvalid = new System.Windows.Forms.CheckBox();
            this.cbBackconvertOnBackspace = new System.Windows.Forms.CheckBox();
            this.btnEnable = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnAbout = new System.Windows.Forms.Button();
            this.lblOptimizeMultilang = new System.Windows.Forms.Label();
            this.udOptimizeMultilang = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.settingsBindingSource)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.udOptimizeMultilang)).BeginInit();
            this.SuspendLayout();
            // 
            // cbDefaultEnabled
            // 
            resources.ApplyResources(this.cbDefaultEnabled, "cbDefaultEnabled");
            this.cbDefaultEnabled.AutoEllipsis = true;
            this.cbDefaultEnabled.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "DefaultEnabled", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.cbDefaultEnabled.Name = "cbDefaultEnabled";
            this.cbDefaultEnabled.UseVisualStyleBackColor = true;
            // 
            // settingsBindingSource
            // 
            this.settingsBindingSource.DataSource = typeof(VietTypeConfig.Settings);
            // 
            // cbOaUy
            // 
            resources.ApplyResources(this.cbOaUy, "cbOaUy");
            this.cbOaUy.AutoEllipsis = true;
            this.cbOaUy.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "OaUy", true));
            this.cbOaUy.Name = "cbOaUy";
            this.cbOaUy.UseVisualStyleBackColor = true;
            // 
            // cbAcceptDd
            // 
            resources.ApplyResources(this.cbAcceptDd, "cbAcceptDd");
            this.cbAcceptDd.AutoEllipsis = true;
            this.cbAcceptDd.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "AcceptDd", true));
            this.cbAcceptDd.Name = "cbAcceptDd";
            this.cbAcceptDd.UseVisualStyleBackColor = true;
            // 
            // cbBackspaceInvalid
            // 
            resources.ApplyResources(this.cbBackspaceInvalid, "cbBackspaceInvalid");
            this.cbBackspaceInvalid.AutoEllipsis = true;
            this.cbBackspaceInvalid.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "BackspaceInvalid", true));
            this.cbBackspaceInvalid.Name = "cbBackspaceInvalid";
            this.cbBackspaceInvalid.UseVisualStyleBackColor = true;
            // 
            // cbBackconvertOnBackspace
            // 
            resources.ApplyResources(this.cbBackconvertOnBackspace, "cbBackconvertOnBackspace");
            this.cbBackconvertOnBackspace.AutoEllipsis = true;
            this.cbBackconvertOnBackspace.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "BackconvertOnBackspace", true));
            this.cbBackconvertOnBackspace.Name = "cbBackconvertOnBackspace";
            this.cbBackconvertOnBackspace.UseVisualStyleBackColor = true;
            // 
            // btnEnable
            // 
            resources.ApplyResources(this.btnEnable, "btnEnable");
            this.btnEnable.Name = "btnEnable";
            this.btnEnable.UseVisualStyleBackColor = true;
            this.btnEnable.Click += new System.EventHandler(this.btnEnable_Click);
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label1.Name = "label1";
            // 
            // btnOK
            // 
            resources.ApplyResources(this.btnOK, "btnOK");
            this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOK.Name = "btnOK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.CloseForm);
            // 
            // btnCancel
            // 
            resources.ApplyResources(this.btnCancel, "btnCancel");
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.CloseForm);
            // 
            // btnAbout
            // 
            resources.ApplyResources(this.btnAbout, "btnAbout");
            this.btnAbout.Name = "btnAbout";
            this.btnAbout.UseVisualStyleBackColor = true;
            this.btnAbout.Click += new System.EventHandler(this.btnAbout_Click);
            // 
            // lblOptimizeMultilang
            // 
            resources.ApplyResources(this.lblOptimizeMultilang, "lblOptimizeMultilang");
            this.lblOptimizeMultilang.Name = "lblOptimizeMultilang";
            // 
            // udOptimizeMultilang
            // 
            resources.ApplyResources(this.udOptimizeMultilang, "udOptimizeMultilang");
            this.udOptimizeMultilang.DataBindings.Add(new System.Windows.Forms.Binding("Value", this.settingsBindingSource, "OptimizeMultilang", true));
            this.udOptimizeMultilang.Maximum = new decimal(new int[] {
            3,
            0,
            0,
            0});
            this.udOptimizeMultilang.Name = "udOptimizeMultilang";
            // 
            // Form1
            // 
            this.AcceptButton = this.btnOK;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.Controls.Add(this.udOptimizeMultilang);
            this.Controls.Add(this.lblOptimizeMultilang);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnEnable);
            this.Controls.Add(this.btnAbout);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.cbBackconvertOnBackspace);
            this.Controls.Add(this.cbBackspaceInvalid);
            this.Controls.Add(this.cbAcceptDd);
            this.Controls.Add(this.cbOaUy);
            this.Controls.Add(this.cbDefaultEnabled);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Form1";
            this.ShowIcon = false;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.Shown += new System.EventHandler(this.Form1_Shown);
            ((System.ComponentModel.ISupportInitialize)(this.settingsBindingSource)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.udOptimizeMultilang)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox cbDefaultEnabled;
        private System.Windows.Forms.CheckBox cbOaUy;
        private System.Windows.Forms.CheckBox cbAcceptDd;
        private System.Windows.Forms.CheckBox cbBackspaceInvalid;
        private System.Windows.Forms.CheckBox cbBackconvertOnBackspace;
        private System.Windows.Forms.BindingSource settingsBindingSource;
        private System.Windows.Forms.Button btnEnable;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnAbout;
        private System.Windows.Forms.Label lblOptimizeMultilang;
        private System.Windows.Forms.NumericUpDown udOptimizeMultilang;
    }
}

