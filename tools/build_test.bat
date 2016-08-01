echo off
rem --------------------------------------------------------------------------------------------------------------------
rem Test Build Script for Windows Compilers
rem
rem The script takes two arguments:
rem     1. Build Configuration: Debug or Release
rem     2. Path to third party binaries: E.g. GTest, NUnit
rem
rem Example Running script (from source directory)
rem     .\tools\build_test.bat Debug c:\external
rem
rem Note, you must already have CMake, MinGW and Visual Studio installed and on your path.
rem
rem --------------------------------------------------------------------------------------------------------------------

rem --------------------------------------------------------------------------------------------------------------------
rem MinGW Build Test Script
rem --------------------------------------------------------------------------------------------------------------------

set SOURCE_DIR=..\
set BUILD_PARAM=""
set BUILD_TYPE=Debug
if NOT "%1" == "" (
set BUILD_TYPE=%1
)
if NOT "%2" == "" (
set GTEST_ROOT=%2
set BUILD_PARAM=-DGTEST_ROOT=%GTEST_ROOT% -DGMOCK_ROOT=%GTEST_ROOT% -DNUNIT_ROOT=%GTEST_ROOT%/NUnit-2.6.4
)

echo ##teamcity[blockOpened name='Configure %BUILD_TYPE% MinGW']
mkdir build_mingw_%BUILD_TYPE%
cd build_mingw_debug
echo cmake %SOURCE_DIR% -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %BUILD_PARAM%
cmake %SOURCE_DIR% -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %BUILD_PARAM%
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockClosed name='Configure %BUILD_TYPE%  MinGW']

echo ##teamcity[blockOpened name='Build %BUILD_TYPE%  MinGW']
cmake --build . -- -j8
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockClosed name='Build %BUILD_TYPE%  MinGW']

echo ##teamcity[blockOpened name='Test %BUILD_TYPE%  MinGW']
cmake --build . --target check -- -j8
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockClosed name='Test %BUILD_TYPE%  MinGW']
cd ..

rem --------------------------------------------------------------------------------------------------------------------
rem Visual Studio 14 2015 Build Test Script
rem --------------------------------------------------------------------------------------------------------------------

echo ##teamcity[blockOpened name='Configure %BUILD_TYPE%  Visual Studio 2015 Win64']
mkdir build_vs2015_x64_%BUILD_TYPE%
cd build_vs2015_x64_%BUILD_TYPE%
echo cmake %SOURCE_DIR% -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%  %BUILD_PARAM%
cmake %SOURCE_DIR% -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%  %BUILD_PARAM%
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockOpened name='Configure %BUILD_TYPE%  Visual Studio 2015 Win64']

echo ##teamcity[blockOpened name='Build %BUILD_TYPE%  Visual Studio 2015 Win64']
cmake --build . --config Debug -- /M
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockClosed name='Build %BUILD_TYPE%  Visual Studio 2015 Win64']

echo ##teamcity[blockOpened name='Test %BUILD_TYPE%  Visual Studio 2015 Win64']
cmake --build . --target check --config Debug -- /M
if %errorlevel% neq 0 exit /b %errorlevel%
echo ##teamcity[blockClosed name='Test %BUILD_TYPE%  Visual Studio 2015 Win64']

