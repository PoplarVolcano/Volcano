@echo off

if exist *.sln del /s /q *.sln
for /r . %%f in (*.vcxproj) do del /q "%%f"
for /r . %%f in (*.vcxproj.filters) do del /q "%%f"
for /r . %%f in (*.vcxproj.user) do del /q "%%f"

call vendor\bin\premake\premake5.exe vs2026

pause