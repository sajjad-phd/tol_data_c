# Channel 4 Ring Buffer Logger

## Overview
This program acquires data from **Channel 4** of the MCC 118 HAT board using a **ring buffer architecture** with three threads:
- **Control thread**: Listens on Unix socket for commands (START, STOP, STATUS, SET_RATE)
- **Producer thread**: Reads from sensor and writes to ring buffer (when START is received)
- **Consumer thread**: Reads from ring buffer and writes to binary chunk files

## Features
- **Default scan rate**: 120 Hz (configurable via SET_RATE command)
- **Chunk duration**: 2 seconds
- **Ring buffer**: 4 MB circular buffer to decouple reading from writing
- **File format**: Binary format with header (as per specification)
- **Atomic writes**: Files written as `.bin.part` then renamed to `.bin` when complete
- **Unix socket control**: Control via `/run/sensor_ctrl.sock`

## Architecture

### Ring Buffer
- **Size**: 4 MB (can hold ~500,000 samples)
- **Overflow policy**: Drops oldest data (keeps latest) if buffer fills
- **Thread-safe**: Uses mutex and condition variables

### File Format
Binary files with the following structure:

**Header** (fixed size, little-endian):
- `magic` (4 bytes): "SDAT"
- `version` (uint16): 1
- `device_id` (uint32): Device identifier
- `boot_id` (uint64): Random ID generated at program start
- `seq_start` (uint64): Monotonic sequence counter
- `sample_rate_hz` (uint32): 4000
- `record_size` (uint16): 8 (sizeof(double))
- `sample_count` (uint32): Number of samples in chunk
- `sensor_time_start` (uint64): Timestamp
- `sensor_time_end` (uint64): Timestamp
- `payload_crc32` (uint32): CRC32 (currently 0)

**Payload**:
- `sample_count` × `record_size` bytes of raw sample data (doubles)

## Requirements

### System Dependencies
- **daqhats library**: Must be installed on the system (typically at `/usr/local/lib/libdaqhats.so`)
- **daqhats headers**: Must be installed at `/usr/local/include/daqhats/`
- **pthread**: Standard POSIX threading library

### Installation
The daqhats library should be installed separately. This project depends on it but does not include it.

## Compilation
```bash
cd tol_data_c
make
```

## Running

### Start the logger
```bash
./channel4_ringbuffer_logger
```

The program will:
1. Create Unix socket at `/run/sensor_ctrl.sock`
2. Wait for commands via socket
3. Start/stop acquisition based on commands

### Control via Python script
```bash
# Start acquisition
python3 send_command.py START

# Check status
python3 send_command.py STATUS

# Change scan rate to 10000 Hz
python3 send_command.py SET_RATE 10000

# Stop acquisition
python3 send_command.py STOP
```

### Control via direct socket connection
```python
import socket

SOCKET_PATH = "/run/sensor_ctrl.sock"

def send_command(command):
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
        client.connect(SOCKET_PATH)
        client.sendall(command.encode())
        response = client.recv(1024).decode()
        print(response)

# Examples:
send_command("START")
send_command("STATUS")
send_command("SET_RATE 10000")
send_command("STOP")
```

## Commands

- **START**: Begin data acquisition
- **STOP**: Stop data acquisition
- **STATUS**: Get current status (capture state, rate, buffer info, sequence counter)
- **SET_RATE <value>**: Set scan rate in Hz (e.g., `SET_RATE 10000`)

## Output Files
Files are saved to: `DAD_Files/`

**File naming format**: `chunk_<sequence>_.bin`

Example: `chunk_0_.bin`, `chunk_240_.bin` (sequence increments by samples per chunk)

- Files are written as `.bin.part` during writing
- Automatically renamed to `.bin` when complete (atomic operation)
- Python uploader should only process `.bin` files, never `.part` files

## Thread Safety
- Producer thread has higher priority (reads sensor continuously)
- Consumer thread writes to disk (can be slower without blocking sensor reads)
- Ring buffer prevents blocking between threads

## Error Handling
- Ring buffer overflow: Oldest data is dropped (keeps latest)
- File write errors: Error message printed, program continues
- Device errors: Program stops gracefully

## Notes
- The ring buffer size (4 MB) provides several seconds of cushion for SD card stalls
- At 120 Hz with 8-byte samples, data rate is ~960 bytes/s
- At 10 kHz with 8-byte samples, data rate is ~80 KB/s
- Ring buffer can hold significant amount of data depending on scan rate
- Chunk file size depends on scan rate: `(rate × 2 seconds × 8 bytes) + header`
- Default scan rate is 120 Hz if no SET_RATE command is sent

## Project Structure
```
tol_data_c/
├── channel4_ringbuffer_logger.c  # Main source file
├── daqhats_utils.h                # Minimal utility functions
├── send_command.py                # Python script to send commands
├── makefile                       # Build configuration
├── README.md                      # This file
└── DAD_Files/                     # Output directory (created automatically)
```

## Socket Permissions

The socket is created at `/run/sensor_ctrl.sock` with permissions 0666 (readable/writable by all).
If you need to run as a regular user, you may need to create `/run` directory or use a different path.

