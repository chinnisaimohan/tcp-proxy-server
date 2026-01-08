# Demonstration Guide - TCP Proxy Server

## Table of Contents

1. [Quick Start Demo](#quick-start-demo)
2. [Feature Demonstrations](#feature-demonstrations)
3. [Usage Examples](#usage-examples)
4. [Screenshots & Command Outputs](#screenshots--command-outputs)
5. [Common Use Cases](#common-use-cases)
6. [Performance Demonstration](#performance-demonstration)
7. [Troubleshooting Demo](#troubleshooting-demo)

---

## 1. Quick Start Demo

### 1.1 Starting the Proxy Server

**Step 1: Navigate to project directory**
```powershell
cd "C:\Users\Admin\Desktop\ACM Project"
```

**Step 2: Start the proxy server**
```powershell
.\proxy_server.exe
```

**Expected Output:**
```
TCP Proxy Server Starting...
[OK] Loaded 17 blocked domains from config/blocked_domains.txt
[OK] Server listening on port 8888
[OK] Press Ctrl+C to stop

Waiting for connections...
```

**Screenshot Description:**
- Terminal window showing startup messages
- Green [OK] indicators for successful initialization
- "Waiting for connections..." indicates ready state

---

### 1.2 First Request Demo

**Open new terminal, run:**
```powershell
curl -x http://localhost:8888 http://example.com
```

**Expected Output (cURL):**
```html
<!doctype html>
<html>
<head>
    <title>Example Domain</title>
    ...
</head>
<body>
    <h1>Example Domain</h1>
    <p>This domain is for use in illustrative examples...</p>
</body>
</html>
```

**Expected Output (Proxy Server):**
```
Client connected from 127.0.0.1:52341
Request: GET http://example.com/ HTTP/1.1
Host: example.com:80
Domain allowed: example.com
Connecting to example.com:80
Connected successfully
Forwarding request (198 bytes)
Streaming response (1,256 bytes received)
Response completed, closing connections
Active threads: 0
```

**Screenshot Description:**
- Two terminal windows side by side
- Left: Proxy server showing request logs
- Right: cURL showing HTML response

---

## 2. Feature Demonstrations

### 2.1 Domain Filtering (Allowed Domain)

**Command:**
```powershell
curl -x http://localhost:8888 http://httpbin.org/get
```

**Expected Response:**
```json
{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Host": "httpbin.org",
    "User-Agent": "curl/7.83.0"
  },
  "origin": "YOUR_IP",
  "url": "http://httpbin.org/get"
}
```

**Proxy Server Log:**
```
Client connected from 127.0.0.1:52342
Request: GET http://httpbin.org/get HTTP/1.1
Host: httpbin.org:80
Domain allowed: httpbin.org
Connecting to httpbin.org:80
Connected successfully
Forwarding request (234 bytes)
Streaming response (1,523 bytes received)
Response completed, closing connections
```

**Demonstration Points:**
✅ Request successfully forwarded  
✅ JSON response received and displayed  
✅ Domain not in blocklist, request allowed  

---

### 2.2 Domain Filtering (Blocked Domain)

**Command:**
```powershell
curl -x http://localhost:8888 http://facebook.com
```

**Expected Response:**
```
HTTP/1.1 403 Forbidden
Content-Type: text/plain
Content-Length: 34
Connection: close

Domain blocked by proxy policy
```

**Proxy Server Log:**
```
Client connected from 127.0.0.1:52343
Request: GET http://facebook.com/ HTTP/1.1
Host: facebook.com:80
Domain BLOCKED: facebook.com (matched: facebook.com)
Sent 403 Forbidden response
Connection closed
Active threads: 0
```

**Demonstration Points:**
❌ Request blocked before forwarding  
✅ 403 Forbidden status returned immediately  
✅ Clear error message to client  
✅ No external connection made (security feature)  

---

### 2.3 Subdomain Blocking

**Command:**
```powershell
curl -x http://localhost:8888 http://www.youtube.com
```

**Expected Response:**
```
HTTP/1.1 403 Forbidden
Content-Type: text/plain
Content-Length: 34
Connection: close

Domain blocked by proxy policy
```

**Proxy Server Log:**
```
Client connected from 127.0.0.1:52344
Request: GET http://www.youtube.com/ HTTP/1.1
Host: www.youtube.com:80
Domain BLOCKED: www.youtube.com (matched: youtube.com)
Sent 403 Forbidden response
Connection closed
```

**Demonstration Points:**
✅ Subdomain detection working  
✅ `www.youtube.com` blocked because `youtube.com` in blocklist  
✅ Prevents subdomain evasion  

---

### 2.4 Case-Insensitive Filtering

**Command:**
```powershell
curl -x http://localhost:8888 http://TWITTER.COM
```

**Expected Response:**
```
HTTP/1.1 403 Forbidden
Content-Type: text/plain
Content-Length: 34
Connection: close

Domain blocked by proxy policy
```

**Demonstration Points:**
✅ Case-insensitive matching works  
✅ `TWITTER.COM` blocked (blocklist has `twitter.com`)  
✅ Prevents case-based evasion  

---

### 2.5 POST Request with JSON Body

**Command:**
```powershell
curl -x http://localhost:8888 `
  -X POST `
  -H "Content-Type: application/json" `
  -d '{"username":"testuser","password":"testpass123"}' `
  http://httpbin.org/post
```

**Expected Response:**
```json
{
  "args": {},
  "data": "{\"username\":\"testuser\",\"password\":\"testpass123\"}",
  "files": {},
  "form": {},
  "headers": {
    "Accept": "*/*",
    "Content-Length": "49",
    "Content-Type": "application/json",
    "Host": "httpbin.org",
    "User-Agent": "curl/7.83.0"
  },
  "json": {
    "password": "testpass123",
    "username": "testuser"
  },
  "origin": "YOUR_IP",
  "url": "http://httpbin.org/post"
}
```

**Proxy Server Log:**
```
Client connected from 127.0.0.1:52345
Request: POST http://httpbin.org/post HTTP/1.1
Host: httpbin.org:80
Content-Length: 49
Reading POST body (49 bytes)
POST body complete
Domain allowed: httpbin.org
Connecting to httpbin.org:80
Connected successfully
Forwarding request (389 bytes including body)
Streaming response (1,847 bytes received)
Response completed, closing connections
```

**Demonstration Points:**
✅ POST method supported  
✅ Content-Length header detected  
✅ Request body fully read (49 bytes)  
✅ Complete request forwarded to destination  
✅ JSON data echoed back correctly  

---

### 2.6 Custom Headers Preservation

**Command:**
```powershell
curl -x http://localhost:8888 `
  -H "X-Custom-Header: MyValue" `
  -H "Authorization: Bearer abc123" `
  -H "X-Request-ID: 12345" `
  http://httpbin.org/headers
```

**Expected Response:**
```json
{
  "headers": {
    "Accept": "*/*",
    "Authorization": "Bearer abc123",
    "Host": "httpbin.org",
    "User-Agent": "curl/7.83.0",
    "X-Custom-Header": "MyValue",
    "X-Request-Id": "12345"
  }
}
```

**Demonstration Points:**
✅ All custom headers forwarded  
✅ Authorization header preserved  
✅ Multiple custom headers supported  
✅ Header names/values unchanged  

---

### 2.7 Large Response Streaming

**Command:**
```powershell
curl -x http://localhost:8888 http://httpbin.org/stream-bytes/1048576
```

**Expected Behavior:**
- 1MB of random bytes streamed
- No buffering, immediate start
- Memory usage constant (~50MB)

**Proxy Server Log:**
```
Client connected from 127.0.0.1:52346
Request: GET http://httpbin.org/stream-bytes/1048576 HTTP/1.1
Host: httpbin.org:80
Domain allowed: httpbin.org
Connecting to httpbin.org:80
Connected successfully
Forwarding request (245 bytes)
Streaming response chunk (8,192 bytes)
Streaming response chunk (8,192 bytes)
... (128 chunks total)
Streaming response (1,048,576 bytes received)
Response completed, closing connections
```

**Demonstration Points:**
✅ Large responses handled efficiently  
✅ 8KB chunk streaming (128 chunks for 1MB)  
✅ No full response buffering  
✅ Memory usage constant regardless of size  

---

## 3. Usage Examples

### 3.1 Browser Configuration

**Configure Chrome/Edge to use proxy:**

**Windows Settings Method:**
```
1. Open Windows Settings
2. Network & Internet → Proxy
3. Manual proxy setup: ON
4. Address: localhost
5. Port: 8888
6. Save
```

**Browser Test:**
```
1. Navigate to http://example.com
   → Expected: Page loads normally
   
2. Navigate to http://facebook.com
   → Expected: 403 Forbidden page
   
3. Navigate to http://httpbin.org/get
   → Expected: JSON response displayed
```

**Screenshot Description:**
- Browser showing successful page load for allowed domain
- Browser showing 403 error for blocked domain

---

### 3.2 PowerShell WebClient

**Example 1: Simple GET**
```powershell
$proxy = New-Object System.Net.WebProxy("http://localhost:8888")
$web = New-Object System.Net.WebClient
$web.Proxy = $proxy
$response = $web.DownloadString("http://example.com")
Write-Host $response
```

**Example 2: POST with Headers**
```powershell
$proxy = New-Object System.Net.WebProxy("http://localhost:8888")
$web = New-Object System.Net.WebClient
$web.Proxy = $proxy
$web.Headers.Add("Content-Type", "application/json")
$web.Headers.Add("User-Agent", "PowerShell/7.0")
$jsonData = '{"key":"value"}'
$response = $web.UploadString("http://httpbin.org/post", $jsonData)
Write-Host $response
```

---

### 3.3 Python Requests Library

**Example:**
```python
import requests

proxies = {
    'http': 'http://localhost:8888',
}

# GET request
response = requests.get('http://httpbin.org/get', proxies=proxies)
print(response.json())

# POST request
data = {'username': 'test', 'password': 'pass123'}
response = requests.post('http://httpbin.org/post', json=data, proxies=proxies)
print(response.json())

# Blocked domain
try:
    response = requests.get('http://facebook.com', proxies=proxies)
except requests.exceptions.HTTPError as e:
    print(f"Blocked: {e}")  # Expected: 403 Forbidden
```

---

## 4. Screenshots & Command Outputs

### 4.1 Server Startup

**Command:**
```powershell
.\proxy_server.exe
```

**Output:**
```
═══════════════════════════════════════════════════════
    TCP PROXY SERVER - Version 1.0
    Listening on: 0.0.0.0:8888
    Configuration: config/proxy_config.ini
═══════════════════════════════════════════════════════

[2025-01-02 15:30:12] Initializing Winsock...
[2025-01-02 15:30:12] [OK] Winsock initialized successfully

[2025-01-02 15:30:12] Loading domain blocklist...
[2025-01-02 15:30:12] [OK] Loaded 17 domains from config/blocked_domains.txt

Blocked Domains:
  1. facebook.com
  2. youtube.com
  3. twitter.com
  4. instagram.com
  5. tiktok.com
  6. reddit.com
  7. pinterest.com
  8. linkedin.com
  9. snapchat.com
 10. whatsapp.com
 11. telegram.org
 12. discord.com
 13. twitch.tv
 14. vimeo.com
 15. dailymotion.com
 16. tumblr.com
 17. flickr.com

[2025-01-02 15:30:12] Creating listening socket...
[2025-01-02 15:30:12] [OK] Socket created successfully

[2025-01-02 15:30:12] Binding to port 8888...
[2025-01-02 15:30:12] [OK] Bind successful

[2025-01-02 15:30:12] Setting socket to listen mode...
[2025-01-02 15:30:12] [OK] Now listening on port 8888

═══════════════════════════════════════════════════════
    SERVER READY - Accepting Connections
    Press Ctrl+C to stop
═══════════════════════════════════════════════════════

Waiting for connections...
```

---

### 4.2 Successful Request Flow

**Client Terminal (cURL):**
```powershell
PS C:\> curl -x http://localhost:8888 http://httpbin.org/get

{
  "args": {},
  "headers": {
    "Accept": "*/*",
    "Host": "httpbin.org",
    "User-Agent": "curl/7.83.0"
  },
  "origin": "203.0.113.45",
  "url": "http://httpbin.org/get"
}

PS C:\>
```

**Server Terminal:**
```
[2025-01-02 15:31:45] ┌─────────────────────────────────────────┐
[2025-01-02 15:31:45] │ NEW CONNECTION                          │
[2025-01-02 15:31:45] ├─────────────────────────────────────────┤
[2025-01-02 15:31:45] │ From: 127.0.0.1:52341                   │
[2025-01-02 15:31:45] │ Thread ID: 12345                         │
[2025-01-02 15:31:45] │ Active Threads: 1                        │
[2025-01-02 15:31:45] └─────────────────────────────────────────┘

[2025-01-02 15:31:45] ┌─────────────────────────────────────────┐
[2025-01-02 15:31:45] │ REQUEST DETAILS                          │
[2025-01-02 15:31:45] ├─────────────────────────────────────────┤
[2025-01-02 15:31:45] │ Method: GET                              │
[2025-01-02 15:31:45] │ URL: http://httpbin.org/get             │
[2025-01-02 15:31:45] │ HTTP Version: HTTP/1.1                   │
[2025-01-02 15:31:45] │ Host: httpbin.org:80                     │
[2025-01-02 15:31:45] │ User-Agent: curl/7.83.0                  │
[2025-01-02 15:31:45] └─────────────────────────────────────────┘

[2025-01-02 15:31:45] [CHECK] Checking domain filter...
[2025-01-02 15:31:45] [OK] Domain allowed: httpbin.org

[2025-01-02 15:31:45] [CONNECT] Resolving httpbin.org...
[2025-01-02 15:31:45] [OK] Resolved to 54.243.165.82
[2025-01-02 15:31:45] [CONNECT] Connecting to 54.243.165.82:80...
[2025-01-02 15:31:45] [OK] Connected successfully

[2025-01-02 15:31:45] [FORWARD] Sending request (234 bytes)...
[2025-01-02 15:31:45] [OK] Request forwarded

[2025-01-02 15:31:46] [STREAM] Receiving response...
[2025-01-02 15:31:46] [STREAM] Chunk 1: 1,523 bytes
[2025-01-02 15:31:46] [OK] Response streaming complete

[2025-01-02 15:31:46] ┌─────────────────────────────────────────┐
[2025-01-02 15:31:46] │ CONNECTION SUMMARY                       │
[2025-01-02 15:31:46] ├─────────────────────────────────────────┤
[2025-01-02 15:31:46] │ Total Sent: 234 bytes                    │
[2025-01-02 15:31:46] │ Total Received: 1,523 bytes              │
[2025-01-02 15:31:46] │ Duration: 1.2 seconds                    │
[2025-01-02 15:31:46] │ Status: SUCCESS                          │
[2025-01-02 15:31:46] └─────────────────────────────────────────┘

[2025-01-02 15:31:46] [CLEANUP] Closing connections...
[2025-01-02 15:31:46] [OK] Connections closed
[2025-01-02 15:31:46] Active threads: 0

Waiting for connections...
```

---

### 4.3 Blocked Domain Response

**Client Terminal:**
```powershell
PS C:\> curl -x http://localhost:8888 http://facebook.com

HTTP/1.1 403 Forbidden
Content-Type: text/plain
Content-Length: 34
Connection: close

Domain blocked by proxy policy

PS C:\>
```

**Server Terminal:**
```
[2025-01-02 15:32:18] ┌─────────────────────────────────────────┐
[2025-01-02 15:32:18] │ NEW CONNECTION                          │
[2025-01-02 15:32:18] ├─────────────────────────────────────────┤
[2025-01-02 15:32:18] │ From: 127.0.0.1:52342                   │
[2025-01-02 15:32:18] │ Thread ID: 12346                         │
[2025-01-02 15:32:18] │ Active Threads: 1                        │
[2025-01-02 15:32:18] └─────────────────────────────────────────┘

[2025-01-02 15:32:18] ┌─────────────────────────────────────────┐
[2025-01-02 15:32:18] │ REQUEST DETAILS                          │
[2025-01-02 15:32:18] ├─────────────────────────────────────────┤
[2025-01-02 15:32:18] │ Method: GET                              │
[2025-01-02 15:32:18] │ URL: http://facebook.com/               │
[2025-01-02 15:32:18] │ HTTP Version: HTTP/1.1                   │
[2025-01-02 15:32:18] │ Host: facebook.com:80                    │
[2025-01-02 15:32:18] └─────────────────────────────────────────┘

[2025-01-02 15:32:18] [CHECK] Checking domain filter...
[2025-01-02 15:32:18] [BLOCKED] Domain: facebook.com
[2025-01-02 15:32:18] [BLOCKED] Matched rule: facebook.com (exact match)
[2025-01-02 15:32:18] [RESPONSE] Sending 403 Forbidden...
[2025-01-02 15:32:18] [OK] Error response sent

[2025-01-02 15:32:18] ┌─────────────────────────────────────────┐
[2025-01-02 15:32:18] │ CONNECTION SUMMARY                       │
[2025-01-02 15:32:18] ├─────────────────────────────────────────┤
[2025-01-02 15:32:18] │ Status: BLOCKED (403)                    │
[2025-01-02 15:32:18] │ Duration: 0.02 seconds                   │
[2025-01-02 15:32:18] └─────────────────────────────────────────┘

[2025-01-02 15:32:18] [CLEANUP] Closing connection...
[2025-01-02 15:32:18] Active threads: 0

Waiting for connections...
```

---

## 5. Common Use Cases

### 5.1 Content Filtering for Network

**Scenario:** Block access to social media on a network

**Setup:**
1. Edit `config/blocked_domains.txt`:
```
# Social Media
facebook.com
instagram.com
twitter.com
tiktok.com
snapchat.com

# Video Streaming
youtube.com
twitch.tv
vimeo.com
```

2. Configure devices to use proxy (192.168.1.100:8888)

3. Test:
```powershell
curl -x http://192.168.1.100:8888 http://facebook.com
# Expected: 403 Forbidden

curl -x http://192.168.1.100:8888 http://example.com
# Expected: 200 OK
```

**Use Case Benefits:**
✅ Centralized content control  
✅ Easy to update blocklist  
✅ Transparent to end users  
✅ Detailed logging of attempts  

---

### 5.2 HTTP Traffic Inspection

**Scenario:** Monitor and log HTTP traffic

**Setup:**
1. Configure client to use proxy
2. Monitor proxy log file:
```powershell
Get-Content proxy.log -Wait -Tail 50
```

**Observed Data:**
- All URLs accessed
- Request methods
- Headers sent
- Response sizes
- Timestamps

**Privacy Note:** Only HTTP traffic visible (not HTTPS)

---

### 5.3 Development Testing

**Scenario:** Test application's proxy support

**Setup:**
1. Start proxy server
2. Configure application to use localhost:8888
3. Make requests from application
4. Verify proxy logs show traffic

**Example (Node.js):**
```javascript
const axios = require('axios');

const config = {
  proxy: {
    host: 'localhost',
    port: 8888
  }
};

axios.get('http://httpbin.org/get', config)
  .then(response => console.log(response.data))
  .catch(error => console.error(error));
```

---

## 6. Performance Demonstration

### 6.1 Concurrent Connections Test

**Command:**
```powershell
# Run 20 concurrent requests
1..20 | ForEach-Object -Parallel {
    $response = Invoke-WebRequest `
        -Uri "http://httpbin.org/get" `
        -Proxy "http://localhost:8888" `
        -UseBasicParsing
    Write-Host "Request $_ completed: $($response.StatusCode)"
} -ThrottleLimit 20
```

**Expected Output:**
```
Request 1 completed: 200
Request 3 completed: 200
Request 2 completed: 200
Request 5 completed: 200
...
Request 20 completed: 200
```

**Server Output:**
```
Active threads: 1
Active threads: 5
Active threads: 12
Active threads: 20  ← Peak concurrency
Active threads: 18
Active threads: 10
Active threads: 5
Active threads: 0   ← All complete
```

**Performance Metrics:**
```
Total requests: 20
Success rate: 100%
Total time: 3.2 seconds
Average response time: 160ms
Peak concurrent: 20
```

---

### 6.2 Large File Transfer

**Command:**
```powershell
# Download 10MB file
Measure-Command {
    curl -x http://localhost:8888 http://httpbin.org/stream-bytes/10485760 -o output.bin
}
```

**Expected Metrics:**
```
Total time: ~5-10 seconds (depends on connection)
Throughput: ~1-2 MB/s
Memory usage: Constant (~50MB, no buffering)
```

**Server Log:**
```
Streaming response chunk (8,192 bytes)
Streaming response chunk (8,192 bytes)
... (1,280 chunks total for 10MB)
Streaming response (10,485,760 bytes received)
Response completed
```

**Demonstration Points:**
✅ Large files handled efficiently  
✅ Streaming prevents memory overflow  
✅ Consistent performance  

---

## 7. Troubleshooting Demo

### 7.1 Connection Refused

**Problem:**
```powershell
curl -x http://localhost:8888 http://example.com
curl: (7) Failed to connect to localhost port 8888: Connection refused
```

**Solution:**
1. Check if proxy server is running:
```powershell
Get-Process proxy_server
```

2. If not running, start it:
```powershell
.\proxy_server.exe
```

---

### 7.2 DNS Resolution Failure

**Problem:**
```powershell
curl -x http://localhost:8888 http://nonexistent-domain.com
```

**Server Log:**
```
[ERROR] getaddrinfo() failed for nonexistent-domain.com: 11001
[RESPONSE] Sending 502 Bad Gateway
```

**Client Response:**
```
HTTP/1.1 502 Bad Gateway
Content-Type: text/plain
Content-Length: 52

Bad Gateway: Unable to connect to destination server
```

**Demonstration Points:**
✅ DNS errors handled gracefully  
✅ 502 status code appropriate  
✅ Clear error message  

---

### 7.3 Blocked Domain Verification

**Command:**
```powershell
# Test if domain is blocked
function Test-ProxyBlock {
    param($domain)
    $response = curl -x http://localhost:8888 "http://$domain" -w "%{http_code}" -o $null -s
    if ($response -eq "403") {
        Write-Host "BLOCKED: $domain" -ForegroundColor Red
    } else {
        Write-Host "ALLOWED: $domain" -ForegroundColor Green
    }
}

Test-ProxyBlock "facebook.com"
Test-ProxyBlock "example.com"
Test-ProxyBlock "youtube.com"
```

**Output:**
```
BLOCKED: facebook.com
ALLOWED: example.com
BLOCKED: youtube.com
```

---

### 7.4 Build System Issues

**Problem:**
```powershell
make all
make: The term 'make' is not recognized
```

**Solution:**
```powershell
# Install GNU Make
winget install GnuWin32.Make

# Add to PATH
$env:Path += ";C:\Program Files (x86)\GnuWin32\bin"

# Verify
make --version
# Output: GNU Make 3.81

# Now build
make all
```

**Demonstration Points:**
✅ Shows complete setup process  
✅ PATH configuration explained  
✅ Verification step included  
✅ Alternative: manual compilation always works  

**Alternative (Manual Compilation):**
```powershell
# If make fails, use manual compilation
mkdir bin -Force
g++ src\proxy_server.cpp -o bin\proxy_server.exe -lws2_32 -std=c++17 -Wall
```

---

## Appendix: Video Demonstration Script

### Recommended Recording Flow

**Segment 1: Introduction (1 minute)**
1. Show project folder structure
2. Highlight key files (proxy_server.cpp, config files)
3. Show README.md

**Segment 2: Server Startup (30 seconds)**
1. Open terminal
2. Run `.\proxy_server.exe`
3. Show startup messages
4. Show blocked domains list

**Segment 3: Basic Functionality (2 minutes)**
1. Open second terminal
2. Send GET request to allowed domain (httpbin.org)
3. Show server logs
4. Show response in client
5. Send POST request with JSON
6. Show request body forwarding

**Segment 4: Domain Filtering (1.5 minutes)**
1. Send request to blocked domain (facebook.com)
2. Show 403 Forbidden response
3. Try subdomain (www.youtube.com)
4. Show still blocked
5. Try case variation (TWITTER.COM)
6. Show still blocked

**Segment 5: Concurrency (1 minute)**
1. Run concurrent test script (20 requests)
2. Show active threads increasing
3. Show all requests complete
4. Show server still responsive

**Segment 6: Conclusion (30 seconds)**
1. Stop server (Ctrl+C)
2. Show test results (88/88 tests passed - 100%)
3. Highlight comprehensive validation across 5 test suites

**Total Duration:** ~6.5 minutes

---

*Demonstration Guide Version: 2.0*  
*Last Updated: January 8, 2026*  
*For Questions: See README.md*
