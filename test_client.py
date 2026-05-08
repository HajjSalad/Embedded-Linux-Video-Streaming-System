#!/usr/bin/env python3
import requests
import time
import sys

PI_IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.185"
PI_PORT = 8080

commands = [
    "/capture-video",
    "/stream-live", 
    "/stream-recorded"
]

print(f"Testing Pi at {PI_IP}:{PI_PORT}")
print("Press Ctrl+C to stop\n")

try:
    while True:
        for cmd in commands:
            url = f"http://{PI_IP}:{PI_PORT}{cmd}"
            print(f"Sending: {cmd}")
            try:
                r = requests.get(url, timeout=3)
                print(f"  → {r.status_code}\n")
            except Exception as e:
                print(f"  → Error: {e}\n")
            time.sleep(3)
except KeyboardInterrupt:
    print("\nStopped")