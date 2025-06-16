#include <Vcl.Forms.hpp>
#include <System.Classes.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Controls.hpp>
#include "checkPassword_logic.h"
#include <vcl.h>
#pragma hdrstop
#pragma resource "*.dfm"
#include "checkPassword.h"
//#include "AwsConfig.h"  // ✨ 추가


int failedAttempts = 0;

__fastcall TFormPassword::TFormPassword(TComponent* Owner)
    : TForm(Owner)
{
}

void __fastcall TFormPassword::btnOkClick(TObject *Sender)
{
    std::string inputPwd = AnsiString(edtPassword->Text).c_str();

    if (checkPassword(inputPwd)) {
        failedAttempts = 0;
        ShowMessage("Password OK!");
        ModalResult = mrOk;
    } else {
		failedAttempts++;
        ShowMessage(L"Wrong Password.");
        SecureLog::LogWarning("Input Wrong Password.");
        edtPassword->Clear();
        edtPassword->SetFocus();

        if (failedAttempts >= 5)
        {
            ShowMessage(L"5 times this password is incorrect! Login blocked..");
            SecureLog::LogError("5 times this password is incorrect! Login blocked..");
        }
    }
}

void __fastcall TFormPassword::SetICAOText(AnsiString icao)
{
	flightNumberBox->Text = icao;
}

void __fastcall TFormPassword::FormClose(TObject *Sender, TCloseAction &Action)
{
    failedAttempts = 0;
}
