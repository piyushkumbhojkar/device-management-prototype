# Device Management Prototype

A gRPC-based device management system featuring a C++ backend server and Python CLI frontend, designed to simulate device tracking and asynchronous software update operations.

## Overview

This prototype demonstrates a modern approach to device fleet management using gRPC for efficient client-server communication. The system enables device registration, status monitoring, and asynchronous update operations through a clean, type-safe API defined with Protocol Buffers.

## Key Features

- **gRPC Backend Architecture**: Type-safe, high-performance API communication using Protocol Buffers
- **Modern C++17 Implementation**: Efficient server design with support for concurrent request handling
- **Asynchronous Task Processing**: Non-blocking execution of long-running operations using detached background threads
- **Python CLI Interface**: Intuitive command-line client for comprehensive API interaction
- **Cross-Platform Development**: Optimized for Linux and WSL environments with CMake build system
- **In-Memory State Management**: Lightweight state persistence using C++ standard containers

## System Requirements

- **Operating System**: Linux or WSL2
- **Compiler**: C++17-compatible compiler (GCC/g++)
- **Build System**: CMake 3.15 or higher
- **Dependencies**: gRPC and Protocol Buffers libraries
- **Python**: Python 3.x with pip and venv support
- **Python Packages**: `grpcio`, `grpcio-tools`

## Installation and Setup

### 1. Environment Preparation

```bash
# Navigate to project directory
cd device_mgmt_project

# Activate Python virtual environment
source venv/bin/activate

# Generate gRPC stubs for C++ and Python
python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. device_service.proto
```

### 2. Backend Compilation

```bash
# Create build directory
mkdir build && cd build

# Configure build with CMake
cmake ..

# Compile the server
make
```

### 3. Server Execution

```bash
# Start the gRPC server
./device_server
```

The server will begin listening on `0.0.0.0:50051`. Keep this terminal session active.

## Python CLI Usage

The CLI client provides the following commands:

### Register a Device
Adds a new device to the managed fleet.
```bash
python3 src/frontend/client.py register --id D-7001 --version 1.0.0
```

### Query Device Information
Retrieves current device status and firmware version.
```bash
python3 src/frontend/client.py info --id D-7001
```

### Initiate Firmware Update
Triggers an asynchronous update operation (simulated 10-second duration).
```bash
python3 src/frontend/client.py update --id D-7001
```

### Check Action Status
Monitors the progress of asynchronous operations (PENDING, RUNNING, COMPLETED).
```bash
python3 src/frontend/client.py check-action --action-id ACTION-1234
```

### List All Registered Devices
Lists all the registered devices with their firmware version.
```bash
python3 src/frontend/client.py list
```

## API Reference

The complete API specification is defined in `device_service.proto` and includes the following RPC methods:

- **RegisterDevice**: Registers a new device in the system
- **SetDeviceStatus**: Manually updates device status
- **GetDeviceInfo**: Retrieves device information and current state
- **InitiateDeviceAction**: Starts an asynchronous operation and returns an action identifier
- **GetDeviceActionStatus**: Queries the status of a specific action
- **ListDevices**: Retrieves list of device with their firmware version

## Architecture

- **Backend**: C++ gRPC server managing device state and processing requests
- **Frontend**: Python CLI client interfacing with the backend via gRPC
- **Communication**: Protocol Buffers for efficient, structured data serialization
- **Concurrency**: Thread-based asynchronous processing for non-blocking operations

## Development Environment

This project is developed and tested on Windows with WSL2 running Ubuntu, providing a Linux-compatible development environment while maintaining Windows workflow integration.

---

*For issues, questions, or contributions, please refer to the repository's issue tracker.*
