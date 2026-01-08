# TCP Proxy Server - 3-Minute Demonstration Script
## WORD-BY-WORD SCRIPT

**Total Time: 3 minutes | Presenter: Read naturally, pause where indicated**

---

## 📹 Recorded Demo Video

**Full demonstration available at:** [Google Drive - Demo Video](https://drive.google.com/drive/folders/1HSS7PyGSW3Qts57dNitf3xQORCubFbMp?usp=sharing)

This script provides the complete word-by-word narration used in the demonstration video.

---

## **[0:00 - 0:25] INTRODUCTION (25 seconds)**

**[Look at camera/audience]**

"Good morning everyone. Today, I'm going to demonstrate a TCP Proxy Server that I built using C++ and the Windows Sockets API. This proxy server intercepts HTTP traffic, parses requests, and filters content based on domain blocking rules.

**[Pause 1 second]**

The key features of this system are: domain-based filtering, support for all standard HTTP methods including GET, POST, PUT, and DELETE, robust error handling for malformed requests, and the ability to handle multiple concurrent connections. 

**[Pause 1 second]**

Most importantly, this project has been thoroughly tested with eighty-eight automated test cases, all passing at one hundred percent success rate."

---

## **[0:25 - 1:00] START THE PROXY SERVER (35 seconds)**

**[Turn to screen, open terminal]**

"Let me start by launching the proxy server."

**[ACTION: Type in terminal]**
```powershell
cd "C:\Users\Admin\Desktop\ACM Project"
```

**[Press Enter, then type]**
```powershell
.\bin\proxy_server.exe
```

**[Press Enter, pause 2 seconds for server to start]**

"As you can see here, the server is now running and listening on port eight-eight-eight-eight. 

**[Point to terminal output]**

It has successfully loaded the blocked domains configuration file. Currently, we have sixteen domains blocked for testing purposes, including popular sites like Facebook, YouTube, Twitter, Instagram, Netflix, and also HTTP Bin dot org which we'll use for testing.

**[Pause 1 second]**

The server is now ready to accept connections and will log all requests in real-time."

---

## **[1:00 - 1:45] AUTOMATED TESTING DEMONSTRATION (45 seconds)**

**[Open second terminal window]**

"Before we test with a browser, let me demonstrate our automated testing framework."

**[ACTION: Type in second terminal]**
```powershell
.\scripts\test_suite.ps1
```

**[Press Enter]**

"This PowerShell script runs our basic test suite, which validates core HTTP functionality and domain filtering."

**[Wait 8-10 seconds as tests run, point to screen as results appear]**

"Watch as each test executes. You can see tests for GET requests, POST requests, PUT and DELETE methods, and domain blocking verification.

**[Pause as final results appear]**

Perfect! All sixteen tests have passed successfully. That's one hundred percent pass rate.

**[Turn slightly toward camera/audience]**

Now, this is just one of our five test suites. In total, we have eighty-eight comprehensive test cases covering everything from basic HTTP methods to stress testing with twenty simultaneous connections, edge cases like malformed requests and extremely long URLs, and complex scenarios like slow data transmission and sudden disconnections. All eighty-eight tests pass at one hundred percent."

---

## **[1:45 - 2:45] LIVE BROWSER DEMONSTRATION (60 seconds)**

**[Switch to browser window]**

"Now, let's see this in action with a real web browser. I've already configured this browser to route all traffic through our proxy server at one-twenty-seven-dot-zero-dot-zero-dot-one, port eight-eight-eight-eight.

**[ACTION: Click address bar, type slowly and clearly]**

### **Test 1: Allowed Domain**

"First, let's try accessing an allowed website. I'll navigate to H-T-T-P colon slash slash example dot com."

**[Press Enter, wait 2-3 seconds for page to load]**

"Excellent! The page loads successfully. The proxy received the request, parsed it, checked that example.com is not in the blocked list, and forwarded it to the actual server. 

**[Point to proxy terminal if visible]**

You can see the request logged in our proxy terminal.

### **Test 2: Blocked Domain #1**

**[ACTION: Click address bar, clear it]**

Now, let's test the blocking functionality. I'll try to access H-T-T-P colon slash slash facebook dot com."

**[Press Enter, wait 2 seconds]**

"And there we have it! Four-oh-three Forbidden. The proxy intercepted this request, identified that facebook.com is in our blocked domains list, and returned a forbidden response without forwarding the request to Facebook's servers.

### **Test 3: Blocked Domain #2**

**[ACTION: Click address bar, clear it]**

Let me try one more blocked domain. H-T-T-P colon slash slash httpbin dot org."

**[Press Enter, wait 2 seconds]**

"Again, four-oh-three Forbidden. The blocking is working perfectly.

### **Test 4: Show Server Logs**

**[Switch back to proxy server terminal]**

Now, look at the server logs. You can clearly see the traffic flow. Here's the request for example.com with the message 'Domain allowed', showing it was successfully proxied. 

**[Point to different log entries]**

And here are the Facebook and HTTP Bin requests, both showing 'Domain blocked' with the four-oh-three response code. The proxy is actively monitoring and controlling all HTTP traffic."

---

## **[2:45 - 3:00] CONCLUSION (15 seconds)**

**[Turn back to camera/audience]**

"So, to summarize what we've seen today: a fully functional TCP proxy server with eighty-eight out of eighty-eight tests passing, real-world browser integration working flawlessly, active domain blocking protecting against unwanted content, and production-ready code with comprehensive documentation.

**[Pause 1 second, smile]**

Thank you for your attention. The complete source code, documentation, and all test suites are available on GitHub. Any questions?"

**[End of presentation]**

---

## **QUICK SETUP CHECKLIST (Before Demo)**

**5 Minutes Before:**
- [ ] Restart proxy server: `.\bin\proxy_server.exe`
- [ ] Configure browser proxy: 127.0.0.1:8888
- [ ] Clear browser cache
- [ ] Test example.com loads
- [ ] Test facebook.com shows 403
- [ ] Have 2 terminal windows ready (1 for server, 1 for tests)
- [ ] Have browser window ready
- [ ] Have VS Code open with key files

**Browser Tabs to Prepare:**
- Tab 1: `http://example.com` (allowed)
- Tab 2: `http://facebook.com` (blocked)
- Tab 3: `http://httpbin.org` (blocked)

---

## **BACKUP TALKING POINTS (If Time Permits)**

**Technical Implementation:**
- "Uses Windows Sockets API (Winsock2) for network communication"
- "Parses HTTP/1.1 request line to extract domain from absolute URLs"
- "Implements case-insensitive string matching for domain filtering"
- "Handles edge cases: malformed requests, concurrent connections, timeouts"

**Testing Approach:**
- "test_suite.ps1: Core HTTP methods and filtering (16 tests)"
- "test_stress.exe: Concurrency with 20 simultaneous connections (18 tests)"
- "test_edge_cases.exe: Malformed data and edge cases (10 tests)"
- "test_comprehensive.exe: End-to-end scenarios (40 tests)"
- "test_post_bodies.ps1: POST request validation (4 tests)"

**Use Cases:**
- "Corporate network filtering"
- "Parental controls"
- "Security research and traffic analysis"
- "Educational tool for understanding HTTP proxies"

---

## **TROUBLESHOOTING (During Demo)**

**If proxy server crashes:**
- Restart: `.\bin\proxy_server.exe`
- Check port 8888 not in use: `netstat -ano | findstr :8888`

**If browser won't load:**
- Verify proxy settings: 127.0.0.1:8888
- Use `http://` explicitly (not https://)
- Clear browser cache

**If tests fail:**
- Ensure proxy is running first
- Check httpbin.org is accessible
- Run individual test: `.\build\test_suite.exe`

---

## **TIME ALLOCATION SUMMARY**

| Section | Duration | Purpose |
|---------|----------|---------|
| Introduction | 20s | Context and features |
| Start Server | 30s | Show initialization |
| Automated Tests | 40s | Prove reliability |
| Browser Demo | 60s | Real-world usage |
| Code Highlights | 20s | Technical depth |
| Conclusion | 10s | Summary |
| **TOTAL** | **180s (3min)** | |

---

**Good luck with your demonstration! 🚀**
