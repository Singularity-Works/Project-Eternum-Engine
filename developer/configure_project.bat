@echo off
echo Configuring Eternum Engine for Rider/Visual Studio...

REM Use Visual Studio 2022 generator
cmake -B build -G "Visual Studio 17 2022" -A x64

echo Done. Open build\EternumEngine.sln in Rider or Visual Studio.
pause
