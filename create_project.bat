@ECHO OFF

ECHO Select the type of project you would like to create:
ECHO 1. Visual Studio 2019 Solution
ECHO 2. Visual Studio 2019 Solution (Android)

CHOICE /N /C:12 /M "[1-2]:"

IF ERRORLEVEL ==2 GOTO TWO
IF ERRORLEVEL ==1 GOTO ONE
GOTO END

:TWO
 ECHO Creating VS2019 Android Project
 external\udCore\bin\premake-bin\premake5.exe vs2019 --os=android %*
 GOTO END

:ONE
 ECHO Creating VS2019 Project...
 external\udCore\bin\premake-bin\premake5.exe vs2019 %*
 GOTO END

:END
