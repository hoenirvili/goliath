@echo off
xcopy "engine-files" bin /c /d /e /h /i /k /r /s /q
xcopy "engine-files\APIExceptions.txt" . /c /d /e /h /i /k /r /s /q
bin\\AnalysisEngine.exe -file:bin\\second_demo.exe -plugins:bin