#include "MultiplayerManager.hpp"

MultiplayerManager::MultiplayerManager(bool isServer)
    : network_(std::make_unique<Network::NetworkManager>(isServer))
    , isServer_(isServer)
    , chatBox_({ 10.0f, static_cast<float>(GetScreenHeight() - 40), static_cast<float>(GetScreenWidth() - 20), CHAT_BOX_HEIGHT })
    , inputText_("")
    , isInputActive_(false) {
}

bool MultiplayerManager::initialize() {
    return network_->initialize();
}

void MultiplayerManager::handleInput() {
    // Handle chat box activation
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePoint = GetMousePosition();
        isInputActive_ = CheckCollisionPointRec(mousePoint, chatBox_);
    }

    if (isInputActive_) {
        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && 
                (inputText_.length() < Network::GameMessage::MAX_LENGTH - 1)) {
                inputText_ += (char)key;
            }
            key = GetCharPressed();
        }

        // Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE) && !inputText_.empty()) {
            inputText_.pop_back();
        }

        // Handle message send
        if (IsKeyPressed(KEY_ENTER) && !inputText_.empty()) {
            network_->sendMessage(inputText_);
            inputText_.clear();
        }
    }
}

void MultiplayerManager::update() {
    handleInput();

    // Update chat box position based on current screen height
    chatBox_.y = static_cast<float>(GetScreenHeight() - 40);
    chatBox_.width = static_cast<float>(GetScreenWidth() - 20);
}

void MultiplayerManager::drawChat() {
    // Draw chat box
    DrawRectangleRec(chatBox_, LIGHTGRAY);
    DrawRectangleLinesEx(chatBox_, 1, isInputActive_ ? RED : DARKGRAY);
    DrawText(inputText_.c_str(), 
             chatBox_.x + CHAT_BOX_PADDING, 
             chatBox_.y + CHAT_BOX_PADDING, 
             20, BLACK);

    // Draw messages
    auto messages = network_->getMessages();
    size_t startIdx = messages.size() > CHAT_MAX_MESSAGES ? 
                      messages.size() - CHAT_MAX_MESSAGES : 0;
    
    for (size_t i = startIdx; i < messages.size(); i++) {
        float y = chatBox_.y - ((messages.size() - i) * 25) - 10;
        DrawText(messages[i].c_str(), 10, y, 20, BLACK);
    }
}

void MultiplayerManager::drawConnectionStatus() {
    const char* statusText;
    const char* helpText;
    Color statusColor;

    if (isServer_) {
        if (isConnected()) {
            statusText = "Server: Client Connected";
            helpText = "";
            statusColor = GREEN;
        } else {
            statusText = "Server: Waiting for Client...";
            helpText = "Start a client in another window to connect";
            statusColor = YELLOW;
        }
    } else {
        if (isConnected()) {
            statusText = "Client: Connected to Server";
            helpText = "";
            statusColor = GREEN;
        } else {
            statusText = "Client: Cannot Connect to Server";
            helpText = "Make sure to start the server first (use --server)";
            statusColor = RED;
        }
    }

    DrawText(statusText, 10, 10, 20, statusColor);
    if (strlen(helpText) > 0) {
        DrawText(helpText, 10, 35, 16, GRAY);
    }
}

void MultiplayerManager::draw() {
    drawConnectionStatus();
    drawChat();
}

bool MultiplayerManager::isConnected() const {
    return network_->isConnected();
}

void MultiplayerManager::sendMessage(const std::string& message) {
    network_->sendMessage(message);
}

std::vector<std::string> MultiplayerManager::getMessages() const {
    return network_->getMessages();
}
