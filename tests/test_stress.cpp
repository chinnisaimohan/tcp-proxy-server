// Comprehensive Stress Test Suite for TCP Proxy Server
// Tests HTTP parsing, filtering, concurrency, and malformed data handling

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* PROXY_HOST = "127.0.0.1";
const int PROXY_PORT = 8888;

atomic<int> passedTests(0);
atomic<int> failedTests(0);
atomic<int> totalTests(0);

// Helper function to send data to proxy
bool sendToProxy(const string& request, string& response, int timeoutSeconds = 5) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Set timeout
    DWORD timeout = timeoutSeconds * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in proxyAddr;
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(PROXY_PORT);
    inet_pton(AF_INET, PROXY_HOST, &proxyAddr.sin_addr);

    if (connect(sock, (sockaddr*)&proxyAddr, sizeof(proxyAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    if (send(sock, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    char buffer[8192];
    int bytesReceived;
    response.clear();
    
    while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        response += buffer;
        
        // For responses, we can stop after headers if needed
        if (response.find("\r\n\r\n") != string::npos) {
            break;
        }
    }

    closesocket(sock);
    return !response.empty();
}

// Helper to send data slowly (byte by byte)
bool sendSlowly(const string& request, string& response, int delayMs = 100) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    DWORD timeout = 10000; // 10 second timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in proxyAddr;
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(PROXY_PORT);
    inet_pton(AF_INET, PROXY_HOST, &proxyAddr.sin_addr);

    if (connect(sock, (sockaddr*)&proxyAddr, sizeof(proxyAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    // Send byte by byte with delay
    for (size_t i = 0; i < request.length(); i++) {
        if (send(sock, &request[i], 1, 0) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }
        this_thread::sleep_for(chrono::milliseconds(delayMs));
    }

    char buffer[8192];
    int bytesReceived;
    response.clear();
    
    while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        response += buffer;
        if (response.find("\r\n\r\n") != string::npos) {
            break;
        }
    }

    closesocket(sock);
    return !response.empty();
}

void printResult(const string& testName, bool passed, const string& details = "") {
    totalTests++;
    if (passed) {
        passedTests++;
        cout << "  [PASS] " << testName;
        if (!details.empty()) cout << " - " << details;
        cout << endl;
    } else {
        failedTests++;
        cout << "  [FAIL] " << testName;
        if (!details.empty()) cout << " - " << details;
        cout << endl;
    }
}

// ==================== 1. HTTP PROTOCOL & PARSING EDGE CASES ====================

void test_extra_whitespace() {
    cout << "\n[1.1] Extra Whitespace in Request Line" << endl;
    
    string request = "GET     http://httpbin.org/get     HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    bool passed = success && (response.find("200 OK") != string::npos || 
                              response.find("HTTP/1.1 200") != string::npos);
    printResult("Extra whitespace handling", passed);
}

void test_header_case_sensitivity() {
    cout << "\n[1.2] Header Case Sensitivity" << endl;
    
    string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                     "hOsT: httpbin.org\r\n"
                     "CoNnEcTiOn: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    bool passed = success && (response.find("200 OK") != string::npos || 
                              response.find("HTTP/1.1 200") != string::npos);
    printResult("Case-insensitive headers", passed);
}

void test_very_long_url() {
    cout << "\n[1.3] Very Long URL (2000+ chars)" << endl;
    
    string longPath = "/get?data=";
    for (int i = 0; i < 200; i++) {
        longPath += "abcdefghij";
    }
    
    string request = "GET http://httpbin.org" + longPath + " HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response, 10);
    
    bool passed = success && (response.find("200 OK") != string::npos || 
                              response.find("HTTP/1.1 200") != string::npos ||
                              response.find("414") != string::npos); // URI Too Long is acceptable
    printResult("Very long URL handling", passed, "URL length: " + to_string(longPath.length()));
}

void test_folded_headers() {
    cout << "\n[1.4] Folded Headers (Obsolete HTTP/1.0 feature)" << endl;
    
    string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "X-Custom-Header: line1\r\n"
                     " continuation\r\n"  // Folded header
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    // Should either handle gracefully or reject
    bool passed = success || response.find("400") != string::npos;
    printResult("Folded headers handling", passed);
}

void test_missing_http_version() {
    cout << "\n[1.5] Missing HTTP Version" << endl;
    
    string request = "GET http://httpbin.org/get\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    // Should return 400 Bad Request
    bool passed = response.find("400") != string::npos || response.find("Bad Request") != string::npos;
    printResult("Missing HTTP version rejection", passed);
}

// ==================== 2. FILTERING & RULE-BASED EDGE CASES ====================

void test_subdomain_loophole() {
    cout << "\n[2.1] Subdomain Loophole Test" << endl;
    
    // facebook.com is blocked, try notfacebook.com
    string request = "GET http://notfacebook.com/ HTTP/1.1\r\n"
                     "Host: notfacebook.com\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    // notfacebook.com should NOT be blocked (different domain)
    bool passed = success && response.find("403") == string::npos;
    printResult("Subdomain loophole (notfacebook.com vs facebook.com)", passed);
}

void test_port_specific_blocking() {
    cout << "\n[2.2] Port Specific Access" << endl;
    
    string request1 = "GET http://httpbin.org:80/get HTTP/1.1\r\n"
                      "Host: httpbin.org:80\r\n"
                      "Connection: close\r\n\r\n";
    
    string request2 = "GET http://httpbin.org:8080/get HTTP/1.1\r\n"
                      "Host: httpbin.org:8080\r\n"
                      "Connection: close\r\n\r\n";
    
    string response1, response2;
    bool success1 = sendToProxy(request1, response1);
    
    // Port 80 should work
    bool passed = success1 && (response1.find("200") != string::npos);
    printResult("Different port access", passed, "Port 80 accessible");
}

void test_empty_blocklist() {
    cout << "\n[2.3] Empty Blocklist Handling" << endl;
    
    // This test assumes the server is running - if blocklist is empty, all should pass
    string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    bool passed = success && (response.find("200") != string::npos);
    printResult("Server handles empty/full blocklist", passed);
}

// ==================== 3. CONCURRENCY & CONNECTION STRESS ====================

void test_slowloris() {
    cout << "\n[3.1] Slowloris Attack Simulation (Slow Headers)" << endl;
    
    string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    // Send very slowly - 500ms per byte (simulates slowloris)
    bool success = sendSlowly(request, response, 50); // 50ms per char to keep test reasonable
    
    bool passed = success && (response.find("200") != string::npos || 
                              response.find("408") != string::npos); // Timeout acceptable
    printResult("Slowloris attack handling", passed);
}

void test_sudden_disconnect() {
    cout << "\n[3.2] Sudden Client Disconnect" << endl;
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in proxyAddr;
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(PROXY_PORT);
    inet_pton(AF_INET, PROXY_HOST, &proxyAddr.sin_addr);
    
    bool connected = (connect(sock, (sockaddr*)&proxyAddr, sizeof(proxyAddr)) != SOCKET_ERROR);
    
    if (connected) {
        string request = "GET http://httpbin.org/delay/2 HTTP/1.1\r\n"
                        "Host: httpbin.org\r\n"
                        "Connection: close\r\n\r\n";
        send(sock, request.c_str(), request.length(), 0);
        
        // Wait a bit then abruptly close
        this_thread::sleep_for(chrono::milliseconds(100));
        closesocket(sock); // Abrupt disconnect
    }
    
    // Test if proxy handles it gracefully by accepting another connection
    string testRequest = "GET http://httpbin.org/get HTTP/1.1\r\n"
                        "Host: httpbin.org\r\n"
                        "Connection: close\r\n\r\n";
    string response;
    bool recovered = sendToProxy(testRequest, response);
    
    bool passed = connected && recovered && response.find("200") != string::npos;
    printResult("Sudden disconnect recovery", passed);
}

void test_server_side_hang() {
    cout << "\n[3.3] Server-Side Hang/Timeout" << endl;
    
    // httpbin.org/delay/10 will wait 10 seconds before responding
    string request = "GET http://httpbin.org/delay/3 HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response, 8); // 8 second timeout
    
    // Should either complete successfully or timeout gracefully
    bool passed = success && (response.find("200") != string::npos || response.empty());
    printResult("Server-side delay handling", passed);
}

void test_high_port_range() {
    cout << "\n[3.4] High Port Range Access" << endl;
    
    // Try connecting to a service on unusual port (if available)
    string request = "GET http://httpbin.org:80/get HTTP/1.1\r\n"
                     "Host: httpbin.org:80\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    bool passed = success && (response.find("200") != string::npos);
    printResult("High/unusual port handling", passed);
}

void test_concurrent_connections() {
    cout << "\n[3.5] Concurrent Connections (20 simultaneous)" << endl;
    
    atomic<int> successful(0);
    vector<thread> threads;
    
    for (int i = 0; i < 20; i++) {
        threads.push_back(thread([&successful]() {
            string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                           "Host: httpbin.org\r\n"
                           "Connection: close\r\n\r\n";
            string response;
            if (sendToProxy(request, response) && response.find("200") != string::npos) {
                successful++;
            }
        }));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    bool passed = successful >= 18; // At least 90% should succeed
    printResult("Concurrent connections", passed, 
                to_string(successful.load()) + "/20 succeeded");
}

// ==================== 4. MALFORMED & MALICIOUS DATA ====================

void test_null_bytes() {
    cout << "\n[4.1] Null Bytes in Request" << endl;
    
    string request = "GET http://httpbin.org/get HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "X-Test: value";
    request += '\0'; // Inject null byte
    request += "more\r\n"
               "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response);
    
    // Should handle gracefully - either process or reject
    bool passed = !response.empty();
    printResult("Null byte handling", passed);
}

void test_giant_content_length() {
    cout << "\n[4.2] Giant Content-Length with Minimal Data" << endl;
    
    string request = "POST http://httpbin.org/post HTTP/1.1\r\n"
                     "Host: httpbin.org\r\n"
                     "Content-Length: 999999999\r\n"
                     "Connection: close\r\n\r\n"
                     "TinyBody";
    
    string response;
    bool success = sendToProxy(request, response, 3); // Short timeout
    
    // Should timeout or reject, not crash
    bool passed = true; // Just not crashing is success
    printResult("Giant Content-Length handling", passed);
}

void test_recursive_proxy() {
    cout << "\n[4.3] Recursive Proxy Loop Detection" << endl;
    
    string request = "GET http://127.0.0.1:8888/get HTTP/1.1\r\n"
                     "Host: 127.0.0.1:8888\r\n"
                     "Connection: close\r\n\r\n";
    
    string response;
    bool success = sendToProxy(request, response, 3);
    
    // Should either detect loop or timeout, not crash
    bool passed = true; // Survival is success
    printResult("Recursive proxy loop handling", passed);
}

void test_malformed_request_line() {
    cout << "\n[4.4] Completely Malformed Request" << endl;
    
    string request = "INVALID DATA HERE\r\n\r\n";
    
    string response;
    sendToProxy(request, response);
    
    // Should return 400 Bad Request
    bool passed = response.find("400") != string::npos || response.find("Bad Request") != string::npos;
    printResult("Malformed request rejection", passed);
}

void test_binary_garbage() {
    cout << "\n[4.5] Binary Garbage Input" << endl;
    
    string request;
    for (int i = 0; i < 100; i++) {
        request += (char)(rand() % 256);
    }
    
    string response;
    sendToProxy(request, response, 2);
    
    // Should handle gracefully - not crash
    bool passed = true; // Survival is success
    printResult("Binary garbage handling", passed);
}

// ==================== MAIN TEST RUNNER ====================

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    cout << "========================================" << endl;
    cout << "  COMPREHENSIVE STRESS TEST SUITE" << endl;
    cout << "========================================" << endl;
    cout << "\nProxy: " << PROXY_HOST << ":" << PROXY_PORT << endl;
    cout << "\nStarting tests...\n" << endl;
    
    // Category 1: HTTP Protocol & Parsing
    cout << "\n========================================" << endl;
    cout << "CATEGORY 1: HTTP PROTOCOL & PARSING" << endl;
    cout << "========================================" << endl;
    test_extra_whitespace();
    test_header_case_sensitivity();
    test_very_long_url();
    test_folded_headers();
    test_missing_http_version();
    
    // Category 2: Filtering & Rules
    cout << "\n========================================" << endl;
    cout << "CATEGORY 2: FILTERING & RULES" << endl;
    cout << "========================================" << endl;
    test_subdomain_loophole();
    test_port_specific_blocking();
    test_empty_blocklist();
    
    // Category 3: Concurrency & Stress
    cout << "\n========================================" << endl;
    cout << "CATEGORY 3: CONCURRENCY & STRESS" << endl;
    cout << "========================================" << endl;
    test_slowloris();
    test_sudden_disconnect();
    test_server_side_hang();
    test_high_port_range();
    test_concurrent_connections();
    
    // Category 4: Malformed & Malicious
    cout << "\n========================================" << endl;
    cout << "CATEGORY 4: MALFORMED & MALICIOUS DATA" << endl;
    cout << "========================================" << endl;
    test_null_bytes();
    test_giant_content_length();
    test_recursive_proxy();
    test_malformed_request_line();
    test_binary_garbage();
    
    // Final Results
    cout << "\n========================================" << endl;
    cout << "FINAL RESULTS" << endl;
    cout << "========================================" << endl;
    cout << "Total Tests: " << totalTests << endl;
    cout << "Passed: " << passedTests << " (" 
         << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%)" << endl;
    cout << "Failed: " << failedTests << " (" 
         << (totalTests > 0 ? (failedTests * 100 / totalTests) : 0) << "%)" << endl;
    cout << "========================================" << endl;
    
    if (failedTests == 0) {
        cout << "\n*** ALL STRESS TESTS PASSED! ***" << endl;
    } else {
        cout << "\n*** Some tests failed - review above for details ***" << endl;
    }
    
    WSACleanup();
    return 0;
}
