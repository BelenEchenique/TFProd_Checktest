@REM @echo off
@REM Title "flashing ToolFES...."
@REM CD  C:\Users\Belen.LAPTOP-CSH9QT2D\Desktop\ProyectosBelen\BLE_ToolFES\armgcc 
@REM cls
@REM DEL /Q /S .\_build
@REM RMDIR /S /Q .\_build
@REM make
@REM pyocd flash -t nrf51 ./_build/nrf51422_xxac.hex

@echo off
Title "flashing NRF51...."
CD   E:\Respaldo\Escritorio\ProyectosBelen\BLE_ToolFES\builddir
cls
ninja
pyocd flash -t nrf51 ./nrf51_ble_test_project.hex
cd E:\Respaldo\Escritorio\ProyectosBelen\BLE_ToolFES