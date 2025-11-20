import sys
import grpc
import argparse
import time

# Import generated classes
import device_service_pb2
import device_service_pb2_grpc

def run():
    # Connect to the server
    channel = grpc.insecure_channel('localhost:50051')
    stub = device_service_pb2_grpc.DeviceManagementStub(channel)

    # Parse Command Line Arguments
    parser = argparse.ArgumentParser(description="Device Management CLI")
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # 1. Register Command
    reg_parser = subparsers.add_parser("register", help="Register a new device")
    reg_parser.add_argument("--id", required=True, help="Device ID")
    reg_parser.add_argument("--version", default="1.0.0", help="Initial Firmware Version")

    # 2. Get Info Command
    info_parser = subparsers.add_parser("info", help="Get device info")
    info_parser.add_argument("--id", required=True, help="Device ID")

    # 3. Update Command (Action)
    update_parser = subparsers.add_parser("update", help="Trigger software update")
    update_parser.add_argument("--id", required=True, help="Device ID")

    # 4. Check Action Status
    status_parser = subparsers.add_parser("check-action", help="Check status of an action")
    status_parser.add_argument("--action-id", required=True, help="Action ID")

    args = parser.parse_args()

    try:
        if args.command == "register":
            response = stub.RegisterDevice(device_service_pb2.RegisterDeviceRequest(
                device_id=args.id, 
                initial_firmware_version=args.version
            ))
            print(f"Success: {response.success}, Message: {response.message}")

        elif args.command == "info":
            response = stub.GetDeviceInfo(device_service_pb2.GetDeviceInfoRequest(device_id=args.id))
            # Map enum integer to string for display
            status_map = {0:"UNKNOWN", 1:"IDLE", 2:"BUSY", 3:"OFFLINE", 4:"MAINTENANCE", 5:"UPDATING", 6:"ERROR"}
            status_str = status_map.get(response.device.status, "UNKNOWN")
            print(f"Device: {response.device.id}")
            print(f"  Version: {response.device.firmware_version}")
            print(f"  Status:  {status_str}")

        elif args.command == "update":
            print(f"Triggering update for {args.id}...")
            response = stub.InitiateDeviceAction(device_service_pb2.InitiateDeviceActionRequest(
                device_id=args.id,
                action_type=device_service_pb2.SOFTWARE_UPDATE,
                parameters="2.0.0"
            ))
            if response.success:
                print(f"Update Started! Action ID: {response.action_id}")
                print("Check status with: python client.py check-action --action-id " + response.action_id)
            else:
                print(f"Failed: {response.message}")

        elif args.command == "check-action":
            response = stub.GetDeviceActionStatus(device_service_pb2.GetDeviceActionStatusRequest(
                action_id=args.action_id
            ))
            # Map enum to string
            status_map = {0:"PENDING", 1:"RUNNING", 2:"COMPLETED", 3:"FAILED"}
            print(f"Action {response.action_id}: {status_map.get(response.status, 'UNKNOWN')}")

        else:
            parser.print_help()

    except grpc.RpcError as e:
        print(f"RPC Error: {e.code()} - {e.details()}")

if __name__ == '__main__':
    run()