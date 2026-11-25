$ErrorActionPreference = "Stop"

$runtimeDir = Join-Path $env:TEMP "kv-store-integration-test"
New-Item -ItemType Directory -Force -Path $runtimeDir | Out-Null

Copy-Item ".\server.exe" $runtimeDir -Force

$server = $null
$client = $null

try {
    $server = Start-Process `
        -FilePath (Join-Path $runtimeDir "server.exe") `
        -WorkingDirectory $runtimeDir `
        -PassThru `
        -NoNewWindow `
        -RedirectStandardOutput (Join-Path $runtimeDir "server.log") `
        -RedirectStandardError (Join-Path $runtimeDir "server-error.log")

    # Wait for the server to start listening.
    $ready = $false

    for ($i = 0; $i -lt 20; $i++) {
        if ($server.HasExited) {
            throw "Server exited unexpectedly. Check $runtimeDir\server-error.log"
        }

        $probe = $null
        try {
            $probe = [System.Net.Sockets.TcpClient]::new()
            $probe.Connect("127.0.0.1", 5000)
            $ready = $true
            break
        }
        catch {
            Start-Sleep -Milliseconds 500
        }
        finally {
            if ($probe) { $probe.Dispose() }
        }
    }

    if (-not $ready) {
        throw "Server did not start listening on port 5000."
    }

    $client = [System.Net.Sockets.TcpClient]::new()
    $client.Connect("127.0.0.1", 5000)

    $stream = $client.GetStream()
    $reader = [System.IO.StreamReader]::new($stream)
    $writer = [System.IO.StreamWriter]::new($stream)
    $writer.AutoFlush = $true

    function Test-Command($command, $expected) {
        $writer.WriteLine($command)
        $actual = $reader.ReadLine()

        if ($actual -ne $expected) {
            throw "FAIL: $command | Expected '$expected', got '$actual'"
        }

        Write-Host "PASS: $command -> $actual" -ForegroundColor Green
    }

    Test-Command "PING" "PONG"
    Test-Command "SET integration_test Piyush" "OK"
    Test-Command "GET integration_test" "Piyush"
    Test-Command "EXISTS integration_test" "1"
    Test-Command "DEL integration_test" "OK"
    Test-Command "GET integration_test" "NOT_FOUND"
    Test-Command "EXISTS integration_test" "0"
    Test-Command "INVALID" "UNKNOWN_COMMAND"

    Write-Host "`nALL INTEGRATION TESTS PASSED!" -ForegroundColor Green
}
finally {
    if ($client) { $client.Dispose() }

    if ($server -and -not $server.HasExited) {
        Stop-Process -Id $server.Id -Force
    }
}
