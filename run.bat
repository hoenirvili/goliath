@echo off
xcopy "engine-files" bin /c /d /e /h /i /k /r /s /q
bin\\AnalysisEngine.exe -file:bin\\first_demo.exe -plugins:bin