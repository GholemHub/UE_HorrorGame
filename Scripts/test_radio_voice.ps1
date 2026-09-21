param(
    [string]$StageDirectory = (Join-Path $PSScriptRoot '../Saved/RadioVoiceTestBuild/Windows'),
    [int]$Port = 17877
)

$ErrorActionPreference = 'Stop'
$projectDirectory = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$stagePath = (Resolve-Path -LiteralPath $StageDirectory).Path
$gameExecutable = Join-Path $stagePath 'Hrono/Binaries/Win64/Hrono.exe'
if (!(Test-Path -LiteralPath $gameExecutable)) {
    throw 'A staged Development build is required (Hrono/Binaries/Win64/Hrono.exe).'
}

$testProcesses = @()
$testLogs = @()
try {
    foreach ($role in @('Host', 'Client')) {
        $testLog = Join-Path $projectDirectory "Saved/Logs/RadioVoiceNet$role.log"
        $testLogs += $testLog
        $userDirectory = Join-Path $projectDirectory "Saved/RadioVoiceNetTest/$role"
        $startURL = if ($role -eq 'Host') { '/Game/_Alex/DemoMap1?listen' } else { "127.0.0.1:$Port" }
        # CSV commands run after connection/possession rather than during startup.
        # Negative gate thresholds keep packets flowing even in a quiet room.
        # These overrides apply only to the test processes.
        $frameCommands = '120:HronoVoiceStatus,130:ONLINE TEST SESSIONHOST LAN,150:ToggleRadioTransmission,300:HronoVoiceStatus,450:ToggleRadioTransmission,480:HronoVoiceStatus,540:ToggleRadioTransmission,600:HronoVoiceStatus,750:ToggleRadioTransmission,780:HronoVoiceStatus,900:quit'
        $initialCommands = 't.MaxFPS 30,voice.SilenceDetectionThreshold -1,voice.MicNoiseGateThreshold -1,Log LogNet VeryVerbose'
        $arguments = "$startURL -port=$Port -nosteam -nullrhi -unattended -nosplash -csvCaptureFrames=1200 -UserDir=`"$userDirectory`" -abslog=`"$testLog`" -ExecCmds=`"$initialCommands`" -csvExecCmds=`"$frameCommands`""
        $testProcesses += Start-Process -FilePath $gameExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru
    }

    foreach ($testProcess in $testProcesses) {
        if (!$testProcess.WaitForExit(50000)) {
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
                $status -notmatch '(?s)Remote Talkers:.*?Talking: 1.*?Muted: 0')) {
                throw "$role did not capture AND receive remote voice at step $stateIndex. See $testLog"
            }
        }
        $switches = [regex]::Matches($logContent, 'Radio microphone (enabled|disabled) for')
        $switchSequence = ($switches | ForEach-Object { $_.Groups[1].Value }) -join ','
        if ($switchSequence -ne 'enabled,disabled,enabled,disabled') {
            throw "$role has an incorrect toggle sequence: $switchSequence. See $testLog"
        }
        $packetCount = [regex]::Matches($logContent, 'AddVoicePacket:').Count
        if ($packetCount -eq 0) { throw "$role sent no voice packets. See $testLog" }
        Write-Output "$role PASS: OFF -> ON -> OFF -> ON -> OFF, capture and remote voice; $packetCount queued voice packets."
    }
}
finally {
    foreach ($testProcess in $testProcesses) {
        if (!$testProcess.HasExited) { Stop-Process -Id $testProcess.Id -Force }
    }
}
