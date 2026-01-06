# Build and Installation Guide

## System Requirements

- **Operating System:** Windows 10/11 (64-bit)
- **Compiler:** g++ (MinGW-w64) with C++17 support
- **Libraries:** Winsock2 (ws2_32.lib) - included with Windows SDK
- **Build Tool:** GNU Make (optional, for Makefile usage)
- **RAM:** Minimum 2GB
- **Disk Space:** 50MB for source and binaries

## Quick Start

### Option 1: Using Makefile (Recommended)

```bash
# Build all components
make

# Build and run server
make run

# Build and run all tests
make test

# Clean and rebuild
make clean
make all
```

### Option 2: Manual Compilation

```powershell
# Create bin directory
mkdir bin

# Compile proxy server
g++ src\proxy_server.cpp -o bin\proxy_server.exe -lws2_32 -std=c++17 -Wall

# Compile test clients
g++ src\test_comprehensive.cpp -o bin\test_comprehensive.exe -lws2_32 -std=c++17 -Wall
g++ src\test_edge_cases.cpp -o bin\test_edge_cases.exe -lws2_32 -std=c++17 -Wall
g++ src\test_stress.cpp -o bin\test_stress.exe -lws2_32 -std=c++17 -Wall
```

## Detailed Build Instructions

### Step 1: Install Prerequisites

1. **Install MinGW-w64:**
   - Download from: https://www.mingw-w64.org/
   - Or use package manager: `choco install mingw`
   - Add to PATH: `C:\mingw-w64\bin`

2. **Verify installation:**
   ```powershell
   g++ --version
   # Should show: g++ (MinGW-w64) 8.1.0 or higher
   ```

3. **Install GNU Make (optional):**
   
   **Option 1: Using winget (Recommended)**
   ```powershell
   winget install GnuWin32.Make
   ```
   
   **Option 2: Using Chocolatey**
   ```powershell
   choco install make
   ```
   
   **Option 3: Manual Download**
   - Download from: http://gnuwin32.sourceforge.net/packages/make.htm
   
   **Important: Add to PATH**
   After installation, add GNU Make to your PATH:
   ```powershell
   # Add to PATH (one-time setup)
   $env:Path += ";C:\Program Files (x86)\GnuWin32\bin"
   
   # Make it permanent
   [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files (x86)\GnuWin32\bin", [EnvironmentVariableTarget]::User)
   
   # Verify installation
   make --version
   ```
   
   **Note:** After adding to PATH, restart PowerShell or run the $env:Path command in your current session.

### Step 2: Extract Source Code

```powershell
# Extract to desired location
cd "C:\path\to\project"

# Verify structure
dir
# Should show: src/, bin/, config/, etc.
```

### Step 3: Configure Settings

Edit `config/proxy_config.ini` if needed:

```ini
[Server]
listen_address = 127.0.0.1
listen_port = 8888

[Filtering]
blocklist_file = config/blocked_domains.txt
```

Edit `config/blocked_domains.txt` to add/remove blocked domains.

### Step 4: Build the Project

#### Using Makefile:

```powershell
# Build everything
make all

# Or build specific components
make server          # Proxy server only
make tests           # Test clients only
```

#### Manual compilation:

```powershell
# Compile proxy server
g++ src\proxy_server.cpp -o bin\proxy_server.exe -lws2_32 -std=c++17 -Wall -O2

# Compile test clients (optional)
g++ src\test_comprehensive.cpp -o bin\test_comprehensive.exe -lws2_32 -std=c++17 -Wall
g++ src\test_edge_cases.cpp -o bin\test_edge_cases.exe -lws2_32 -std=c++17 -Wall
g++ src\test_stress.cpp -o bin\test_stress.exe -lws2_32 -std=c++17 -Wall
```

### Step 5: Verify Build

```powershell
# Check compiled files
dir bin\

# Should show:
# proxy_server.exe
# test_comprehensive.exe
# test_edge_cases.exe
# test_stress.exe
```

## Runtime Invocation

### Starting the Proxy Server

#### Basic Invocation:

```powershell
# Start in foreground
.\bin\proxy_server.exe

# Expected output:
# TCP Proxy Server Started
# Listening on 127.0.0.1:8888
# Press Ctrl+C to stop
```

#### Background Invocation:

```powershell
# Start in new window
Start-Process powershell -ArgumentList "-NoExit", "-Command", ".\bin\proxy_server.exe"

# Or start minimized
Start-Process .\bin\proxy_server.exe -WindowStyle Minimized
```

#### With Output Redirection:

```powershell
# Redirect output to file
.\bin\proxy_server.exe > server_output.log 2>&1

# Run in background with logging
Start-Job -ScriptBlock { .\bin\proxy_server.exe }
```

### Using the Proxy

#### Configure Browser:

1. Open browser settings
2. Search for "proxy"
3. Set HTTP Proxy: `127.0.0.1`
4. Set Port: `8888`
5. Save and browse

#### Using cURL:

```bash
# Test with allowed domain
curl -x http://127.0.0.1:8888 http://example.com

# Test with blocked domain (should return 403)
curl -x http://127.0.0.1:8888 http://facebook.com

# POST request through proxy
curl -x http://127.0.0.1:8888 -X POST -d "data=test" http://httpbin.org/post

# With custom headers
curl -x http://127.0.0.1:8888 -H "User-Agent: TestClient" http://example.com
```

#### Using PowerShell:

```powershell
# Set proxy for session
$env:HTTP_PROXY = "http://127.0.0.1:8888"

# Make request
Invoke-WebRequest -Uri "http://example.com"

# Test blocked domain
Invoke-WebRequest -Uri "http://facebook.com"
# Should return: 403 Forbidden

# POST request
Invoke-WebRequest -Uri "http://httpbin.org/post" -Method POST -Body "test=data"
```

### Running Tests

#### Comprehensive Test Suite:

```powershell
# Run all tests using PowerShell scripts
.\test_suite.ps1           # 15 functional tests
.\test_post_bodies.ps1     # 4 POST body tests

# Run stress tests
.\bin\test_stress.exe      # 18 stress tests
```

#### Individual Test Clients:

```powershell
# Comprehensive tests (40 tests)
.\bin\test_comprehensive.exe

# Edge cases (10 tests)
.\bin\test_edge_cases.exe

# Stress tests (18 tests)
.\bin\test_stress.exe
```

#### Using Makefile:

```powershell
# Run all tests
make test
```

### Stopping the Server

```powershell
# If running in foreground
# Press Ctrl+C

# If running in background
Get-Process proxy_server | Stop-Process

# Kill all instances
Get-Process | Where-Object {$_.Name -eq "proxy_server"} | Stop-Process -Force
```

## Configuration Options

### Port Configuration

To change the listening port, modify the source code:

```cpp
// In proxy_server.cpp, line ~500
serverAddr.sin_port = htons(8888);  // Change 8888 to desired port
```

Then recompile:

```powershell
make clean
make server
```

### Domain Blocklist

Edit `config/blocked_domains.txt`:

```
# Add domains (one per line)
facebook.com
youtube.com
twitter.com

# Comments start with #
# Empty lines are ignored
```

No recompilation needed - file is loaded at startup.

### Logging

Logs are written to `proxy.log` in the project directory.

View logs:
```powershell
# View last 20 lines
Get-Content proxy.log -Tail 20

# Follow logs in real-time
Get-Content proxy.log -Wait

# Clear logs
Remove-Item proxy.log
```

## Troubleshooting

### Compilation Errors

**Error: "g++ not found"**
- Install MinGW-w64
- Add to PATH

**Error: "cannot find -lws2_32"**
- Ensure using MinGW (not MSVC)
- Winsock2 should be included

**Warning: "ignoring '#pragma comment'"**
- Safe to ignore (pragma is for MSVC)

### Makefile Errors

**Error: "make: The term 'make' is not recognized"**
- GNU Make is not installed or not in PATH
- Solution:
  ```powershell
  # Check if Make is installed
  winget install GnuWin32.Make
  
  # Add to PATH for current session
  $env:Path += ";C:\Program Files (x86)\GnuWin32\bin"
  
  # Verify
  make --version
  ```

**Error: "process_begin: CreateProcess(NULL, echo., ...) failed"**
- This error occurs with older GnuWin32 Make versions (3.81)
- The Makefile has been updated to be compatible (removed `@echo.` commands)
- If you have an older version of the Makefile, it has been fixed
- Pull latest version from repository or use manual compilation as fallback

### Runtime Errors

**Error: "Address already in use"**
```powershell
# Port 8888 is in use, kill existing process
Get-Process proxy_server | Stop-Process
```

**Error: "WSAStartup failed"**
- Check Windows Firewall
- Run as Administrator if needed

**Error: "Cannot open blocked_domains.txt"**
- Ensure `config/blocked_domains.txt` exists
- Check file path in configuration

### Performance Issues

**Slow connections:**
- Check DNS resolution
- Test with IP addresses instead of domains
- Verify network connectivity

**High memory usage:**
- Monitor with Task Manager
- Check for memory leaks in long-running sessions

## Clean Build

```powershell
# Remove all build artifacts
make clean

# Deep clean (remove bin directory)
make distclean

# Rebuild from scratch
make clean
make all
```

## Platform-Specific Notes

### Windows 10/11

- Firewall may prompt on first run (allow access)
- Some antivirus may flag proxy behavior (whitelist if needed)
- Use PowerShell (not cmd.exe) for best experience

### Windows Server

- May need to adjust Windows Firewall rules
- Consider running as Windows Service (not implemented)

## Next Steps

After successful build:

1. ✅ Run server: `.\bin\proxy_server.exe`
2. ✅ Run tests: `.\test_suite.ps1`
3. ✅ Configure browser to use proxy
4. ✅ Test with real traffic
5. ✅ Review logs: `proxy.log`

## Support

For issues or questions, refer to:
- README.md - Project overview
- Design documentation - Architecture details
- Test results - Expected behavior
