@echo off
setlocal EnableDelayedExpansion
SET LOCALPORT = COM4
:startowanie
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%USERPROFILE%\.platformio\packages\framework-arduinoespressif8266\tools\espota.py" --debug --progress -i 10.48.18.29  -f .pio\build\esp12e\firmware.bin

"%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\tools\python3\3.7.2-post1/python3" -I "%USERPROFILE%\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2/tools/upload.py" --chip esp8266 --port COM4 --baud 921600 --before default_reset --after hard_reset erase_flash write_flash 0x0 .pio\build\esp12e\firmware.bin


echo %errorlevel%
echo !errorlevel!
if  errorlevel 1 goto startowanie
pause