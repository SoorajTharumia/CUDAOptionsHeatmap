import socket
import struct
import time
import urllib.request
import json

UDP_IP = "127.0.0.1"
UDP_PORT = 54000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Initializing ZERO-DEPENDENCY Multi-Variable Market Feed to {UDP_IP}:{UDP_PORT}...")

def get_yahoo_price(ticker):
    url = f"https://query1.finance.yahoo.com/v8/finance/chart/{ticker}"
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    
    with urllib.request.urlopen(req, timeout=5) as response:
        data = json.loads(response.read().decode())
        return data['chart']['result'][0]['meta']['regularMarketPrice']

while True:
    try:
        price = get_yahoo_price("AAPL")
        
        r = get_yahoo_price("^IRX") / 100.0
        
        sigma = get_yahoo_price("^VIX") / 100.0

        data = struct.pack("fff", price, r, sigma)
        sock.sendto(data, (UDP_IP, UDP_PORT))
        
        print(f"Transmitted LIVE Tick -> AAPL: ${price:.2f} | Rate: {r:.4f} | Volatility: {sigma:.4f}")
        
        time.sleep(1.0) 
        
    except Exception as e:
        print(f"Network/API error: {e}")
        time.sleep(2.0)