@echo off
if not defined VS140COMNTOOLS goto VS2015_not_installed
if not defined THIRD_PARTY_BUILD_ROOT goto :Third_party_build_root_var_undefined
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86

set BASE_OUTPUT_PATH=%THIRD_PARTY_BUILD_ROOT%\nmea_3_0_0\lib
IF "%THIRD_PARTY_TMP%"=="" (
	set BASE_INT_PATH=%THIRD_PARTY_BUILD_ROOT%\nmea_3_0_0\Build
) ELSE (
	set BASE_INT_PATH=%THIRD_PARTY_TMP%\nmea_3_0_0\Build
)
set PROJECT=%cd%\src\nmea.vcxproj

msbuild.exe %PROJECT% /t:Clean /p:Platform="x64" /p:OutDir=%BASE_OUTPUT_PATH%\Release_x64\ /p:IntDir=%BASE_INT_PATH%\Release_x64\
msbuild.exe %PROJECT% /t:Clean /p:Platform="x64" /p:OutDir=%BASE_OUTPUT_PATH%\Debug_x64\ /p:IntDir=%BASE_INT_PATH%\Debug_x64\
msbuild.exe %PROJECT% /t:Clean /p:Platform="Win32" /p:OutDir=%BASE_OUTPUT_PATH%\Release_Win32\ /p:IntDir=%BASE_INT_PATH%\Release_Win32\
msbuild.exe %PROJECT% /t:Clean /p:Platform="Win32" /p:OutDir=%BASE_OUTPUT_PATH%\Debug_Win32\ /p:IntDir=%BASE_INT_PATH%\Debug_Win32\

msbuild.exe %PROJECT% /t:Build /p:Configuration="Release" /p:Platform="Win32" /p:OutDir=%BASE_OUTPUT_PATH%\Release_Win32\ /p:IntDir=%BASE_INT_PATH%\Release_Win32\
if not "0"=="%errorlevel%" goto build_failed
msbuild.exe %PROJECT% /p:Configuration="Debug" /p:Platform="Win32" /p:OutDir=%BASE_OUTPUT_PATH%\Debug_Win32\ /p:IntDir=%BASE_INT_PATH%\Debug_Win32\
if not "0"=="%errorlevel%" goto build_failed

msbuild.exe %PROJECT% /p:Configuration="Release" /p:Platform="x64" /p:OutDir=%BASE_OUTPUT_PATH%\Release_x64\ /p:IntDir=%BASE_INT_PATH%\Release_x64\
if not "0"=="%errorlevel%" goto build_failed
msbuild.exe %PROJECT% /t:Build /p:Configuration="Debug" /p:Platform="x64" /p:OutDir=%BASE_OUTPUT_PATH%\Debug_x64\ /p:IntDir=%BASE_INT_PATH%\Debug_x64\
if not "0"=="%errorlevel%" goto build_failed

goto :end

:build_failed
    cd "%~dp0"
    echo Error - build failed.
    pause
    exit /b 1

:Third_party_build_root_var_undefined
    echo Error - THIRD_PARTY_BUILD_ROOT undefined.
    pause
    exit /b 1

:VS2015_not_installed
    echo Error - VS140COMNTOOLS environment variable is not defined.
    echo Visual Studio 2015 must be installed.
    pause
    exit /b 1

:end
