ninja -C build -j8 cfgtracetest
$process = Start-Process ctest -ArgumentList "-VV -j8 -C Release --build-and-test" -NoNewWindow -PassThru -WorkingDirectory "build"
$process.WaitForExit()