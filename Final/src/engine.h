#ifndef ENGINE_H
#define ENGINE_H
#include <cstdint>
#include <vector>
#include <SFML/Network.hpp>

class MarketClient {
public:
    uint32_t numOptions;

    std::vector<float> hostStockPrices;
    std::vector<float> hostStrikePrices;
    std::vector<float> hostTimeToMaturity;
    std::vector<float> hostRiskFreeRates;
    std::vector<float> hostVolatilities;
    std::vector<float> hostPrices;

    float* deviceStockPrices = nullptr;
    float* deviceStrikePrices = nullptr;
    float* deviceTimeToMaturity = nullptr;
    float* deviceRiskFreeRates = nullptr;
    float* deviceVolatilities = nullptr;
    float* devicePrices = nullptr;

    sf::UdpSocket socket;
    unsigned short port;

    MarketClient(unsigned short listenPort);
    ~MarketClient();

    bool pollLiveMarketData();
};

void allocateAndLaunchBlackScholes(MarketClient& client);

#endif // ENGINE_H