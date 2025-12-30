#!/usr/bin/env python3
"""
Send commands to the sensor controller via Unix domain socket.
Commands: START, STOP, STATUS, SET_RATE <value>
"""

import socket
import sys

SOCKET_PATH = "/tmp/sensor_ctrl.sock"

def send_command(command):
    """Send a command to the sensor controller and print the response."""
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
            client.connect(SOCKET_PATH)
            # Send command with newline
            client.sendall((command + "\n").encode())
            
            # Receive response (read until connection closes)
            response = b""
            while True:
                chunk = client.recv(1024)
                if not chunk:
                    break
                response += chunk
            
            print(f"Sent: {command}")
            print(f"Response: {response.decode().strip()}")
            return response.decode()
    except FileNotFoundError:
        print(f"Error: Socket not found at {SOCKET_PATH}")
        print("Make sure the sensor controller is running.")
        sys.exit(1)
    except ConnectionRefusedError:
        print(f"Error: Connection refused to {SOCKET_PATH}")
        print("Make sure the sensor controller is running.")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python3 send_command.py START")
        print("  python3 send_command.py STOP")
        print("  python3 send_command.py STATUS")
        print("  python3 send_command.py SET_RATE 10000")
        sys.exit(1)
    
    command = " ".join(sys.argv[1:])
    send_command(command)

