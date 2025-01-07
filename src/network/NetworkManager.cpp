#include "NetworkManager.hpp"
#include <iostream>
#include <cstring>

namespace Network {

NetworkManager::NetworkManager(bool isServer)
    : isServer_(isServer)
    , isConnected_(false)
    , socket_(INVALID_SOCKET)
    , shouldRun_(true) {
    
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
}

NetworkManager::~NetworkManager() {
    cleanup();
}

void NetworkManager::cleanup() {
    shouldRun_ = false;
    isConnected_ = false;

    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    if (networkThread_.joinable()) {
        networkThread_.join();
    }

    #ifdef _WIN32
        WSACleanup();
    #endif
}

bool NetworkManager::initialize(int port) {
    return isServer_ ? initializeServer(port) : initializeClient(port);
}

bool NetworkManager::initializeServer(int port) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Allow socket reuse
    int opt = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << 
            #ifdef _WIN32
                WSAGetLastError()
            #else
                errno
            #endif
            << std::endl;
        return false;
    }

    if (listen(socket_, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        return false;
    }

    networkThread_ = std::thread(&NetworkManager::serverLoop, this);
    return true;
}

bool NetworkManager::initializeClient(int port) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, DEFAULT_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }

    if (connect(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }

    isConnected_ = true;
    networkThread_ = std::thread(&NetworkManager::clientLoop, this);
    return true;
}

void NetworkManager::serverLoop() {
    SocketType clientSocket = accept(socket_, nullptr, nullptr);
    if (clientSocket != INVALID_SOCKET) {
        isConnected_ = true;
        char buffer[sizeof(GameMessage)];
        
        while (shouldRun_ && isConnected_) {
            ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(GameMessage), 0);
            if (bytesReceived <= 0) {
                isConnected_ = false;
                break;
            }

            GameMessage* message = (GameMessage*)buffer;
            {
                std::lock_guard<std::mutex> lock(messagesMutex_);
                messages_.push_back(std::string(message->text));
            }
        }
        closesocket(clientSocket);
    }
}

void NetworkManager::clientLoop() {
    char buffer[sizeof(GameMessage)];
    while (shouldRun_ && isConnected_) {
        ssize_t bytesReceived = recv(socket_, buffer, sizeof(GameMessage), 0);
        if (bytesReceived <= 0) {
            isConnected_ = false;
            break;
        }

        GameMessage* message = (GameMessage*)buffer;
        {
            std::lock_guard<std::mutex> lock(messagesMutex_);
            messages_.push_back(std::string(message->text));
        }
    }
}

void NetworkManager::sendMessage(const std::string& text) {
    if (!isConnected_) return;

    GameMessage message;
    #ifdef _WIN32
        strncpy_s(message.text, text.c_str(), sizeof(message.text) - 1);
    #else
        strncpy(message.text, text.c_str(), sizeof(message.text) - 1);
    #endif
    
    send(socket_, (char*)&message, sizeof(GameMessage), 0);
}

std::vector<std::string> NetworkManager::getMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex_);
    return messages_;
}

} // namespace Network
