/*
 * Copyright (C) 2011 printf.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace SequenceLogId
{
	partial class FormMain
	{
		/// <summary>
		/// 必要なデザイナ変数です。
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		/// <param name="disposing">マネージ リソースが破棄される場合 true、破棄されない場合は false です。</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows フォーム デザイナで生成されたコード

		/// <summary>
		/// デザイナ サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディタで変更しないでください。
		/// </summary>
		private void InitializeComponent()
		{
			this.radioCpp = new System.Windows.Forms.RadioButton();
			this.radioCs = new System.Windows.Forms.RadioButton();
			this.radioJava = new System.Windows.Forms.RadioButton();
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// radioCpp
			// 
			this.radioCpp.AutoSize = true;
			this.radioCpp.Location = new System.Drawing.Point(13, 41);
			this.radioCpp.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
			this.radioCpp.Name = "radioCpp";
			this.radioCpp.Size = new System.Drawing.Size(89, 27);
			this.radioCpp.TabIndex = 0;
			this.radioCpp.TabStop = true;
			this.radioCpp.Text = "C / C++";
			this.radioCpp.UseVisualStyleBackColor = true;
			this.radioCpp.CheckedChanged += new System.EventHandler(this.radioCpp_CheckedChanged);
			// 
			// radioCs
			// 
			this.radioCs.AutoSize = true;
			this.radioCs.Location = new System.Drawing.Point(13, 78);
			this.radioCs.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
			this.radioCs.Name = "radioCs";
			this.radioCs.Size = new System.Drawing.Size(50, 27);
			this.radioCs.TabIndex = 1;
			this.radioCs.TabStop = true;
			this.radioCs.Text = "C#";
			this.radioCs.UseVisualStyleBackColor = true;
			this.radioCs.CheckedChanged += new System.EventHandler(this.radioCs_CheckedChanged);
			// 
			// radioJava
			// 
			this.radioJava.AutoSize = true;
			this.radioJava.Location = new System.Drawing.Point(13, 113);
			this.radioJava.Name = "radioJava";
			this.radioJava.Size = new System.Drawing.Size(61, 27);
			this.radioJava.TabIndex = 2;
			this.radioJava.TabStop = true;
			this.radioJava.Text = "Java";
			this.radioJava.UseVisualStyleBackColor = true;
			this.radioJava.CheckedChanged += new System.EventHandler(this.radioJava_CheckedChanged);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(155, 23);
			this.label1.TabIndex = 3;
			this.label1.Text = "Please drag csv file.";
			// 
			// FormMain
			// 
			this.AllowDrop = true;
			this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 23F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.White;
			this.ClientSize = new System.Drawing.Size(237, 148);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.radioJava);
			this.Controls.Add(this.radioCs);
			this.Controls.Add(this.radioCpp);
			this.Font = new System.Drawing.Font("メイリオ", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
			this.MaximizeBox = false;
			this.Name = "FormMain";
			this.Text = "Sequnce Log Id";
			this.DragDrop += new System.Windows.Forms.DragEventHandler(this.FormMain_DragDrop);
			this.DragEnter += new System.Windows.Forms.DragEventHandler(this.FormMain_DragEnter);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.RadioButton radioCpp;
		private System.Windows.Forms.RadioButton radioCs;
		private System.Windows.Forms.RadioButton radioJava;
		private System.Windows.Forms.Label label1;
	}
}

