cmake_minimum_required(VERSION 3.19)
project(SummerPractice C)

set(CMAKE_C_STANDARD 99)
find_package(OpenSSL REQUIRED)
    add_executable(SummerPractice main.c scanner.c scanner.h utils.h)


#target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto )
#target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    #target_include_directories(${PROJECT_NAME} PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
else()
    message(STATUS "OpenSSL Not Found")
endif()