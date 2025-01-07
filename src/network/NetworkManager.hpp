#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

// Cross-platform socket definitions
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

namespace Network {

struct GameMessage {
    static constexpr size_t MAX_LENGTH = 256;
    char text[MAX_LENGTH];
};

class NetworkManager {
public:
    NetworkManager(bool isServer);
    ~NetworkManager();

    // Delete copy constructor and assignment operator
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    bool initialize(int port = 12345);
    void sendMessage(const std::string& text);
    bool isConnected() const { return isConnected_; }
    std::vector<std::string> getMessages();

private:
    bool initializeServer(int port);
    bool initializeClient(int port);
    void serverLoop();
    void clientLoop();
    void cleanup();

    SocketType socket_;
    bool isServer_;
    bool isConnected_;
    std::vector<std::string> messages_;
    std::mutex messagesMutex_;
    std::thread networkThread_;
    bool shouldRun_;

    static constexpr const char* DEFAULT_IP = "127.0.0.1";
};

} // namespace Network
