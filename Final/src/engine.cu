#include "engine.h"
#include <iostream>

__global__ void calculateBlackScholesPriceKernel(uint32_t numOptions, const float* deviceStockPrices, const float* deviceStrikePrices, const float* deviceTimeToMaturity, const float* deviceRiskFreeRates, const float* deviceVolatilities, float* devicePrices) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= numOptions)
    {
        return;
    }

    float S = deviceStockPrices[index];
    float K = deviceStrikePrices[index];
    float r = deviceRiskFreeRates[index];
    float t = deviceTimeToMaturity[index];
    float sigma = deviceVolatilities[index];

    float d1 = (logf(S / K) + (r + (0.5 * sigma * sigma) * t)) / (sigma * sqrtf(t));
    float d2 = d1 - sigma * sqrtf(t);

    devicePrices[index] = S * normcdff(d1) - K * expf(-r * t) * normcdff(d2);
}

void allocateAndLaunchBlackScholes(MarketClient& client) {
    uint32_t numOptions = client.numOptions;
    size_t bytes = numOptions * sizeof(float);

    cudaMalloc((void**) &client.deviceStockPrices, bytes);
    cudaMalloc((void**) &client.deviceStrikePrices, bytes);
    cudaMalloc((void**) &client.deviceTimeToMaturity, bytes);
    cudaMalloc((void**) &client.deviceRiskFreeRates, bytes);
    cudaMalloc((void**) &client.deviceVolatilities, bytes);
    cudaMalloc((void**) &client.devicePrices, bytes);

    cudaMemcpy(client.deviceStockPrices, client.hostStockPrices.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(client.deviceStrikePrices, client.hostStrikePrices.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(client.deviceTimeToMaturity, client.hostTimeToMaturity.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(client.deviceRiskFreeRates, client.hostRiskFreeRates.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(client.deviceVolatilities, client.hostVolatilities.data(), bytes, cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (numOptions + threadsPerBlock - 1) / threadsPerBlock;
    
    calculateBlackScholesPriceKernel<<<blocksPerGrid, threadsPerBlock>>>(
        numOptions, client.deviceStockPrices, client.deviceStrikePrices, 
        client.deviceTimeToMaturity, client.deviceRiskFreeRates, 
        client.deviceVolatilities, client.devicePrices
    );
    
    cudaDeviceSynchronize(); 
    cudaMemcpy(client.hostPrices.data(), client.devicePrices, bytes, cudaMemcpyDeviceToHost);

    cudaFree(client.deviceStockPrices);
    cudaFree(client.deviceStrikePrices);
    cudaFree(client.deviceTimeToMaturity);
    cudaFree(client.deviceRiskFreeRates);
    cudaFree(client.deviceVolatilities);
    cudaFree(client.devicePrices);
}