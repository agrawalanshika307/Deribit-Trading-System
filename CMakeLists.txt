cmake_minimum_required(VERSION 3.15)

# Set the project name
project(GoQuant)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# cmake_policy(SET CMP0167 NEW)

# Include vcpkg toolchain (optional: set externally if already configured)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

set(OPENSSL_ROOT "C:/vcpkg/installed/x64-windows")
set(OPENSSL_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(OPENSSL_LIBRARY_DIR "C:/vcpkg/installed/x64-windows/lib")
find_package(OpenSSL REQUIRED)

set(CURL_ROOT "C:/vcpkg/installed/x64-windows")
set(CURL_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(CURL_LIBRARY_DIR "C:/vcpkg/installed/x64-windows/lib")

# Find the libcurl package
find_package(CURL REQUIRED)

# Specify the Boost include and library directories explicitly
set(BOOST_ROOT "C:/vcpkg/installed/x64-windows")
set(Boost_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(Boost_LIBRARY_DIR "C:/vcpkg/installed/x64-windows/lib")

find_package(Boost REQUIRED COMPONENTS system asio)

# Add your executable
add_executable(GoQuant main.cpp WebSocketClient.cpp APIClient.cpp)

# Link libcurl to your executable
target_link_libraries(GoQuant PRIVATE CURL::libcurl Boost::system OpenSSL::SSL OpenSSL::Crypto)
add_definitions(-D_WIN32_WINNT=0x0601) # For Windows 7 or higher
