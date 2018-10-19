ninja -C build -j2cfgtracetest
$process = Start-Process ctest -ArgumentList "-VV -j2 -C Debug" -NoNewWindow -PassThru -WorkingDirectory "build/tests"
$process.WaitForExit()
#drmemory -no_follow_children -brief bin/cfgtracetest.exe