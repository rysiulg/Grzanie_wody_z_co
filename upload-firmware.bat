@echo off
setlocal EnableDelayedExpansion
:startowanie
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif8266\tools\espota.py" --debug --progress -i 10.48.18.29  -f .pio\build\esp12e\firmware.bin


echo %errorlevel%
echo !errorlevel!
if  errorlevel 1 goto startowanie
pause