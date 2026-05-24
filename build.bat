@echo off
taskkill /IM keycustom.exe /F >nul 2>nul
del keycustom.exe >nul 2>nul
"C:\winlibs\mingw64\bin\g++.exe" main.cpp -municode -o keycustom.exe -mwindows
pause
