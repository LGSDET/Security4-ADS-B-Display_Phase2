﻿// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("DisplayGUI.cpp", Form1);
USEFORM("checkPassword.cpp", FormPassword);
USEFORM("AreaDialog.cpp", AreaConfirm);
//---------------------------------------------------------------------------
static FILE* pCout = NULL;
static void SetStdOutToNewConsole(void);
//---------------------------------------------------------------------------
static void SetStdOutToNewConsole(void)
{
	// Allocate a console for this app
	AllocConsole();
	//AttachConsole(ATTACH_PARENT_PROCESS);
	freopen_s(&pCout, "CONOUT$", "w", stdout);
}

//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{

	try
	{
		SetStdOutToNewConsole();
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TAreaConfirm), &AreaConfirm);
		Application->Run();
	   if (pCout)
		{
		 fclose(pCout);
		 FreeConsole();
	    }
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
