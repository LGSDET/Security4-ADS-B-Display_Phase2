#include <vcl.h>
#pragma hdrstop          
#pragma resource "*.dfm"
#include "checkPassword.h" 

__fastcall TFormPassword::TFormPassword(TComponent* Owner)
	: TForm(Owner)
{
}

void __fastcall TFormPassword::btnOkClick(TObject *Sender)
{
    const String VALID_PASSWORD = "1234";

    if (edtPassword->Text == VALID_PASSWORD) {
        ModalResult = mrOk; // 비밀번호 맞으면 OK
    } else {
        ShowMessage(L"비밀번호가 틀렸습니다.");
        edtPassword->Clear();
        edtPassword->SetFocus();
    }
}

void __fastcall TFormPassword::titleClick(TObject *Sender)
{
}
