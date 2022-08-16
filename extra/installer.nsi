!include 'LogicLib.nsh'
!include 'MUI2.nsh'

;-----------
; Install directory variables
Var DIR_INSTALL

;-----------
; General
SetCompressor /SOLID lzma
Name "Open Cloud Tools Installer"
OutFile "OpenCloudTools-Install.exe"

;-----------
; MUI Settings
!define MUI_ABORTWARNING

;-----------
; Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE componentsLeave
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_DIRECTORYPAGE_VARIABLE $DIR_INSTALL
;!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "Installation Folder"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section /o "Qt 6 Version" SecQt6
  SetOutPath "$DIR_INSTALL\"
  File /r "install_qt6\"
SectionEnd

Section /o "Qt 5 Version" SecQt5
  SetOutPath "$DIR_INSTALL\"
  File /r "install_qt5\"
SectionEnd

Section /o "Desktop Shortcut" SecDektopShortcut
  CreateShortCut "$DESKTOP\Open Cloud Tools.lnk" "$DIR_INSTALL\OpenCloudTools.exe"
SectionEnd

Section "Start Menu Shortcut" SecStartMenuShortcut
  CreateShortCut "$SMPROGRAMS\Open Cloud Tools.lnk" "$DIR_INSTALL\OpenCloudTools.exe"
SectionEnd

;-----------
; Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 !insertmacro MUI_DESCRIPTION_TEXT ${SecQt6} "This is the main version of OpenCloudTools, requires Windows 10 or newer."
 !insertmacro MUI_DESCRIPTION_TEXT ${SecQt5} "This is the legacy version of OpenCloudTools for older versions of Windows, requires Windows 7 or newer."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;-----------
; Functions

Function componentsLeave
  ${IfNot} ${SectionIsSelected} ${SecQt6}
  ${AndIfNot} ${SectionIsSelected} ${SecQt5}
    MessageBox MB_OK "You must select either Qt 5 or 6."
    Abort
  ${EndIf}
  ${If} ${SectionIsSelected} ${SecQt6}
  ${AndIf} ${SectionIsSelected} ${SecQt5}
    MessageBox MB_OK "You must select only one of either Qt 5 or 6."
    Abort
  ${EndIf}
FunctionEnd

Function .onInit
  StrCpy $DIR_INSTALL "C:\Program Files\OpenCloudTools\"
FunctionEnd
