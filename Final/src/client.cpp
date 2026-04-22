#include "engine.h"
#include <iostream>

MarketClient::MarketClient(unsigned short listenPort) {
    int width = 500;
    int height = 200;
    
    numOptions = width * height;
    port = listenPort;

    hostStockPrices.resize(numOptions, 100.0f);
    hostStrikePrices.resize(numOptions);
    hostTimeToMaturity.resize(numOptions);
    hostRiskFreeRates.resize(numOptions, 0.05f); 
    hostVolatilities.resize(numOptions, 0.20f);  
    hostPrices.resize(numOptions, 0.0f);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            hostStrikePrices[idx] = 50.0f + ((float)x / (width - 1)) * 100.0f;
            hostTimeToMaturity[idx] = 0.1f + ((float)y / (height - 1)) * 1.9f;
        }
    }

    socket.bind(port); 
    socket.setBlocking(false); 
}

MarketClient::~MarketClient() {
    socket.unbind();
}

bool MarketClient::pollLiveMarketData() {
    float newStockPrice;
    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;

    socket.receive(&newStockPrice, sizeof(float), received, sender, senderPort);
    if(received == sizeof(float)) {
        std::cout << "Updated Stock Price: $" << newStockPrice << " from " << sender << std::endl;
        
        int width = 500;
        int height = 200;
        
        float minStrike = newStockPrice * 0.80f; 
        float maxStrike = newStockPrice * 1.20f;
        float strikeRange = maxStrike - minStrike;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                hostStockPrices[idx] = newStockPrice;
                hostStrikePrices[idx] = minStrike + ((float)x / (width - 1)) * strikeRange;
            }
        }
        return true;
    }

    return false;
}