/**
 * COMPLETE TCP PROXY SERVER - ALL PHASES (1-5) INTEGRATED
 * 
 * This is the final production-ready implementation combining all learning phases:
 * 
 * PHASE 1 - Socket Fundamentals:
 * ✓ Winsock 2.2 initialization (WSAStartup/WSACleanup)
 * ✓ TCP socket creation (SOCK_STREAM)
 * ✓ Error handling and resource management
 * 
 * PHASE 2 - Server Lifecycle:
 * ✓ bind() - Attach socket to port 8888
 * ✓ listen() - Queue incoming connections
 * ✓ accept() - Accept client connections
 * ✓ recv() - Receive data with accumulation loop
 * 
 * PHASE 3 - HTTP Protocol:
 * ✓ HTTP request parsing (method, URL, version)
 * ✓ Header extraction (Host, Content-Length, etc.)
 * ✓ Domain filtering with blocked_domains.txt
 * ✓ Response generation (200 OK, 400 Bad Request, 403 Forbidden)
 * 
 * PHASE 4 - Data Forwarding:
 * ✓ Secondary socket connection to target server
 * ✓ DNS resolution with getaddrinfo()
 * ✓ Request forwarding to real HTTP servers
 * ✓ Response streaming (8KB chunks)
 * ✓ POST body forwarding
 * 
 * PHASE 5 - Concurrency:
 * ✓ Multi-threaded request handling (C++ <thread>)
 * ✓ Thread-safe logging with std::mutex
 * ✓ Proper resource cleanup (closesocket(), thread detach)
 * ✓ Concurrent client support
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Configuration
const int PROXY_PORT = 8888;
const int BUFFER_SIZE = 8192;
const string BLOCKED_DOMAINS_FILE = "config/blocked_domains.txt";
const string LOG_FILE = "proxy.log";

// Global thread-safe components
mutex logMutex;
mutex fileMutex;
atomic<int> activeThreads(0);
atomic<int> totalRequests(0);

// HTTP Request structure
struct HTTPRequest {
    string method;
    string url;
    string httpVersion;
    string host;
    int port;
    string path;
    map<string, string> headers;
    string body;
    bool isValid;
};

// Thread-safe logging function
void logMessage(const string& message) {
    lock_guard<mutex> lock(logMutex);
    
    // Get current timestamp
    time_t now = time(nullptr);
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%a %b %d %H:%M:%S %Y", localtime(&now));
    
    // Get thread ID
    ostringstream threadId;
    threadId << this_thread::get_id();
    
    string logEntry = "[" + string(timeStr) + "] [Thread " + threadId.str() + "] " + message + "\n";
    
    // Console output
    cout << logEntry;
    
    // File output
    lock_guard<mutex> fileLock(fileMutex);
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open()) {
        logFile << logEntry;
        logFile.close();
    }
}

// Structured log entry for traffic logging (REQUIRED FOR DELIVERABLE)
void logTrafficEntry(const string& clientIP, int clientPort,
                     const string& targetHost, int targetPort,
                     const string& requestLine,
                     const string& action,
                     int statusCode = 0,
                     int bytesTransferred = 0) {
    lock_guard<mutex> lock(logMutex);
    
    // Get current timestamp
    time_t now = time(nullptr);
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Build structured log entry
    ostringstream logEntry;
    logEntry << "========================================\n";
    logEntry << "[TRAFFIC LOG]\n";
    logEntry << "Timestamp: " << timeStr << "\n";
    logEntry << "Client: " << clientIP << ":" << clientPort << "\n";
    logEntry << "Destination: " << targetHost << ":" << targetPort << "\n";
    logEntry << "Request: " << requestLine << "\n";
    logEntry << "Action: " << action << "\n";
    if (statusCode > 0) {
        logEntry << "Status Code: " << statusCode << "\n";
    }
    if (bytesTransferred > 0) {
        logEntry << "Bytes Transferred: " << bytesTransferred << "\n";
    }
    logEntry << "========================================\n\n";
    
    string logStr = logEntry.str();
    
    // Console output
    cout << logStr;
    
    // File output (CRITICAL: Write to log file)
    lock_guard<mutex> fileLock(fileMutex);
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open()) {
        logFile << logStr;
        logFile.close();
    }
}

// Load blocked domains from file
vector<string> loadBlockedDomains() {
    vector<string> blockedDomains;
    ifstream file(BLOCKED_DOMAINS_FILE);
    
    if (!file.is_open()) {
        logMessage("⚠ Warning: Could not open " + BLOCKED_DOMAINS_FILE);
        return blockedDomains;
    }
    
    string line;
    while (getline(file, line)) {
        // Remove whitespace and comments
        size_t commentPos = line.find('#');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (!line.empty()) {
            blockedDomains.push_back(line);
        }
    }
    
    file.close();
    logMessage("✓ Loaded " + to_string(blockedDomains.size()) + " blocked domains");
    return blockedDomains;
}

// Check if domain is blocked (with subdomain matching)
bool isDomainBlocked(const string& domain, const vector<string>& blockedDomains) {
    string lowerDomain = domain;
    transform(lowerDomain.begin(), lowerDomain.end(), lowerDomain.begin(), ::tolower);
    
    // Remove port if present
    size_t colonPos = lowerDomain.find(':');
    if (colonPos != string::npos) {
        lowerDomain = lowerDomain.substr(0, colonPos);
    }
    
    for (const string& blocked : blockedDomains) {
        string lowerBlocked = blocked;
        transform(lowerBlocked.begin(), lowerBlocked.end(), lowerBlocked.begin(), ::tolower);
        
        // Exact match
        if (lowerDomain == lowerBlocked) {
            return true;
        }
        
        // Subdomain match (e.g., www.example.com matches example.com)
        if (lowerDomain.length() > lowerBlocked.length()) {
            size_t pos = lowerDomain.length() - lowerBlocked.length();
            if (lowerDomain.substr(pos) == lowerBlocked && 
                lowerDomain[pos - 1] == '.') {
                return true;
            }
        }
    }
    
    return false;
}

// Parse HTTP request line (GET /path HTTP/1.1)
bool parseRequestLine(const string& requestLine, HTTPRequest& request) {
    istringstream iss(requestLine);
    
    if (!(iss >> request.method >> request.url >> request.httpVersion)) {
        return false;
    }
    
    // Extract path from URL
    request.path = request.url;
    
    return true;
}

// Parse HTTP headers
void parseHeaders(const string& requestData, HTTPRequest& request) {
    istringstream stream(requestData);
    string line;
    
    // Skip request line
    getline(stream, line);
    
    // Parse headers
    while (getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);
            
            // Trim whitespace from value
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            // Convert header name to lowercase for case-insensitive comparison
            string lowerKey = key;
            transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
            
            request.headers[lowerKey] = value;
        }
    }
    
    // Extract Host header (now lowercase)
    if (request.headers.count("host") > 0) {
        string hostHeader = request.headers["host"];
        size_t colonPos = hostHeader.find(':');
        
        if (colonPos != string::npos) {
            request.host = hostHeader.substr(0, colonPos);
            request.port = stoi(hostHeader.substr(colonPos + 1));
        } else {
            request.host = hostHeader;
            request.port = 80;  // Default HTTP port
        }
    }
}

// Parse complete HTTP request
HTTPRequest parseHTTPRequest(const string& requestData) {
    HTTPRequest request;
    request.isValid = false;
    request.port = 80;
    
    if (requestData.empty()) {
        return request;
    }
    
    // Find first line
    size_t lineEnd = requestData.find("\r\n");
    if (lineEnd == string::npos) {
        lineEnd = requestData.find("\n");
    }
    
    if (lineEnd == string::npos) {
        return request;
    }
    
    string requestLine = requestData.substr(0, lineEnd);
    
    if (!parseRequestLine(requestLine, request)) {
        return request;
    }
    
    parseHeaders(requestData, request);
    
    // Extract body for POST/PUT requests
    size_t bodyStart = requestData.find("\r\n\r\n");
    if (bodyStart != string::npos) {
        request.body = requestData.substr(bodyStart + 4);
    }
    
    request.isValid = true;
    return request;
}

// Send HTTP response
void sendResponse(SOCKET clientSocket, int statusCode, const string& statusText, const string& body) {
    ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
}

// Connect to target server
SOCKET connectToServer(const string& host, int port) {
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    string portStr = to_string(port);
    
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
        logMessage("✗ DNS resolution failed for " + host);
        return INVALID_SOCKET;
    }
    
    SOCKET serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }
    
    if (connect(serverSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        closesocket(serverSocket);
        freeaddrinfo(result);
        logMessage("✗ Connection failed to " + host + ":" + to_string(port));
        return INVALID_SOCKET;
    }
    
    freeaddrinfo(result);
    logMessage("✓ Connected to " + host + ":" + to_string(port));
    return serverSocket;
}

// Forward request and stream response back
struct ForwardResult {
    bool success;
    int statusCode;
    int bytesTransferred;
};

ForwardResult forwardRequestAndStreamResponse(SOCKET clientSocket, const HTTPRequest& request) {
    ForwardResult result = {false, 502, 0};
    
    // Connect to target server
    SOCKET serverSocket = connectToServer(request.host, request.port);
    if (serverSocket == INVALID_SOCKET) {
        sendResponse(clientSocket, 502, "Bad Gateway",
            "<html><head><title>502 Bad Gateway</title></head>"
            "<body><h1>502 Bad Gateway</h1>"
            "<p>Could not connect to " + request.host + "</p></body></html>");
        return result;
    }
    
    // Reconstruct and forward the request
    ostringstream forwardRequest;
    forwardRequest << request.method << " " << request.path << " " << request.httpVersion << "\r\n";
    
    for (const auto& header : request.headers) {
        forwardRequest << header.first << ": " << header.second << "\r\n";
    }
    forwardRequest << "\r\n";
    
    string requestStr = forwardRequest.str();
    int sent = send(serverSocket, requestStr.c_str(), requestStr.length(), 0);
    
    if (sent == SOCKET_ERROR) {
        closesocket(serverSocket);
        sendResponse(clientSocket, 502, "Bad Gateway",
            "<html><head><title>502 Bad Gateway</title></head>"
            "<body><h1>502 Bad Gateway</h1>"
            "<p>Failed to send request to " + request.host + "</p></body></html>");
        return result;
    }
    
    logMessage("✓ Forwarded " + to_string(sent) + " bytes to " + request.host);
    
    // Forward POST/PUT body if present
    if (!request.body.empty()) {
        send(serverSocket, request.body.c_str(), request.body.length(), 0);
        logMessage("✓ Forwarded " + to_string(request.body.length()) + " bytes of body data");
    }
    
    // Stream response back to client in chunks
    char buffer[BUFFER_SIZE];
    int totalReceived = 0;
    int bytesReceived;
    int extractedStatusCode = 200; // Default to 200 OK
    
    bool firstChunk = true;
    while ((bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        // Extract status code from first chunk
        if (firstChunk && bytesReceived > 12) {
            string responseStart(buffer, min(bytesReceived, 100));
            if (responseStart.find("HTTP/1.") == 0) {
                size_t codePos = responseStart.find(' ');
                if (codePos != string::npos && codePos + 4 < responseStart.length()) {
                    try {
                        extractedStatusCode = stoi(responseStart.substr(codePos + 1, 3));
                    } catch (...) {
                        extractedStatusCode = 200;
                    }
                }
            }
            firstChunk = false;
        }
        
        send(clientSocket, buffer, bytesReceived, 0);
        totalReceived += bytesReceived;
    }
    
    closesocket(serverSocket);
    logMessage("✓ Streamed " + to_string(totalReceived) + " bytes from " + request.host + " to client");
    
    result.success = true;
    result.statusCode = extractedStatusCode;
    result.bytesTransferred = totalReceived;
    return result;
}

// Handle individual client connection (runs in separate thread)
void handleClient(SOCKET clientSocket, const vector<string>& blockedDomains) {
    activeThreads++;
    int requestId = ++totalRequests;
    
    // Get client address information
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen);
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);
    
    logMessage("=== New client connection [Request #" + to_string(requestId) + "] ===");
    
    // Receive request data with accumulation
    string requestData;
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    // First, read until we have complete headers
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        requestData.append(buffer, bytesReceived);
        
        // Check if we have complete headers
        if (requestData.find("\r\n\r\n") != string::npos) {
            break;
        }
    }
    
    if (requestData.empty()) {
        logMessage("✗ No data received from client");
        closesocket(clientSocket);
        activeThreads--;
        return;
    }
    
    // Check if there's a Content-Length header for POST/PUT requests
    size_t headerEnd = requestData.find("\r\n\r\n");
    if (headerEnd != string::npos) {
        string headers = requestData.substr(0, headerEnd);
        
        // Convert headers to lowercase for case-insensitive search
        string lowerHeaders = headers;
        transform(lowerHeaders.begin(), lowerHeaders.end(), lowerHeaders.begin(), ::tolower);
        size_t clPos = lowerHeaders.find("content-length:");
        
        if (clPos != string::npos) {
            // Extract Content-Length value (from original headers to preserve case)
            size_t clStart = clPos + 15; // Length of "content-length:"
            size_t clEnd = headers.find("\r\n", clStart);
            string clValue = headers.substr(clStart, clEnd - clStart);
            
            // Trim whitespace
            clValue.erase(0, clValue.find_first_not_of(" \t"));
            clValue.erase(clValue.find_last_not_of(" \t\r\n") + 1);
            
            int contentLength = stoi(clValue);
            int currentBodyLength = requestData.length() - (headerEnd + 4);
            
            // Continue reading until we have all the body
            while (currentBodyLength < contentLength) {
                bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesReceived <= 0) break;
                
                requestData.append(buffer, bytesReceived);
                currentBodyLength += bytesReceived;
            }
            
            logMessage("✓ Received complete POST/PUT body (" + to_string(contentLength) + " bytes)");
        }
    }
    
    logMessage("✓ Received " + to_string(requestData.length()) + " bytes from client");
    
    // Parse HTTP request
    HTTPRequest request = parseHTTPRequest(requestData);
    
    if (!request.isValid) {
        logMessage("✗ Invalid HTTP request");
        sendResponse(clientSocket, 400, "Bad Request",
            "<html><head><title>400 Bad Request</title></head>"
            "<body><h1>400 Bad Request</h1>"
            "<p>The request could not be parsed.</p></body></html>");
        closesocket(clientSocket);
        activeThreads--;
        return;
    }
    
    logMessage("✓ Parsed: " + request.method + " " + request.url + " " + request.httpVersion);
    
    // Check for Host header
    if (request.host.empty()) {
        logMessage("✗ Missing Host header");
        sendResponse(clientSocket, 400, "Bad Request",
            "<html><head><title>400 Bad Request</title></head>"
            "<body><h1>400 Bad Request</h1>"
            "<p>Missing Host header.</p></body></html>");
        closesocket(clientSocket);
        activeThreads--;
        return;
    }
    
    // Check for recursive proxy loop (prevent connecting to ourselves)
    if ((request.host == "127.0.0.1" || request.host == "localhost") && request.port == PROXY_PORT) {
        logMessage("✗ Recursive proxy loop detected: " + request.host + ":" + to_string(request.port));
        sendResponse(clientSocket, 508, "Loop Detected",
            "<html><head><title>508 Loop Detected</title></head>"
            "<body><h1>508 Loop Detected</h1>"
            "<p>Recursive proxy loop prevented. Target " + request.host + ":" + to_string(request.port) + 
            " is this proxy server.</p></body></html>");
        closesocket(clientSocket);
        activeThreads--;
        return;
    }
    
    // Check if domain is blocked
    if (isDomainBlocked(request.host, blockedDomains)) {
        logMessage("✗ Domain BLOCKED: " + request.host);
        
        // LOG STRUCTURED ENTRY FOR BLOCKED REQUEST
        string requestLine = request.method + " " + request.url + " " + request.httpVersion;
        logTrafficEntry(clientIP, clientPort,
                       request.host, request.port,
                       requestLine,
                       "BLOCKED",
                       403, 0);
        
        sendResponse(clientSocket, 403, "Forbidden",
            "<html><head><title>403 Forbidden</title></head>"
            "<body><h1>403 Forbidden</h1>"
            "<p>Access to " + request.host + " is blocked by proxy policy.</p></body></html>");
        closesocket(clientSocket);
        activeThreads--;
        return;
    }
    
    logMessage("✓ Domain ALLOWED: " + request.host);
    
    // Forward request to target server
    ForwardResult result = forwardRequestAndStreamResponse(clientSocket, request);
    
    // LOG STRUCTURED ENTRY FOR ALLOWED REQUEST
    string requestLine = request.method + " " + request.url + " " + request.httpVersion;
    logTrafficEntry(clientIP, clientPort,
                   request.host, request.port,
                   requestLine,
                   "ALLOWED",
                   result.statusCode, result.bytesTransferred);
    
    // Cleanup
    closesocket(clientSocket);
    logMessage("✓ Client connection closed [Request #" + to_string(requestId) + "]");
    activeThreads--;
}

// Main server function
int main() {
    cout << "========================================\n";
    cout << "  Complete TCP Proxy Server (Phases 1-5)\n";
    cout << "========================================\n";
    cout << "Features:\n";
    cout << "  ✓ Multi-threaded concurrent handling\n";
    cout << "  ✓ HTTP parsing & domain filtering\n";
    cout << "  ✓ Real HTTP forwarding & streaming\n";
    cout << "  ✓ Thread-safe logging\n";
    cout << "========================================\n\n";
    
    // Initialize Winsock (Phase 1)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "✗ WSAStartup failed\n";
        return 1;
    }
    cout << "✓ Winsock initialized\n";
    
    // Load blocked domains (Phase 3)
    vector<string> blockedDomains = loadBlockedDomains();
    
    // Create listening socket (Phase 1)
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "✗ Socket creation failed\n";
        WSACleanup();
        return 1;
    }
    cout << "✓ Socket created\n";
    
    // Bind to port (Phase 2)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PROXY_PORT);
    
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "✗ Bind failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    cout << "✓ Bound to port " << PROXY_PORT << "\n";
    
    // Listen for connections (Phase 2)
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "✗ Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    cout << "✓ Listening for connections...\n";
    cout << "✓ Proxy server running on port " << PROXY_PORT << "\n\n";
    
    logMessage("========================================");
    logMessage("Proxy Server Started on Port " + to_string(PROXY_PORT));
    logMessage("========================================");
    
    // Accept connections (Phase 2 + Phase 5 threading)
    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        
        if (clientSocket == INVALID_SOCKET) {
            logMessage("✗ Accept failed");
            continue;
        }
        
        // Spawn new thread for each client (Phase 5)
        thread clientThread(handleClient, clientSocket, blockedDomains);
        clientThread.detach();  // Allow thread to run independently
        
        logMessage("✓ Spawned new thread (Active threads: " + to_string(activeThreads.load()) + ")");
    }
    
    // Cleanup (Phase 1)
    closesocket(listenSocket);
    WSACleanup();
    
    return 0;
}
