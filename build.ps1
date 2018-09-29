function Invoke-CmdScript {
    param(
        [String] $scriptName
    )
    $cmdLine = """$scriptName"" $args & set"
    Write-Output "[Running batch script] : "$cmdLine
    & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
        select-string '^([^=]*)=(.*)$' | foreach-object {
        $varName = $_.Matches[0].Groups[1].Value
        $varValue = $_.Matches[0].Groups[2].Value
        set-item Env:$varName $varValue
    }
}
Invoke-CmdScript "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 10.0.16299.0
$build_dir = "build"
If (!(test-path $build_dir)) {
    New-Item -ItemType Directory -Force -Path $build_dir
}

cd build; cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ../
ninja -j8 cfgtrace
ninja -j8 demo
ninja -j8 cfgtracetest