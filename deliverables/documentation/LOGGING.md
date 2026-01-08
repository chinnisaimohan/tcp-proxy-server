# Proxy Server Logging Documentation

## Overview
The proxy server implements comprehensive traffic logging that writes structured data to both the console (for real-time monitoring) and a persistent log file (`proxy.log`) for record-keeping and auditing purposes.

## Log File Location
- **File Name**: `proxy.log`
- **Location**: Same directory as the proxy executable
- **Format**: Plain text, UTF-8 encoded
- **Append Mode**: New entries are appended to preserve historical data

## Logged Information

Each traffic request generates a structured log entry containing:

### 1. **Timestamp**
- **Format**: `YYYY-MM-DD HH:MM:SS`
- **Example**: `2026-01-08 14:30:20`
- **Purpose**: Track exactly when each request occurred

### 2. **Client Details**
- **Format**: `Client: IP_ADDRESS:PORT`
- **Example**: `Client: 127.0.0.1:52341` or `Client: 192.168.1.100:53012`
- **Purpose**: Identify which computer/user made the request

### 3. **Requested Destination**
- **Format**: `Destination: HOSTNAME:PORT`
- **Example**: `Destination: example.com:80`
- **Purpose**: Track what websites/services are being accessed through the proxy

### 4. **Request Line**
- **Format**: `Request: METHOD URL HTTP_VERSION`
- **Example**: `Request: GET http://example.com/ HTTP/1.1`
- **Purpose**: Record the exact HTTP request being made

### 5. **Action Taken**
- **Values**: `ALLOWED` or `BLOCKED`
- **Example**: `Action: ALLOWED` or `Action: BLOCKED`
- **Purpose**: Track whether the proxy permitted or denied the request

### 6. **HTTP Status Code**
- **Format**: `Status Code: CODE`
- **Example**: `Status Code: 200` (success) or `Status Code: 403` (forbidden)
- **Purpose**: Record the HTTP response status
- **Note**: Only present for allowed requests that complete successfully

### 7. **Bytes Transferred**
- **Format**: `Bytes Transferred: NUMBER`
- **Example**: `Bytes Transferred: 1256`
- **Purpose**: Track bandwidth usage and response sizes
- **Note**: Shows 0 for blocked requests (no data transferred)

## Sample Log Entry (Allowed Request)

```
========================================
[TRAFFIC LOG]
Timestamp: 2026-01-08 14:30:20
Client: 127.0.0.1:52341
Destination: example.com:80
Request: GET http://example.com/ HTTP/1.1
Action: ALLOWED
Status Code: 200
Bytes Transferred: 1256
========================================
```

## Sample Log Entry (Blocked Request)

```
========================================
[TRAFFIC LOG]
Timestamp: 2026-01-08 14:30:25
Client: 127.0.0.1:52342
Destination: facebook.com:80
Request: GET http://facebook.com/ HTTP/1.1
Action: BLOCKED
Status Code: 403
Bytes Transferred: 0
========================================
```

## Thread-Safe Implementation

The logging system uses mutex locks to ensure thread safety in the multi-threaded environment:

- **`logMutex`**: Protects console output and timestamp generation
- **`fileMutex`**: Protects file write operations
- **Result**: Multiple concurrent requests can log safely without data corruption

## Use Cases

### 1. **Monitoring**
- Real-time visibility into proxy activity
- Track what sites users are accessing
- Monitor bandwidth usage

### 2. **Auditing**
- Historical record of all proxy traffic
- Compliance verification
- Security incident investigation

### 3. **Analytics**
- Identify most-accessed domains
- Calculate total data transferred
- Analyze traffic patterns by time/client

### 4. **Troubleshooting**
- Debug connection issues
- Verify blocking rules are working
- Investigate performance problems

## Log File Management

### Viewing Logs
```powershell
# View entire log file
Get-Content proxy.log

# View last 20 entries
Get-Content proxy.log -Tail 20

# Search for specific domain
Select-String -Path proxy.log -Pattern "example.com"

# Count blocked vs allowed
(Select-String -Path proxy.log -Pattern "Action: ALLOWED").Count
(Select-String -Path proxy.log -Pattern "Action: BLOCKED").Count
```

### Clearing Logs
```powershell
# Clear log file (start fresh)
Remove-Item proxy.log

# The server will create a new file automatically
```

### Archiving Logs
```powershell
# Archive logs by date
$date = Get-Date -Format "yyyy-MM-dd"
Move-Item proxy.log "proxy_$date.log"
```

## Performance Considerations

- **Append Mode**: Fast file writes, no need to read existing content
- **Buffered I/O**: Operating system buffers reduce disk I/O overhead
- **Thread Safety**: Minimal lock contention due to fast write operations
- **File Size**: Grows continuously; implement log rotation for long-running deployments

## Compliance Notes

This logging implementation meets the project requirements for:
- ✓ Timestamp recording
- ✓ Client identification (IP:Port)
- ✓ Destination tracking (Host:Port)
- ✓ Request line logging
- ✓ Action tracking (Allow/Block)
- ✓ Response metrics (Status code, bytes transferred)
- ✓ Physical file output (not just console display)
