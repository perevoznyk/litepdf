object DelphiFrm: TDelphiFrm
  Left = 0
  Top = 0
  BorderStyle = bsToolWindow
  Caption = 'litePDF Delphi Example'
  ClientHeight = 321
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
  object DivideBvl: TBevel
    Left = 8
    Top = 80
    Width = 209
    Height = 2
  end
  object HelloWorldBtn: TButton
    Left = 32
    Top = 16
    Width = 161
    Height = 25
    Caption = 'Create &Hello World PDF'
    TabOrder = 0
    OnClick = HelloWorldBtnClick
  end
  object OpenHelloWorldBtn: TButton
    Left = 32
    Top = 47
    Width = 161
    Height = 25
    Caption = '&Open Hello World PDF'
    TabOrder = 1
    OnClick = OpenHelloWorldBtnClick
  end
  object AttachmentsBtn: TButton
    Left = 32
    Top = 88
    Width = 161
    Height = 25
    Caption = '&attachments'
    TabOrder = 2
    OnClick = AttachmentsBtnClick
  end
  object DocinfoBtn: TButton
    Left = 32
    Top = 120
    Width = 161
    Height = 25
    Caption = 'doc&info'
    TabOrder = 3
    OnClick = DocinfoBtnClick
  end
  object DrawtoresourceBtn: TButton
    Left = 32
    Top = 152
    Width = 161
    Height = 25
    Caption = 'drawto&resource'
    TabOrder = 4
    OnClick = DrawtoresourceBtnClick
  end
  object EncryptBtn: TButton
    Left = 32
    Top = 184
    Width = 161
    Height = 25
    Caption = '&encrypt'
    TabOrder = 5
    OnClick = EncryptBtnClick
  end
  object FromdocBtn: TButton
    Left = 32
    Top = 216
    Width = 161
    Height = 25
    Caption = '&fromdoc'
    TabOrder = 6
    OnClick = FromdocBtnClick
  end
  object PagesperpageBtn: TButton
    Left = 32
    Top = 248
    Width = 161
    Height = 25
    Caption = '&pagesperpage'
    TabOrder = 7
    OnClick = PagesperpageBtnClick
  end
  object SignManualBtn: TButton
    Left = 32
    Top = 280
    Width = 161
    Height = 25
    Caption = '&sign manual'
    TabOrder = 8
    OnClick = SignManualBtnClick
  end
end
