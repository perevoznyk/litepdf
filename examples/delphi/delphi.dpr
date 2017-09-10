program delphi;

{*
 * (c) 2013 http://www.litePDF.cz
 *
 * The example code is supplied "AS IS". It disclaims all warranties, expressed
 * or implied, including, without limitation, the warranties of merchantability
 * and of fitness for any purpose. It assumes no liability for direct, indirect,
 * incidental, special, exemplary, or consequential damages, which may result
 * from the use of the code, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee.
 *}

uses
  Vcl.Forms,
  DelphiUnit in 'DelphiUnit.pas' {DelphiFrm},
  litePDF in '..\..\share\litePDF.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TDelphiFrm, DelphiFrm);
  Application.Run;
end.
