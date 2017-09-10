//---------------------------------------------------------------------------

#ifndef CppBuilderUnitH
#define CppBuilderUnitH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TCppBuilderFrm : public TForm
{
__published: // IDE-managed Components
   TButton *CreateBtn;
   TButton *OpenBtn;
   void __fastcall CreateBtnClick(TObject *Sender);
   void __fastcall OpenBtnClick(TObject *Sender);
private: // User declarations
public: // User declarations
   __fastcall TCppBuilderFrm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCppBuilderFrm *CppBuilderFrm;
//---------------------------------------------------------------------------
#endif
