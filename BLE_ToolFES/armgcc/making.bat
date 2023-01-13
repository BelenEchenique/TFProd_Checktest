@echo off
Title "flashing ToolFES...."
CD  C:\Users\Belen.LAPTOP-CSH9QT2D\Desktop\ProyectosBelen\BLE_ToolFES\armgcc
cls
DEL /Q /S .\_build
RMDIR /S /Q .\_build
make