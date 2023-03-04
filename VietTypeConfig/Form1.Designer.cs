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
            this.cbDefaultEnabled = new System.Windows.Forms.CheckBox();
            this.settingsBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.cbOaUy = new System.Windows.Forms.CheckBox();
            this.cbAcceptDd = new System.Windows.Forms.CheckBox();
            this.cbBackspaceInvalid = new System.Windows.Forms.CheckBox();
            this.cbBackconvertOnBackspace = new System.Windows.Forms.CheckBox();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnAbout = new System.Windows.Forms.Button();
            this.btnEnable = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.settingsBindingSource)).BeginInit();
            this.SuspendLayout();
            // 
            // cbDefaultEnabled
            // 
            this.cbDefaultEnabled.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cbDefaultEnabled.AutoEllipsis = true;
            this.cbDefaultEnabled.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "DefaultEnabled", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.cbDefaultEnabled.Location = new System.Drawing.Point(12, 51);
            this.cbDefaultEnabled.Name = "cbDefaultEnabled";
            this.cbDefaultEnabled.Size = new System.Drawing.Size(320, 17);
            this.cbDefaultEnabled.TabIndex = 1;
            this.cbDefaultEnabled.Text = "Vietnamese mode by de&fault";
            this.cbDefaultEnabled.UseVisualStyleBackColor = true;
            // 
            // settingsBindingSource
            // 
            this.settingsBindingSource.DataSource = typeof(VietTypeConfig.Settings);
            // 
            // cbOaUy
            // 
            this.cbOaUy.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cbOaUy.AutoEllipsis = true;
            this.cbOaUy.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "OaUy", true));
            this.cbOaUy.Location = new System.Drawing.Point(12, 74);
            this.cbOaUy.Name = "cbOaUy";
            this.cbOaUy.Size = new System.Drawing.Size(320, 17);
            this.cbOaUy.TabIndex = 2;
            this.cbOaUy.Text = "&Use \"oà\", \"uý\" instead of \"òa\", \"úy\"";
            this.cbOaUy.UseVisualStyleBackColor = true;
            // 
            // cbAcceptDd
            // 
            this.cbAcceptDd.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cbAcceptDd.AutoEllipsis = true;
            this.cbAcceptDd.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "AcceptDd", true));
            this.cbAcceptDd.Location = new System.Drawing.Point(12, 97);
            this.cbAcceptDd.Name = "cbAcceptDd";
            this.cbAcceptDd.Size = new System.Drawing.Size(320, 17);
            this.cbAcceptDd.TabIndex = 3;
            this.cbAcceptDd.Text = "Accept \'d\' any&where";
            this.cbAcceptDd.UseVisualStyleBackColor = true;
            // 
            // cbBackspaceInvalid
            // 
            this.cbBackspaceInvalid.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cbBackspaceInvalid.AutoEllipsis = true;
            this.cbBackspaceInvalid.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "BackspaceInvalid", true));
            this.cbBackspaceInvalid.Location = new System.Drawing.Point(12, 120);
            this.cbBackspaceInvalid.Name = "cbBackspaceInvalid";
            this.cbBackspaceInvalid.Size = new System.Drawing.Size(320, 17);
            this.cbBackspaceInvalid.TabIndex = 4;
            this.cbBackspaceInvalid.Text = "Backspaced &invalid word stays invalid";
            this.cbBackspaceInvalid.UseVisualStyleBackColor = true;
            // 
            // cbBackconvertOnBackspace
            // 
            this.cbBackconvertOnBackspace.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cbBackconvertOnBackspace.AutoEllipsis = true;
            this.cbBackconvertOnBackspace.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this.settingsBindingSource, "BackconvertOnBackspace", true));
            this.cbBackconvertOnBackspace.Location = new System.Drawing.Point(12, 143);
            this.cbBackconvertOnBackspace.Name = "cbBackconvertOnBackspace";
            this.cbBackconvertOnBackspace.Size = new System.Drawing.Size(320, 17);
            this.cbBackconvertOnBackspace.TabIndex = 5;
            this.cbBackconvertOnBackspace.Text = "&Reconvert word on Backspace (Experimental)";
            this.cbBackconvertOnBackspace.UseVisualStyleBackColor = true;
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOK.Location = new System.Drawing.Point(176, 226);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 6;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.CloseForm);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(257, 226);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 7;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.CloseForm);
            // 
            // btnAbout
            // 
            this.btnAbout.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnAbout.Location = new System.Drawing.Point(12, 226);
            this.btnAbout.Name = "btnAbout";
            this.btnAbout.Size = new System.Drawing.Size(75, 23);
            this.btnAbout.TabIndex = 8;
            this.btnAbout.Text = "A&bout...";
            this.btnAbout.UseVisualStyleBackColor = true;
            this.btnAbout.Click += new System.EventHandler(this.btnAbout_Click);
            // 
            // btnEnable
            // 
            this.btnEnable.Location = new System.Drawing.Point(12, 12);
            this.btnEnable.Name = "btnEnable";
            this.btnEnable.Size = new System.Drawing.Size(105, 23);
            this.btnEnable.TabIndex = 0;
            this.btnEnable.Text = "&Enable VietType";
            this.btnEnable.UseVisualStyleBackColor = true;
            this.btnEnable.Click += new System.EventHandler(this.btnEnable_Click);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label1.Location = new System.Drawing.Point(0, 42);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(344, 2);
            this.label1.TabIndex = 9;
            this.label1.Text = "label1";
            // 
            // Form1
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(344, 261);
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
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "VietType Settings";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.Shown += new System.EventHandler(this.Form1_Shown);
            ((System.ComponentModel.ISupportInitialize)(this.settingsBindingSource)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.CheckBox cbDefaultEnabled;
        private System.Windows.Forms.CheckBox cbOaUy;
        private System.Windows.Forms.CheckBox cbAcceptDd;
        private System.Windows.Forms.CheckBox cbBackspaceInvalid;
        private System.Windows.Forms.CheckBox cbBackconvertOnBackspace;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.BindingSource settingsBindingSource;
        private System.Windows.Forms.Button btnAbout;
        private System.Windows.Forms.Button btnEnable;
        private System.Windows.Forms.Label label1;
    }
}

