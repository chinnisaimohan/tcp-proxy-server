# Quick Reference Guide

## Project: Custom Network Proxy Server

---

## File Organization

### Source Code (`src/`)
| File | Description | Lines |
|------|-------------|-------|
| `proxy_server.cpp` | Main proxy server (Phases 1-5) | ~570 |
| `test_comprehensive.cpp` | Comprehensive test suite | ~800 |
| `test_edge_cases.cpp` | Edge case tests | ~500 |
| `test_stress.cpp` | Stress & malicious input tests | ~600 |

### Executables (`bin/`)
| File | Purpose |
|------|---------|
| `proxy_server.exe` | Main server (port 8888) |
| `test_comprehensive.exe` | Run 40 general tests |
| `test_edge_cases.exe` | Run 10 advanced tests |
| `test_stress.exe` | Run 18 stress tests |

### Test Scripts
| Script | Tests | Description |
|--------|-------|-------------|
| `test_suite.ps1` | 20 | Basic functionality, filtering, errors |
| `test_post_bodies.ps1` | 4 | Large POST body handling (1KB-100KB) |

### Configuration
| File | Purpose |
|------|---------|
| `config/blocked_domains.txt` | Domain blocklist (17 entries) |

### Documentation
| File | Content |
|------|---------|
| `README.md` | Professional academic documentation |
| `TEST_RESULTS.md` | Detailed test results (42/42 passing) |
| `CLEANUP_SUMMARY.md` | Project reorganization summary |

---

## Quick Commands

### Build Everything
```powershell
# Main server
g++ src\proxy_server.cpp -o bin\proxy_server.exe -lws2_32 -std=c++17 -Wall

# Test clients
g++ src\test_comprehensive.cpp -o bin\test_comprehensive.exe -lws2_32 -std=c++17 -Wall
g++ src\test_edge_cases.cpp -o bin\test_edge_cases.exe -lws2_32 -std=c++17 -Wall
g++ src\test_stress.cpp -o bin\test_stress.exe -lws2_32 -std=c++17 -Wall
```

### Run Server
```powershell
.\bin\proxy_server.exe
```

### Run Tests
```powershell
# All tests (42 total)
.\test_suite.ps1            # 20 tests
.\test_post_bodies.ps1      # 4 tests
.\bin\test_stress.exe       # 18 tests
```

### Configure Proxy
**Browser/System Settings:**
- HTTP Proxy: `127.0.0.1`
- Port: `8888`

**Command Line (cURL):**
```bash
curl -x http://127.0.0.1:8888 http://example.com
```

---

## Test Results Summary

| Category | Tests | Status |
|----------|-------|--------|
| **Basic Functionality** | 20/20 | ✓ 100% |
| **Large POST Bodies** | 4/4 | ✓ 100% |
| **Stress & Edge Cases** | 18/18 | ✓ 100% |
| **TOTAL** | **42/42** | **✓ 100%** |

---

## Key Features

✓ HTTP/1.0 and HTTP/1.1 support  
✓ Multi-threaded (thread-per-connection)  
✓ Domain filtering (17 blocked domains)  
✓ Case-insensitive header parsing  
✓ POST/PUT body handling (up to 100KB tested)  
✓ Thread-safe logging  
✓ Subdomain matching  
✓ Stress-tested (20+ concurrent connections)  

---

## Technology Stack

- **Language:** C++17
- **API:** Winsock 2.2
- **Concurrency:** `<thread>`, `<mutex>`, `<atomic>`
- **Compiler:** g++ (MinGW-w64)
- **Platform:** Windows 10/11

---

## Project Statistics

- **Source Files:** 4 (.cpp)
- **Executables:** 4 (.exe)
- **Test Scripts:** 2 (.ps1)
- **Total Tests:** 42 (100% passing)
- **Total Lines:** ~2500+ (source code)
- **Blocked Domains:** 17 (configurable)
- **Buffer Size:** 8192 bytes
- **Port:** 8888

---

## Directory Tree

```
ACM Project/
├── src/
│   ├── proxy_server.cpp
│   ├── test_comprehensive.cpp
│   ├── test_edge_cases.cpp
│   └── test_stress.cpp
├── bin/
│   ├── proxy_server.exe
│   ├── test_comprehensive.exe
│   ├── test_edge_cases.exe
│   └── test_stress.exe
├── config/
│   └── blocked_domains.txt
├── test_suite.ps1
├── test_post_bodies.ps1
├── README.md
├── TEST_RESULTS.md
└── CLEANUP_SUMMARY.md
```

---

*Last Updated: January 2, 2026*  
*Version: 1.0.0*  
*Status: Production Ready*
