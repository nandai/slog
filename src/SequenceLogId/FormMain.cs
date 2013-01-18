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

using System;
using System.Windows.Forms;
using System.IO;
using Microsoft.VisualBasic.FileIO;

namespace SequenceLogId
{
	public partial class FormMain : Form
	{
		const int TYPE_CPP =  0;
		const int TYPE_CS =   1;
		const int TYPE_JAVA = 2;

		int mOutputType = TYPE_CPP;

		public FormMain()
		{
			InitializeComponent();
		}

		private void radioCpp_CheckedChanged(object sender, EventArgs e)
		{
			mOutputType = TYPE_CPP;
		}

		private void radioCs_CheckedChanged(object sender, EventArgs e)
		{
			mOutputType = TYPE_CS;
		}

		private void radioJava_CheckedChanged(object sender, EventArgs e)
		{
			mOutputType = TYPE_JAVA;
		}

		private void FormMain_DragEnter(object sender, DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop) == false)
				return;

			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);

			if (files.GetLength(0) != 1)
				return;

			if (System.IO.File.Exists(files[0]) == false)
				return;

			e.Effect = DragDropEffects.Copy;
		}

		private void FormMain_DragDrop(object sender, DragEventArgs e)
		{
			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
			string outputFileName = "";

			switch (mOutputType)
			{
				case TYPE_CPP:
					outputFileName = files[0].Replace("csv", "h");
					break;

				case TYPE_CS:
					outputFileName = files[0].Replace("csv", "cs");
					break;

				case TYPE_JAVA:
					outputFileName = files[0].Replace("csv", "java");
					break;
			}

			StreamWriter writer =    new StreamWriter(outputFileName);
			StreamWriter writerSid = new StreamWriter(files[0].Replace("csv", "sid"));

			switch (mOutputType)
			{
				case TYPE_CPP:
					writer.WriteLine("#pragma once");
					writer.WriteLine("");
					break;

				case TYPE_CS:
					writer.WriteLine("namespace Slog");
					writer.WriteLine("{");
					writer.WriteLine("\tclass Id");
					writer.WriteLine("\t{");
					break;

				case TYPE_JAVA:
					writer.WriteLine("public class Id");
					writer.WriteLine("{");
					break;
			}

			TextFieldParser parser = new TextFieldParser(files[0]);
			parser.TextFieldType = FieldType.Delimited;
			parser.SetDelimiters(",");

			int id = 1;

			while (parser.EndOfData == false)
			{
				string[] row = parser.ReadFields();
				string output = "";

				if (row.GetLength(0) != 2)
					continue;

				switch (mOutputType)
				{
				case TYPE_CPP:
					output = "#define " + row[0] + " " + id;
					break;

				case TYPE_CS:
					output = "\t\tpublic const int " + row[0] + " = " + id + ";";
					break;

				case TYPE_JAVA:
					output = "\tpublic static final int " + row[0] + " = " + id + ";";
					break;
				}

				writer.   WriteLine(output);
				writerSid.WriteLine(id + "," + row[1]);
				id++;
			}

			switch (mOutputType)
			{
				case TYPE_CPP:
					break;

				case TYPE_CS:
					writer.WriteLine("\t}");
					writer.WriteLine("}");
					break;

				case TYPE_JAVA:
					writer.WriteLine("}");
					break;
			}

			parser.   Close();
			writer.   Close();
			writerSid.Close();
		}
	}
}
