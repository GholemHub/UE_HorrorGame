param([int]$Port = 17937)
$ErrorActionPreference = 'Stop'
$projectDirectory = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$outputDirectory = Join-Path $projectDirectory 'Saved/Tests/Mannequin/Fear/UnconditionalNetwork'
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$gameExecutable = 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe'
$processes = [System.Collections.Generic.List[System.Diagnostics.Process]]::new()
function Start-FearProcess($role, $url, $commands, $frames) {
    $log = Join-Path $outputDirectory "$role.log"
    $userDirectory = Join-Path $outputDirectory $(if ($role -eq 'Reconnect') { 'Client' } else { $role })
    $projectFile = Join-Path $projectDirectory 'Hrono.uproject'
    $arguments = "`"$projectFile`" $url -game -port=$Port -nosteam -nullrhi -unattended -nosplash -PktLag=100 -PktLoss=2 -csvCaptureFrames=$frames -UserDir=`"$userDirectory`" -abslog=`"$log`" -ExecCmds=`"t.MaxFPS 30`" -csvExecCmds=`"$commands`""
    $process = Start-Process -FilePath $gameExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru
    $processes.Add($process)
    return $process
}
try {
    $hostCommands = '380:ONLINE TEST SESSIONHOST LAN,480:Mannequin.FearProbe prepare,600:Mannequin.FearProbe stage,650:Mannequin.FearProbe watch,700:Mannequin.FearProbe start,740:Mannequin.FearProbe status,810:Mannequin.FearProbe status,900:Mannequin.FearProbe status,1000:Mannequin.FearProbe watch,1050:Mannequin.FearProbe status,1120:Mannequin.FearProbe status,1200:Mannequin.FearProbe away,1280:Mannequin.FearProbe status,1450:Mannequin.FearProbe status,1750:Mannequin.FearProbe watch,1850:Mannequin.FearProbe status,1950:Mannequin.FearProbe away,2000:Mannequin.FearProbe status,2100:Mannequin.FearProbe stage,2150:Mannequin.FearProbe away,2250:Mannequin.FearProbe status,2800:Mannequin.FearProbe end,3570:Mannequin.FearProbe status,3700:Mannequin.FearProbe cleanup,3750:Mannequin.FearProbe status,3900:quit'
    $clientCommands = '480:Mannequin.FearProbe prepare,650:Mannequin.FearProbe watch,750:Mannequin.FearProbe status,930:Mannequin.FearProbe status,1100:Mannequin.FearProbe status,1150:Mannequin.FearProbe watch,1250:Mannequin.FearProbe status,1350:Mannequin.FearProbe away,1430:Mannequin.FearProbe status,1500:quit'
    $hostProcess = Start-FearProcess 'Host' '/Game/_Alex/DemoMap1?listen' $hostCommands 3920
    $clientProcess = Start-FearProcess 'Client' "127.0.0.1:$Port" $clientCommands 1520
    $deadline = [DateTime]::UtcNow.AddSeconds(240)
    while (!$clientProcess.WaitForExit(1000)) {
        if ([DateTime]::UtcNow -gt $deadline) { throw 'Initial client timed out.' }
    }
    $reconnectCommands = '100:Mannequin.FearProbe prepare,150:Mannequin.FearProbe away,250:Mannequin.FearProbe status,450:Mannequin.FearProbe away,550:Mannequin.FearProbe status,850:Mannequin.FearProbe status,1000:Mannequin.FearProbe status,1800:Mannequin.FearProbe status,2200:Mannequin.FearProbe status,2400:Mannequin.FearProbe status,2500:quit'
    $reconnectProcess = Start-FearProcess 'Reconnect' "127.0.0.1:$Port" $reconnectCommands 2520
    foreach ($process in @($hostProcess, $reconnectProcess)) {
        while (!$process.WaitForExit(1000)) {
            if ([DateTime]::UtcNow -gt $deadline) { throw 'Network test timed out.' }
        }
    }
    foreach ($role in @('Host', 'Client', 'Reconnect')) {
        $lines = Get-Content -LiteralPath (Join-Path $outputDirectory "$role.log")
        $samples = @($lines | Select-String '\[FearProbe\]')
        if ($samples.Count -lt 4) { throw "$role has insufficient samples." }
        $expectedMode = if ($role -eq 'Host') { 2 } else { 3 }
        if (!($samples -match "net=$expectedMode")) { throw "$role has incorrect net mode." }
        if (!($samples -match 'active=1')) { throw "$role did not observe active fear." }
        $samples | ForEach-Object { $_.Line }
    }
    Write-Output 'NETWORK_PROBE_COMPLETED: inspect timestamps and sampled animation positions; this is a headless check.'
}
finally {
    foreach ($process in $processes) {
        if (!$process.HasExited) { Stop-Process -Id $process.Id -Force }
    }
}
