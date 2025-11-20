cmake_minimum_required(VERSION 3.15)
project(DeviceManagement)

set(CMAKE_CXX_STANDARD 17)

# Find Protobuf and gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Generate C++ sources from .proto
set(PROTO_SRC "device_service.proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Commands to run protoc
add_custom_command(
  OUTPUT "${PROTO_GEN_DIR}/device_service.pb.cc" "${PROTO_GEN_DIR}/device_service.pb.h"
         "${PROTO_GEN_DIR}/device_service.grpc.pb.cc" "${PROTO_GEN_DIR}/device_service.grpc.pb.h"
  COMMAND protoc
    --grpc_out="${PROTO_GEN_DIR}"
    --cpp_out="${PROTO_GEN_DIR}"
    --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
    -I "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}"
)

# Define the server executable
add_executable(device_server src/backend/service_impl.cc
  "${PROTO_GEN_DIR}/device_service.pb.cc"
  "${PROTO_GEN_DIR}/device_service.grpc.pb.cc"
)

# Link libraries
target_include_directories(device_server PUBLIC "${PROTO_GEN_DIR}")
target_link_libraries(device_server protobuf::libprotobuf gRPC::grpc++ gRPC::grpc++_reflection)