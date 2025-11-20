#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>

#include <grpcpp/grpcpp.h>
#include "device_service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using namespace device;

// Helper to generate a simple random Action ID
std::string GenerateActionID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    return "ACTION-" + std::to_string(dis(gen));
}

class DeviceServiceImpl final : public DeviceManagement::Service {
public:
    // 1. Register Device
    Status RegisterDevice(ServerContext* context, const RegisterDeviceRequest* request,
                          RegisterDeviceResponse* response) override {
        std::lock_guard<std::mutex> lock(db_mutex_);
        
        if (device_db_.find(request->device_id()) != device_db_.end()) {
            response->set_success(false);
            response->set_message("Device already exists.");
            return Status::OK;
        }

        Device new_device;
        new_device.set_id(request->device_id());
        new_device.set_firmware_version(request->initial_firmware_version());
        new_device.set_status(DeviceStatus::IDLE);

        device_db_[request->device_id()] = new_device;

        response->set_success(true);
        response->set_message("Device registered successfully.");
        std::cout << "[Server] Registered device: " << request->device_id() << std::endl;
        return Status::OK;
    }

    // 2. Set Device Status
    Status SetDeviceStatus(ServerContext* context, const SetDeviceStatusRequest* request,
                           SetDeviceStatusResponse* response) override {
        std::lock_guard<std::mutex> lock(db_mutex_);

        auto it = device_db_.find(request->device_id());
        if (it == device_db_.end()) {
            response->set_success(false);
            return Status(grpc::NOT_FOUND, "Device not found");
        }

        it->second.set_status(request->status());
        response->set_success(true);
        std::cout << "[Server] Updated status for: " << request->device_id() << std::endl;
        return Status::OK;
    }

    // 3. Get Device Info
    Status GetDeviceInfo(ServerContext* context, const GetDeviceInfoRequest* request,
                         GetDeviceInfoResponse* response) override {
        std::lock_guard<std::mutex> lock(db_mutex_);

        auto it = device_db_.find(request->device_id());
        if (it == device_db_.end()) {
            return Status(grpc::NOT_FOUND, "Device not found");
        }

        *response->mutable_device() = it->second;
        return Status::OK;
    }

    // 4. Initiate Device Action (Async Simulation)
    Status InitiateDeviceAction(ServerContext* context, const InitiateDeviceActionRequest* request,
                                InitiateDeviceActionResponse* response) override {
        std::lock_guard<std::mutex> lock(db_mutex_);

        auto it = device_db_.find(request->device_id());
        if (it == device_db_.end()) {
            response->set_success(false);
            response->set_message("Device not found");
            return Status::OK;
        }

        // Generate ID and set initial state
        std::string action_id = GenerateActionID();
        action_db_[action_id] = {ActionStatus::RUNNING, "Starting..."};
        
        // Update device to UPDATING state
        it->second.set_status(DeviceStatus::UPDATING);

        std::cout << "[Server] Starting action " << action_id << " on device " << request->device_id() << std::endl;

        // Launch a detached thread to simulate long-running work
        std::string device_id = request->device_id();
        std::thread([this, action_id, device_id]() {
            // Simulate 10 seconds of work
            std::this_thread::sleep_for(std::chrono::seconds(10));

            // Re-acquire lock to update state
            std::lock_guard<std::mutex> lock(db_mutex_);
            
            // Update Action to COMPLETED
            action_db_[action_id] = {ActionStatus::COMPLETED, "Success"};

            // Update Device back to IDLE
            if (device_db_.find(device_id) != device_db_.end()) {
                device_db_[device_id].set_status(DeviceStatus::IDLE);
                // Bump version number for fun
                device_db_[device_id].set_firmware_version("2.0.0"); 
            }
            std::cout << "[Server] Action " << action_id << " finished." << std::endl;
        }).detach();

        response->set_action_id(action_id);
        response->set_success(true);
        response->set_message("Action initiated successfully.");
        return Status::OK;
    }

    // 5. Get Action Status
    Status GetDeviceActionStatus(ServerContext* context, const GetDeviceActionStatusRequest* request,
                                 GetDeviceActionStatusResponse* response) override {
        std::lock_guard<std::mutex> lock(db_mutex_);

        auto it = action_db_.find(request->action_id());
        if (it == action_db_.end()) {
            return Status(grpc::NOT_FOUND, "Action ID not found");
        }

        response->set_action_id(request->action_id());
        response->set_status(it->second.first);
        response->set_details(it->second.second);
        return Status::OK;
    }

private:
    // In-memory storage
    std::map<std::string, Device> device_db_;
    // Map of ActionID -> {Status, Details}
    std::map<std::string, std::pair<ActionStatus, std::string>> action_db_;
    
    // Mutex to protect shared data
    std::mutex db_mutex_;
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    DeviceServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}