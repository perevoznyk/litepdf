object CppBuilderFrm: TCppBuilderFrm
  Left = 0
  Top = 0
  Caption = 'litePDF C++ Builder Example'
  ClientHeight = 100
  ClientWidth = 225
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object CreateBtn: TButton
    Left = 32
    Top = 24
    Width = 161
    Height = 25
    Caption = '&Create Hello World PDF'
    TabOrder = 0
    OnClick = CreateBtnClick
  end
  object OpenBtn: TButton
    Left = 32
    Top = 55
    Width = 161
    Height = 25
    Caption = '&Open Created PDF'
    TabOrder = 1
    OnClick = OpenBtnClick
  end
end
