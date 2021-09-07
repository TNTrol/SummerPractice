cmake_minimum_required(VERSION 3.19)
project(SummerPractice C)

set(CMAKE_C_STANDARD 99)
find_package(OpenSSL REQUIRED)
    add_executable(SummerPractice src/main.c src/scanner.c include/scanner.h include/utils.h src/visiting_servers.c include/visiting_servers.h src/thread_pool.c include/thread_pool.h)

#target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto )
TARGET_LINK_LIBRARIES( SummerPractice pthread)

find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    #target_include_directories(${PROJECT_NAME} PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
else()
    message(STATUS "OpenSSL Not Found")
endif()

