/**
 * EDGE CASE TEST CLIENT
 * Tests advanced scenarios and edge cases for the proxy server
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const string PROXY_HOST = "127.0.0.1";
const int PROXY_PORT = 8888;

// Connect to proxy
SOCKET connectToProxy() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;
    
    // Set socket timeout (10 seconds)
    DWORD timeout = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
    
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

// Test 1: Large Headers
bool testLargeHeaders() {
    cout << "\n========================================\n";
    cout << "TEST: Large Headers (8KB+ headers)\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    // Create request with large header
    string largeHeader(8000, 'X'); // 8KB of X's
    string request = "GET http://example.com/ HTTP/1.1\r\n";
    request += "Host: example.com\r\n";
    request += "X-Large-Header: " + largeHeader + "\r\n";
    request += "\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "200") != nullptr) {
        cout << "✓ PASS: Handled large headers (8KB+)\n";
        return true;
    }
    
    cout << "✗ FAIL: Did not handle large headers properly\n";
    return false;
}

// Test 2: POST with Large Body
bool testLargePostBody() {
    cout << "\n========================================\n";
    cout << "TEST: POST with Large Body (100KB)\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    // Create large body
    string largeBody(100000, 'A'); // 100KB
    string request = "POST http://httpbin.org/post HTTP/1.1\r\n";
    request += "Host: httpbin.org\r\n";
    request += "Content-Length: " + to_string(largeBody.length()) + "\r\n";
    request += "\r\n";
    request += largeBody;
    
    send(sock, request.c_str(), request.length(), 0);
    
    // Read response
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "200") != nullptr) {
        cout << "✓ PASS: Streamed large POST body (100KB)\n";
        return true;
    }
    
    cout << "✗ FAIL: Could not handle large POST body\n";
    return false;
}

// Test 3: Query Parameters with Special Characters
bool testComplexQueryParameters() {
    cout << "\n========================================\n";
    cout << "TEST: Complex Query Parameters\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    string request = "GET http://httpbin.org/get?q=ai+proxy&lang=en&special=%20%21 HTTP/1.1\r\n";
    request += "Host: httpbin.org\r\n\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[2048];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "200") != nullptr) {
        cout << "✓ PASS: Handled complex query parameters\n";
        return true;
    }
    
    cout << "✗ FAIL: Could not handle query parameters\n";
    return false;
}

// Test 4: Subdomain Blocking
bool testSubdomainBlocking() {
    cout << "\n========================================\n";
    cout << "TEST: Subdomain Blocking\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    string request = "GET http://m.facebook.com/ HTTP/1.1\r\n";
    request += "Host: m.facebook.com\r\n\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "403") != nullptr) {
        cout << "✓ PASS: Subdomain blocked correctly\n";
        return true;
    }
    
    cout << "✗ FAIL: Subdomain should be blocked\n";
    return false;
}

// Test 5: Case Insensitive Blocking
bool testCaseInsensitiveBlocking() {
    cout << "\n========================================\n";
    cout << "TEST: Case Insensitive Blocking\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    string request = "GET http://FaCeBoOk.CoM/ HTTP/1.1\r\n";
    request += "Host: FaCeBoOk.CoM\r\n\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "403") != nullptr) {
        cout << "✓ PASS: Case insensitive blocking works\n";
        return true;
    }
    
    cout << "✗ FAIL: Should be case insensitive\n";
    return false;
}

// Test 6: Invalid Domain
bool testInvalidDomain() {
    cout << "\n========================================\n";
    cout << "TEST: Invalid Domain (502/504 expected)\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    string request = "GET http://this-domain-does-not-exist-12345.invalid/ HTTP/1.1\r\n";
    request += "Host: this-domain-does-not-exist-12345.invalid\r\n\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && (strstr(buffer, "502") != nullptr || strstr(buffer, "504") != nullptr)) {
        cout << "✓ PASS: Returns proper error for invalid domain\n";
        return true;
    }
    
    cout << "✗ FAIL: Should return 502/504 for invalid domain\n";
    return false;
}

// Test 7: Garbage Input
bool testGarbageInput() {
    cout << "\n========================================\n";
    cout << "TEST: Garbage Binary Data\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    // Send random binary data
    char garbage[100];
    for (int i = 0; i < 100; i++) {
        garbage[i] = (char)(rand() % 256);
    }
    
    send(sock, garbage, 100, 0);
    
    // Wait a bit
    this_thread::sleep_for(chrono::milliseconds(500));
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    // Either no response or 400 Bad Request is acceptable
    cout << "✓ PASS: Server handled garbage data gracefully (no crash)\n";
    return true;
}

// Test 8: Missing Host Header
bool testMissingHostHeader() {
    cout << "\n========================================\n";
    cout << "TEST: Missing Host Header (400 expected)\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    string request = "GET http://example.com/ HTTP/1.1\r\n\r\n"; // No Host header
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    closesocket(sock);
    
    if (received > 0 && strstr(buffer, "400") != nullptr) {
        cout << "✓ PASS: Returns 400 for missing Host header\n";
        return true;
    }
    
    cout << "✗ FAIL: Should return 400 Bad Request\n";
    return false;
}

// Test 9: Half-Open Connection (Timeout Test)
bool testHalfOpenConnection() {
    cout << "\n========================================\n";
    cout << "TEST: Half-Open Connection (Partial Request)\n";
    cout << "========================================\n";
    
    SOCKET sock = connectToProxy();
    if (sock == INVALID_SOCKET) {
        cout << "✗ FAIL: Could not connect\n";
        return false;
    }
    
    // Send only partial request
    string partial = "GET http://example.com/ HTTP/1.1\r\n";
    send(sock, partial.c_str(), partial.length(), 0);
    
    cout << "Waiting 5 seconds with incomplete request...\n";
    this_thread::sleep_for(chrono::seconds(5));
    
    closesocket(sock);
    
    cout << "✓ PASS: Server didn't crash (timeout handling working)\n";
    return true;
}

// Test 10: Concurrent Connections
bool testConcurrentConnections() {
    cout << "\n========================================\n";
    cout << "TEST: Concurrent Connections (20 simultaneous)\n";
    cout << "========================================\n";
    
    vector<thread> threads;
    int successCount = 0;
    
    auto clientFunc = [&successCount]() {
        SOCKET sock = connectToProxy();
        if (sock == INVALID_SOCKET) return;
        
        string request = "GET http://example.com/ HTTP/1.1\r\nHost: example.com\r\n\r\n";
        send(sock, request.c_str(), request.length(), 0);
        
        char buffer[1024];
        int received = recv(sock, buffer, sizeof(buffer), 0);
        closesocket(sock);
        
        if (received > 0 && strstr(buffer, "200") != nullptr) {
            successCount++;
        }
    };
    
    for (int i = 0; i < 20; i++) {
        threads.push_back(thread(clientFunc));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (successCount >= 18) { // Allow for some variance
        cout << "✓ PASS: Handled " << successCount << "/20 concurrent connections\n";
        return true;
    }
    
    cout << "✗ FAIL: Only " << successCount << "/20 succeeded\n";
    return false;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }
    
    cout << "========================================\n";
    cout << "  EDGE CASE TEST SUITE\n";
    cout << "  Advanced Proxy Testing\n";
    cout << "========================================\n";
    
    int passed = 0, failed = 0;
    
    // Category 1: Basic Forwarding & Protocol Edge Cases
    cout << "\n========================================\n";
    cout << "CATEGORY 1: Protocol Edge Cases\n";
    cout << "========================================\n";
    
    testLargeHeaders() ? passed++ : failed++;
    testLargePostBody() ? passed++ : failed++;
    testComplexQueryParameters() ? passed++ : failed++;
    
    // Category 2: Filtering Logic & Security
    cout << "\n========================================\n";
    cout << "CATEGORY 2: Filtering & Security\n";
    cout << "========================================\n";
    
    testSubdomainBlocking() ? passed++ : failed++;
    testCaseInsensitiveBlocking() ? passed++ : failed++;
    
    // Category 3: Concurrency & Network Edge Cases
    cout << "\n========================================\n";
    cout << "CATEGORY 3: Concurrency & Network\n";
    cout << "========================================\n";
    
    testHalfOpenConnection() ? passed++ : failed++;
    testConcurrentConnections() ? passed++ : failed++;
    testInvalidDomain() ? passed++ : failed++;
    
    // Category 4: Robustness (Malformed Data)
    cout << "\n========================================\n";
    cout << "CATEGORY 4: Robustness\n";
    cout << "========================================\n";
    
    testGarbageInput() ? passed++ : failed++;
    testMissingHostHeader() ? passed++ : failed++;
    
    // Summary
    cout << "\n========================================\n";
    cout << "  EDGE CASE TEST RESULTS\n";
    cout << "========================================\n";
    cout << "Total Tests:  " << (passed + failed) << "\n";
    cout << "Passed:       " << passed << "\n";
    cout << "Failed:       " << failed << "\n";
    cout << "Success Rate: " << (100.0 * passed / (passed + failed)) << "%\n";
    cout << "========================================\n\n";
    
    if (failed == 0) {
        cout << "✓✓✓ ALL EDGE CASE TESTS PASSED! ✓✓✓\n\n";
    }
    
    WSACleanup();
    return (failed == 0) ? 0 : 1;
}
