# Complete Test Suite with Proper Counting

function Test-ProxyRequest {
    param([string]$Name, [string]$Request, [int]$Expected, [string]$Category)
    
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $client.ReceiveTimeout = 5000
        $client.SendTimeout = 5000
        $client.Connect("127.0.0.1", 8888)
        
        $stream = $client.GetStream()
        $bytes = [System.Text.Encoding]::ASCII.GetBytes($Request)
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush()
        
        $buffer = New-Object byte[] 16384
        $response = ""
        $bytesRead = $stream.Read($buffer, 0, $buffer.Length)
        if ($bytesRead -gt 0) {
            $response = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $bytesRead)
        }
        
        $client.Close()
        
        if ($response -match "HTTP/\d\.\d (\d{3})") {
            $status = [int]$matches[1]
            if ($status -eq $Expected) {
                Write-Host "  [PASS] $Name" -ForegroundColor Green
                return $true
            }
        }
        Write-Host "  [FAIL] $Name - Expected $Expected" -ForegroundColor Red
        return $false
    }
    catch {
        Write-Host "  [FAIL] $Name - $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  COMPLETE PROXY TEST SUITE" -ForegroundColor White
Write-Host "========================================`n" -ForegroundColor Cyan

$results = @{}

# CATEGORY 1: Basic HTTP Forwarding (PHASE 2 & 4)
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "CATEGORY 1: Basic HTTP Forwarding" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
$cat1 = @()
$cat1 += Test-ProxyRequest "1.1 Standard GET (example.com)" "GET / HTTP/1.1`r`nHost: example.com`r`n`r`n" 200 "Cat1"
Start-Sleep -Milliseconds 300
$cat1 += Test-ProxyRequest "1.2 HEAD Request" "HEAD / HTTP/1.1`r`nHost: example.com`r`n`r`n" 200 "Cat1"
Start-Sleep -Milliseconds 300
$cat1 += Test-ProxyRequest "1.3 POST with Body" "POST /post HTTP/1.1`r`nHost: httpbin.org`r`nContent-Length: 13`r`n`r`n{`"key`":`"val`"}" 200 "Cat1"
Start-Sleep -Milliseconds 300
$cat1 += Test-ProxyRequest "1.4 Query Parameters" "GET /get?test=1 HTTP/1.1`r`nHost: httpbin.org`r`n`r`n" 200 "Cat1"
Start-Sleep -Milliseconds 300
$cat1 += Test-ProxyRequest "1.5 Multiple Headers" "GET /headers HTTP/1.1`r`nHost: httpbin.org`r`nUser-Agent: Test`r`n`r`n" 200 "Cat1"
Start-Sleep -Milliseconds 300
$cat1Pass = ($cat1 | Where-Object {$_ -eq $true}).Count
Write-Host "`nResult: $cat1Pass/5 PASSED`n" -ForegroundColor $(if($cat1Pass -eq 5){"Green"}else{"Red"})

# CATEGORY 2: Domain Filtering (PHASE 3)
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "CATEGORY 2: Domain Filtering (Phase 3)" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
$cat2 = @()
$cat2 += Test-ProxyRequest "2.1 Blocked Domain (facebook.com)" "GET / HTTP/1.1`r`nHost: facebook.com`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2 += Test-ProxyRequest "2.2 Case Insensitive (YOUTUBE.COM)" "GET / HTTP/1.1`r`nHost: YOUTUBE.COM`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2 += Test-ProxyRequest "2.3 Subdomain (www.twitter.com)" "GET / HTTP/1.1`r`nHost: www.twitter.com`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2 += Test-ProxyRequest "2.4 Port Handling (netflix.com:80)" "GET / HTTP/1.1`r`nHost: netflix.com:80`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2 += Test-ProxyRequest "2.5 Blocked (instagram.com)" "GET / HTTP/1.1`r`nHost: instagram.com`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2 += Test-ProxyRequest "2.6 Blocked (reddit.com)" "GET / HTTP/1.1`r`nHost: reddit.com`r`n`r`n" 403 "Cat2"
Start-Sleep -Milliseconds 300
$cat2Pass = ($cat2 | Where-Object {$_ -eq $true}).Count
Write-Host "`nResult: $cat2Pass/6 PASSED`n" -ForegroundColor $(if($cat2Pass -eq 6){"Green"}else{"Red"})

# CATEGORY 3: Error Handling (PHASE 2 & 3)
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "CATEGORY 3: Error Handling" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
$cat3 = @()
$cat3 += Test-ProxyRequest "3.1 Missing Host Header (400)" "GET / HTTP/1.1`r`n`r`n" 400 "Cat3"
Start-Sleep -Milliseconds 300
$cat3 += Test-ProxyRequest "3.2 HTTP/1.0 Support" "GET / HTTP/1.0`r`nHost: example.com`r`n`r`n" 200 "Cat3"
Start-Sleep -Milliseconds 300
$cat3Pass = ($cat3 | Where-Object {$_ -eq $true}).Count
Write-Host "`nResult: $cat3Pass/2 PASSED`n" -ForegroundColor $(if($cat3Pass -eq 2){"Green"}else{"Red"})

# CATEGORY 4: Large Transfers (PHASE 4 - Streaming)
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "CATEGORY 4: Large Response Streaming" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
$cat4 = @()
$cat4 += Test-ProxyRequest "4.1 Large HTML (3.7KB)" "GET /html HTTP/1.1`r`nHost: httpbin.org`r`n`r`n" 200 "Cat4"
Start-Sleep -Milliseconds 300
$cat4 += Test-ProxyRequest "4.2 Stream 50 objects (9KB+)" "GET /stream/50 HTTP/1.1`r`nHost: httpbin.org`r`n`r`n" 200 "Cat4"
Start-Sleep -Milliseconds 300
$cat4Pass = ($cat4 | Where-Object {$_ -eq $true}).Count
Write-Host "`nResult: $cat4Pass/2 PASSED`n" -ForegroundColor $(if($cat4Pass -eq 2){"Green"}else{"Red"})
# CATEGORY 5: Concurrent Client Handling (PHASE 5)
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "CATEGORY 5: Concurrent Client Handling" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "Testing 10 simultaneous client connections..." -ForegroundColor Gray

$cat5 = @()
$jobs = @()

# Launch 10 concurrent requests using background jobs
1..10 | ForEach-Object {
    $jobNum = $_
    $jobs += Start-Job -ScriptBlock {
        param($num)
        try {
            $client = New-Object System.Net.Sockets.TcpClient
            $client.ReceiveTimeout = 5000
            $client.SendTimeout = 5000
            $client.Connect("127.0.0.1", 8888)
            
            $stream = $client.GetStream()
            $request = "GET / HTTP/1.1`r`nHost: example.com`r`n`r`n"
            $bytes = [System.Text.Encoding]::ASCII.GetBytes($request)
            $stream.Write($bytes, 0, $bytes.Length)
            $stream.Flush()
            
            $buffer = New-Object byte[] 8192
            $response = ""
            $bytesRead = $stream.Read($buffer, 0, $buffer.Length)
            if ($bytesRead -gt 0) {
                $response = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $bytesRead)
            }
            
            $client.Close()
            
            if ($response -match "HTTP/\d\.\d 200") {
                return @{Success=$true; JobNum=$num}
            }
            return @{Success=$false; JobNum=$num}
        }
        catch {
            return @{Success=$false; JobNum=$num; Error=$_.Exception.Message}
        }
    } -ArgumentList $jobNum
}

# Wait for all jobs to complete
$jobs | Wait-Job | Out-Null

# Collect results
$successCount = 0
$failCount = 0
foreach ($job in $jobs) {
    $result = Receive-Job -Job $job
    if ($result.Success) {
        $successCount++
        Write-Host "  [PASS] Concurrent Request #$($result.JobNum)" -ForegroundColor Green
    } else {
        $failCount++
        Write-Host "  [FAIL] Concurrent Request #$($result.JobNum)" -ForegroundColor Red
    }
}

# Clean up jobs
$jobs | Remove-Job

$cat5Pass = if ($successCount -ge 8) { 1 } else { 0 }
Write-Host "`nResult: $successCount/10 requests successful (Pass threshold: 8/10)" -ForegroundColor $(if($cat5Pass -eq 1){"Green"}else{"Red"})
Write-Host "This demonstrates thread-per-connection concurrency model`n" -ForegroundColor Gray
# FINAL SUMMARY
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  FINAL RESULTS" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Category 1 (Basic Forwarding):  $cat1Pass/5 PASSED" -ForegroundColor $(if($cat1Pass -eq 5){"Green"}else{"Red"})
Write-Host "Category 2 (Domain Filtering):  $cat2Pass/6 PASSED" -ForegroundColor $(if($cat2Pass -eq 6){"Green"}else{"Red"})
Write-Host "Category 3 (Error Handling):    $cat3Pass/2 PASSED" -ForegroundColor $(if($cat3Pass -eq 2){"Green"}else{"Red"})
Write-Host "Category 4 (Large Transfers):   $cat4Pass/2 PASSED" -ForegroundColor $(if($cat4Pass -eq 2){"Green"}else{"Red"})
Write-Host "Category 5 (Concurrent):        $cat5Pass/1 PASSED" -ForegroundColor $(if($cat5Pass -eq 1){"Green"}else{"Red"})
Write-Host "----------------------------------------" -ForegroundColor Cyan

$totalTests = 5 + 6 + 2 + 2 + 1
$totalPass = $cat1Pass + $cat2Pass + $cat3Pass + $cat4Pass + $cat5Pass
Write-Host "Total Tests: $totalTests" -ForegroundColor White
Write-Host "Total Passed: $totalPass" -ForegroundColor Green
Write-Host "Total Failed: $($totalTests - $totalPass)" -ForegroundColor $(if($totalPass -eq $totalTests){"Green"}else{"Red"})
Write-Host "Success Rate: $([math]::Round(100.0 * $totalPass / $totalTests, 1))%" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

if ($totalPass -eq $totalTests) {
    Write-Host "*** ALL TESTS PASSED ***`n" -ForegroundColor Green
} else {
    Write-Host "Some tests failed - review results above`n" -ForegroundColor Yellow
}
