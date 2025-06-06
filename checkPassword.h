//---------------------------------------------------------------------------

#ifndef checkPasswordH
#define checkPasswordH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "SecureLog.h"
//---------------------------------------------------------------------------
class TFormPassword : public TForm
{
__published:	// IDE-managed Components
	TLabel *password;
	TEdit *edtPassword;
	TButton *btnOk;
	TLabel *title;
	TLabel *flightNumber;
	TEdit *flightNumberBox;
	void __fastcall btnOkClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
	__fastcall TFormPassword(TComponent* Owner);
	void __fastcall SetICAOText(AnsiString icao); 
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPassword *FormPassword;
//---------------------------------------------------------------------------
#endif
