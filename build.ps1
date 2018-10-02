function Invoke-CmdScript {
    param(
        [String] $scriptName
    )
    If (Test-Path env:VSCMD_VER) { Return }
    $cmdLine = """$scriptName"" $args & set"
    Write-Output "[Running batch script] "$cmdLine
    & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
        select-string '^([^=]*)=(.*)$' | foreach-object {
        $varName = $_.Matches[0].Groups[1].Value
        $varValue = $_.Matches[0].Groups[2].Value
        set-item Env:$varName $varValue
    }
}

Invoke-CmdScript "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 10.0.16299.0
$process = Start-Process cmake -ArgumentList "-DCMAKE_BUILD_TYPE=Release -G Ninja -Bbuild/ -H." -NoNewWindow -PassThru 
#$process = Start-Process cmake -ArgumentList "-G `"Visual Studio 15 2017`" -Bbuild/ -H." -NoNewWindow -PassThru
$process.WaitForExit()
ninja -C build -j8 cfgtrace
ninja -C build -j8 demo
ninja -C build -j8 cfgtracetest