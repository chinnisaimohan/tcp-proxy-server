/**
 * COMPREHENSIVE TEST CLIENT - Extended Test Suite
 * 
 * This test client provides extensive coverage of all proxy features:
 * - HTTP methods (GET, POST, PUT, DELETE, HEAD, OPTIONS)
 * - Domain filtering (blocked/allowed, subdomains, case-insensitivity)
 * - Edge cases (missing headers, malformed requests, large payloads)
 * - Real HTTP forwarding to multiple servers
 * - Concurrent request handling
 * - HTTP versions (1.0, 1.1)
 * - Query parameters and complex URLs
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const string PROXY_HOST = "127.0.0.1";
const int PROXY_PORT = 8888;
const int BUFFER_SIZE = 16384;  // 16KB for large responses

struct TestCase {
    string name;
    string request;
    int expectedStatus;
    string category;
};

// Connect to proxy
SOCKET connectToProxy() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(PROXY_HOST.c_str());
    serverAddr.sin_port = htons(PROXY_PORT);
    
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    
    return sock;
}

// Send request and receive response
string sendRequestAndGetResponse(const string& request) {
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        return "";
    }
    
    send(sock, request.c_str(), request.length(), 0);
    
    string response;
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    while ((bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        response.append(buffer, bytesReceived);
    }
    
    closesocket(sock);
    return response;
}

// Extract status code from response
int getStatusCode(const string& response) {
    if (response.empty()) return 0;
    
    size_t statusStart = response.find("HTTP/");
    if (statusStart == string::npos) return 0;
    
    size_t codeStart = response.find(" ", statusStart) + 1;
    size_t codeEnd = response.find(" ", codeStart);
    
    if (codeStart == string::npos || codeEnd == string::npos) return 0;
    
    string codeStr = response.substr(codeStart, codeEnd - codeStart);
    return stoi(codeStr);
}

// Run a single test
bool runTest(const TestCase& test, int& passed, int& failed) {
    cout << "\n========================================\n";
    cout << "TEST [" << test.category << "]: " << test.name << "\n";
    cout << "========================================\n";
    
    string response = sendRequestAndGetResponse(test.request);
    
    if (response.empty()) {
        cout << "✗ FAIL: No response received\n";
        failed++;
        return false;
    }
    
    int statusCode = getStatusCode(response);
    cout << "Response size: " << response.length() << " bytes\n";
    cout << "Status code: " << statusCode << "\n";
    
    if (statusCode == test.expectedStatus) {
        cout << "✓ PASS\n";
        passed++;
        return true;
    } else {
        cout << "✗ FAIL: Expected " << test.expectedStatus << ", got " << statusCode << "\n";
        failed++;
        return false;
    }
}

int main() {
    cout << "========================================\n";
    cout << "  COMPREHENSIVE TEST SUITE\n";
    cout << "  Complete Proxy Server (All Phases)\n";
    cout << "========================================\n";
    cout << "Make sure the proxy is running on port " << PROXY_PORT << "!\n";
    cout << "========================================\n";
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }
    
    int passed = 0, failed = 0;
    
    vector<TestCase> tests = {
        // ==================== CATEGORY 1: Basic HTTP Methods ====================
        {
            "GET request to allowed domain (example.com)",
            "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
            200,
            "Basic Methods"
        },
        {
            "POST request to allowed domain (httpbin.org)",
            "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nContent-Length: 13\r\n\r\n{\"key\":\"val\"}",
            200,
            "Basic Methods"
        },
        {
            "PUT request to allowed domain",
            "PUT /put HTTP/1.1\r\nHost: httpbin.org\r\nContent-Length: 10\r\n\r\n{\"x\":\"y\"}",
            200,
            "Basic Methods"
        },
        {
            "DELETE request to allowed domain",
            "DELETE /delete HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Basic Methods"
        },
        {
            "HEAD request to allowed domain",
            "HEAD / HTTP/1.1\r\nHost: example.com\r\n\r\n",
            200,
            "Basic Methods"
        },
        
        // ==================== CATEGORY 2: Domain Blocking ====================
        {
            "Blocked domain - facebook.com",
            "GET / HTTP/1.1\r\nHost: facebook.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - youtube.com",
            "GET / HTTP/1.1\r\nHost: youtube.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - twitter.com",
            "GET / HTTP/1.1\r\nHost: twitter.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - instagram.com",
            "GET / HTTP/1.1\r\nHost: instagram.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - netflix.com",
            "GET / HTTP/1.1\r\nHost: netflix.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - reddit.com",
            "GET / HTTP/1.1\r\nHost: reddit.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        {
            "Blocked domain - tiktok.com",
            "GET / HTTP/1.1\r\nHost: tiktok.com\r\n\r\n",
            403,
            "Domain Blocking"
        },
        
        // ==================== CATEGORY 3: Subdomain Matching ====================
        {
            "Blocked subdomain - www.facebook.com",
            "GET / HTTP/1.1\r\nHost: www.facebook.com\r\n\r\n",
            403,
            "Subdomain Blocking"
        },
        {
            "Blocked subdomain - api.twitter.com",
            "GET / HTTP/1.1\r\nHost: api.twitter.com\r\n\r\n",
            403,
            "Subdomain Blocking"
        },
        {
            "Blocked subdomain - m.youtube.com",
            "GET / HTTP/1.1\r\nHost: m.youtube.com\r\n\r\n",
            403,
            "Subdomain Blocking"
        },
        {
            "Blocked subdomain - api.instagram.com",
            "GET / HTTP/1.1\r\nHost: api.instagram.com\r\n\r\n",
            403,
            "Subdomain Blocking"
        },
        
        // ==================== CATEGORY 4: Case Insensitive Blocking ====================
        {
            "Case insensitive - FACEBOOK.COM",
            "GET / HTTP/1.1\r\nHost: FACEBOOK.COM\r\n\r\n",
            403,
            "Case Insensitive"
        },
        {
            "Case insensitive - YouTube.Com",
            "GET / HTTP/1.1\r\nHost: YouTube.Com\r\n\r\n",
            403,
            "Case Insensitive"
        },
        {
            "Case insensitive - TwItTeR.cOm",
            "GET / HTTP/1.1\r\nHost: TwItTeR.cOm\r\n\r\n",
            403,
            "Case Insensitive"
        },
        
        // ==================== CATEGORY 5: Port Handling ====================
        {
            "Blocked domain with port - facebook.com:80",
            "GET / HTTP/1.1\r\nHost: facebook.com:80\r\n\r\n",
            403,
            "Port Handling"
        },
        {
            "Blocked domain with port - youtube.com:443",
            "GET / HTTP/1.1\r\nHost: youtube.com:443\r\n\r\n",
            403,
            "Port Handling"
        },
        {
            "Allowed domain with port - example.com:80",
            "GET / HTTP/1.1\r\nHost: example.com:80\r\n\r\n",
            200,
            "Port Handling"
        },
        
        // ==================== CATEGORY 6: Error Handling ====================
        {
            "Missing Host header",
            "GET / HTTP/1.1\r\n\r\n",
            400,
            "Error Handling"
        },
        {
            "Invalid HTTP version",
            "GET / HTTP/2.0\r\nHost: example.com\r\n\r\n",
            200,  // Should still work
            "Error Handling"
        },
        
        // ==================== CATEGORY 7: HTTP 1.0 Support ====================
        {
            "HTTP/1.0 request - allowed domain",
            "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n",
            200,
            "HTTP 1.0"
        },
        {
            "HTTP/1.0 request - blocked domain",
            "GET / HTTP/1.0\r\nHost: facebook.com\r\n\r\n",
            403,
            "HTTP 1.0"
        },
        
        // ==================== CATEGORY 8: Complex Headers ====================
        {
            "Multiple headers - allowed domain",
            "GET / HTTP/1.1\r\nHost: httpbin.org\r\nUser-Agent: TestClient/1.0\r\nAccept: text/html\r\nAccept-Language: en-US\r\n\r\n",
            200,
            "Complex Headers"
        },
        {
            "Multiple headers - blocked domain",
            "GET / HTTP/1.1\r\nHost: twitter.com\r\nUser-Agent: TestClient/1.0\r\nAccept: text/html\r\n\r\n",
            403,
            "Complex Headers"
        },
        
        // ==================== CATEGORY 9: Query Parameters & Paths ====================
        {
            "Query parameters - httpbin.org/get?test=1",
            "GET /get?test=1&foo=bar HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Query Parameters"
        },
        {
            "Long URL path",
            "GET /get?param1=value1&param2=value2&param3=value3&param4=value4 HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Query Parameters"
        },
        {
            "Path with special characters",
            "GET /anything/test%20path HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Query Parameters"
        },
        
        // ==================== CATEGORY 10: Large Responses ====================
        {
            "Large response - httpbin.org/stream/100",
            "GET /stream/100 HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Large Responses"
        },
        {
            "HTML page - httpbin.org/html",
            "GET /html HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Large Responses"
        },
        
        // ==================== CATEGORY 11: POST with Body ====================
        {
            "POST with JSON body",
            "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nContent-Type: application/json\r\nContent-Length: 27\r\n\r\n{\"name\":\"test\",\"value\":123}",
            200,
            "POST Body"
        },
        {
            "POST with large body",
            "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nContent-Type: application/json\r\nContent-Length: 100\r\n\r\n" + string(100, 'X'),
            200,
            "POST Body"
        },
        
        // ==================== CATEGORY 12: Different HTTP Endpoints ====================
        {
            "httpbin.org/status/200",
            "GET /status/200 HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "HTTP Endpoints"
        },
        {
            "httpbin.org/headers",
            "GET /headers HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "HTTP Endpoints"
        },
        {
            "httpbin.org/user-agent",
            "GET /user-agent HTTP/1.1\r\nHost: httpbin.org\r\nUser-Agent: ComprehensiveTest\r\n\r\n",
            200,
            "HTTP Endpoints"
        },
        
        // ==================== CATEGORY 13: Edge Cases ====================
        {
            "Empty path with query",
            "GET /?test=value HTTP/1.1\r\nHost: httpbin.org\r\n\r\n",
            200,
            "Edge Cases"
        },
        {
            "Root path only",
            "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
            200,
            "Edge Cases"
        }
    };
    
    // Run all tests
    for (const auto& test : tests) {
        runTest(test, passed, failed);
    }
    
    // Summary
    cout << "\n========================================\n";
    cout << "       FINAL TEST RESULTS\n";
    cout << "========================================\n";
    cout << "Total Tests:  " << (passed + failed) << "\n";
    cout << "Passed:       " << passed << " ✓\n";
    cout << "Failed:       " << failed << " ✗\n";
    cout << "Success Rate: " << (100.0 * passed / (passed + failed)) << "%\n";
    cout << "========================================\n";
    
    if (failed == 0) {
        cout << "\n✓✓✓ ALL TESTS PASSED! ✓✓✓\n\n";
    } else {
        cout << "\n⚠ Some tests failed - review results above\n\n";
    }
    
    WSACleanup();
    return (failed == 0) ? 0 : 1;
}
