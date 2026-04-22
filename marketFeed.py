import socket
import struct
import time
import random
import yfinance as yf

use_live_data = True
UDP_IP = "127.0.0.1"
UDP_PORT = 54000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Initializing Market Feed to {UDP_IP}:{UDP_PORT}...")

current_price = 150.00

while True:
    try:
        if use_live_data:
            ticker = yf.Ticker("AAPL")
            price = ticker.fast_info['last_price']
        else:
            price = current_price + random.uniform(-0.50, 0.50)
            current_price = price

        data = struct.pack("f", price)
        sock.sendto(data, (UDP_IP, UDP_PORT))
        
        print(f"Transmitted Live Tick: ${price:.2f}")
        time.sleep(1.0)
        
    except Exception as e:
        print(f"Network error: {e}")
        time.sleep(1)