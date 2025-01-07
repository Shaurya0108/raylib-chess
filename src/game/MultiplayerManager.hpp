#pragma once

#include "raylib.h"
#include "../network/NetworkManager.hpp"
#include <memory>
#include <string>

class MultiplayerManager {
public:
    MultiplayerManager(bool isServer);
    ~MultiplayerManager() = default;

    bool initialize();
    void update();
    void draw();

    bool isConnected() const;
    void sendMessage(const std::string& message);
    std::vector<std::string> getMessages() const;

private:
    void handleInput();
    void drawChat();
    void drawConnectionStatus();

    std::unique_ptr<Network::NetworkManager> network_;
    bool isServer_;
    
    // Chat UI elements
    Rectangle chatBox_;
    std::string inputText_;
    bool isInputActive_;

    static constexpr int CHAT_MAX_MESSAGES = 10;
    static constexpr float CHAT_BOX_HEIGHT = 30.0f;
    static constexpr float CHAT_BOX_PADDING = 5.0f;
};
