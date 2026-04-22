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
    struct MarketPacket {
        float price;
        float riskFreeRate;
        float volatility;
    } packet;

    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;

    socket.receive(&packet, sizeof(MarketPacket), received, sender, senderPort);
    
    if(received == sizeof(MarketPacket)) {
        std::cout << "\nData received from " << sender << " | Price: $" << packet.price << " | Rate: " << packet.riskFreeRate << " | Vol: " << packet.volatility << std::endl;
        
        int width = 500;
        int height = 200;
        
        float minStrike = packet.price * 0.80f; 
        float maxStrike = packet.price * 1.20f;
        float strikeRange = maxStrike - minStrike;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                
                hostStockPrices[idx] = packet.price;
                hostStrikePrices[idx] = minStrike + ((float)x / (width - 1)) * strikeRange;
                
                hostRiskFreeRates[idx] = packet.riskFreeRate;
                hostVolatilities[idx] = packet.volatility;
            }
        }
        return true;
    }

    return false;
}