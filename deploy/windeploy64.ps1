Set-Variable -name QtDir -value C:\Qt\5.9.1\msvc2015_64
Set-Variable -name BuildDir -value ..\build-bmsone-Desktop_Qt_5_9_1_MSVC2015_64bit-Release
Set-Variable -name OutDir -value BmsONE

rd -r $OutDir
New-Item -ItemType directory -Path $OutDir

Copy-Item "$BuildDir\release\BmsONE.exe" -Destination "$OutDir\BmsONE.exe"
$QtDir+"\bin\windeployqt"
$OutDir+"\BmsONE.exe"
Copy-Item "$QtDir\bin\translations\qtbase_*.qm" -Destination "$OutDir\translations"

Copy-Item "..\docs\readme*.txt" -Destination "$OutDir"
Copy-Item "..\COPYING" -Destination "$OutDir" -recurse
pause