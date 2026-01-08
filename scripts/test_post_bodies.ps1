# Quick Large POST Test

function Test-LargePost {
    param([int]$SizeKB)
    
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $client.ReceiveTimeout = 10000
        $client.SendTimeout = 10000
        $client.Connect("127.0.0.1", 8888)
        
        $stream = $client.GetStream()
        
        # Create large body
        $body = "A" * ($SizeKB * 1024)
        $contentLength = $body.Length
        
        # Build request
        $request = "POST /post HTTP/1.1`r`n"
        $request += "Host: httpbin.org`r`n"
        $request += "Content-Type: text/plain`r`n"
        $request += "Content-Length: $contentLength`r`n"
        $request += "`r`n"
        $request += $body
        
        Write-Host "Sending ${SizeKB}KB POST request..." -ForegroundColor Cyan
        
        $bytes = [System.Text.Encoding]::ASCII.GetBytes($request)
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush()
        
        # Read response
        $buffer = New-Object byte[] 4096
        $response = ""
        $bytesRead = $stream.Read($buffer, 0, $buffer.Length)
        if ($bytesRead -gt 0) {
            $response = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $bytesRead)
        }
        
        $client.Close()
        
        if ($response -match "HTTP/\d\.\d (\d{3})") {
            $status = [int]$matches[1]
            if ($status -eq 200) {
                Write-Host "  [PASS] ${SizeKB}KB POST - Status $status" -ForegroundColor Green
                return $true
            } else {
                Write-Host "  [FAIL] ${SizeKB}KB POST - Status $status (expected 200)" -ForegroundColor Red
                return $false
            }
        }
        
        Write-Host "  [FAIL] ${SizeKB}KB POST - No valid response" -ForegroundColor Red
        return $false
    }
    catch {
        Write-Host "  [FAIL] ${SizeKB}KB POST - $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  LARGE POST BODY TESTS" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan

$results = @()

# Test various sizes
$results += Test-LargePost -SizeKB 1
Start-Sleep -Milliseconds 500

$results += Test-LargePost -SizeKB 10
Start-Sleep -Milliseconds 500

$results += Test-LargePost -SizeKB 50
Start-Sleep -Milliseconds 500

$results += Test-LargePost -SizeKB 100
Start-Sleep -Milliseconds 500

$passed = ($results | Where-Object {$_ -eq $true}).Count
$total = $results.Count

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Results: $passed/$total PASSED" -ForegroundColor $(if($passed -eq $total){"Green"}else{"Yellow"})
Write-Host "========================================`n" -ForegroundColor Cyan

if ($passed -eq $total) {
    Write-Host "*** All large POST tests passed! ***" -ForegroundColor Green
} else {
    Write-Host "Some tests failed" -ForegroundColor Yellow
}
