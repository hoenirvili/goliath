@echo off
del /f /q *.log *.png *.dbt *.dot 2>nul
rmdir /s /q build 2>nul
rmdir /s /q bin 2>nul
rmdir /s /q demo\bin\ 2>nul