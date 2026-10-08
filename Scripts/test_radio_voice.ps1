param(
    [string]$StageDirectory = '',
    [int]$Port = 17877,
    [int]$TimeoutSeconds = 150
)

$ErrorActionPreference = 'Stop'
$projectDirectory = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$projectFile = Join-Path $projectDirectory 'Hrono.uproject'
$useCurrentEditor = [string]::IsNullOrWhiteSpace($StageDirectory)
if ($useCurrentEditor) {
    $gameExecutable = 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe'
} else {
    $stagePath = (Resolve-Path -LiteralPath $StageDirectory).Path
    $gameExecutable = Join-Path $stagePath 'Hrono/Binaries/Win64/Hrono.exe'
}
if (!(Test-Path -LiteralPath $gameExecutable)) {
    throw "Missing game executable: $gameExecutable"
}

$testProcesses = @()
$testLogs = @()
try {
    foreach ($role in @('Host', 'Client')) {
        $testLog = Join-Path $projectDirectory "Saved/Logs/RadioVoiceCurrent$role.log"
        $testLogs += $testLog
        if (Test-Path -LiteralPath $testLog) { Remove-Item -LiteralPath $testLog }
        $userDirectory = Join-Path $projectDirectory "Saved/RadioVoiceCurrentTest/$role"
        $startURL = if ($role -eq 'Host') { '/Game/_Alex/DemoMap1?listen' } else { "127.0.0.1:$Port" }
        # CSV commands run after connection/possession rather than during startup.
        # Negative gate thresholds keep packets flowing even in a quiet room.
        # These overrides apply only to the test processes.
        $frameCommands = '380:ONLINE TEST SESSIONHOST LAN,500:HronoVoiceStatus,520:ToggleRadioTransmission,650:HronoVoiceStatus,760:ToggleRadioTransmission,800:HronoVoiceStatus,850:ToggleRadioTransmission,980:HronoVoiceStatus,1090:ToggleRadioTransmission,1130:HronoVoiceStatus,1250:quit'
        $initialCommands = 't.MaxFPS 30,voice.SilenceDetectionThreshold -1,voice.MicNoiseGateThreshold -1'
        $projectArgument = if ($useCurrentEditor) { "`"$projectFile`" " } else { '' }
        $editorGameArgument = if ($useCurrentEditor) { ' -game' } else { '' }
        $arguments = "$projectArgument$startURL$editorGameArgument -port=$Port -nosteam -nullrhi -unattended -nosplash -csvCaptureFrames=1300 -UserDir=`"$userDirectory`" -abslog=`"$testLog`" -ExecCmds=`"$initialCommands`" -csvExecCmds=`"$frameCommands`""
        $testProcesses += Start-Process -FilePath $gameExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru
    }

    foreach ($testProcess in $testProcesses) {
        if (!$testProcess.WaitForExit($TimeoutSeconds * 1000)) {
            throw "Voice test timed out (PID $($testProcess.Id))."
        }
    }

    foreach ($testLog in $testLogs) {
        $logContent = Get-Content -LiteralPath $testLog -Raw
        $role = if ($testLog.EndsWith('Host.log')) { 'Host' } else { 'Client' }
        $expectedNetMode = if ($role -eq 'Host') { 2 } else { 3 }
        if ($logContent -notmatch "Radio voice ready: subsystem=NULL localUser=0 netMode=$expectedNetMode") {
            throw "$role did not initialize local voice in the expected network role. See $testLog"
        }
        $statuses = [regex]::Matches($logContent, '(?s)Radio voice status:.*?(?=\r?\n\[\d{4}\.|\z)')
        if ($statuses.Count -ne 5) { throw "$role has missing voice diagnostics. See $testLog" }
        $expectedStates = @(0, 1, 0, 1, 0)
        for ($stateIndex = 0; $stateIndex -lt $expectedStates.Count; $stateIndex++) {
            $expectedState = $expectedStates[$stateIndex]
            $status = $statuses[$stateIndex].Value
            if ($status -notmatch "requested=$expectedState" -or
                $status -notmatch "IsRecording: $expectedState" -or
                $status -notmatch "Networked: $expectedState") {
                throw "$role has an incorrect switch state at step $stateIndex. See $testLog"
            }
            if ($expectedState -eq 1 -and ($status -notmatch 'Registered: 1' -or
                $status -notmatch 'sessions=1' -or
                $status -notmatch '(?s)Remote Talkers:.*?Talking: 1.*?Muted: 0')) {
                throw "$role did not capture AND receive remote voice at step $stateIndex. See $testLog"
            }
        }
        $switches = [regex]::Matches($logContent, 'Radio microphone (enabled|disabled) for')
        $switchSequence = ($switches | ForEach-Object { $_.Groups[1].Value }) -join ','
        if ($switchSequence -ne 'enabled,disabled,enabled,disabled') {
            throw "$role has an incorrect toggle sequence: $switchSequence. See $testLog"
        }
        Write-Output "$role PASS: OFF -> ON -> OFF -> ON -> OFF, local capture and remote talking detected."
    }
}
finally {
    foreach ($testProcess in $testProcesses) {
        if (!$testProcess.HasExited) { Stop-Process -Id $testProcess.Id -Force }
    }
}
