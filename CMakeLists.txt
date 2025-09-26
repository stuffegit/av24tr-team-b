cmake_minimum_required(VERSION 3.22)

project(av24tr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

#Strcutre project
set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/desktop/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/desktop/server)
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/shared)

# Path to the target font and install location
set(FONT_NAME MaterialIcons.ttf)
set(SOURCE_FONT_PATH ${CMAKE_SOURCE_DIR}/desktop/client/res/${FONT_NAME})
set(TARGET_FONT_DIR $ENV{HOME}/.local/share/fonts)
set(TARGET_FONT_PATH ${TARGET_FONT_DIR}/${FONT_NAME})

# Check if the font is already installed
if(NOT EXISTS ${TARGET_FONT_PATH})
    message(STATUS "Installing ${FONT_NAME} to ${TARGET_FONT_DIR}")
    # Create the fonts directory if it doesn't exist
    file(MAKE_DIRECTORY ${TARGET_FONT_DIR})
    # Copy the font
    file(COPY ${SOURCE_FONT_PATH} DESTINATION ${TARGET_FONT_DIR})
    # Update the font cache
    execute_process(
        COMMAND fc-cache -f -v ${TARGET_FONT_DIR}
        RESULT_VARIABLE FC_RESULT
        OUTPUT_VARIABLE FC_OUTPUT
        ERROR_VARIABLE FC_ERROR
    )

    if(NOT FC_RESULT EQUAL 0)
        message(WARNING "fc-cache failed: ${FC_ERROR}")
    else()
        message(STATUS "Font cache updated:\n${FC_OUTPUT}")
    endif()
else()
    message(STATUS "${FONT_NAME} already exists, skipping installation")
endif()

# Path to the target sound and install location
set(SOUND_NAME "sound.wav")
set(SOURCE_SOUND_PATH ${CMAKE_SOURCE_DIR}/desktop/client/res/${SOUND_NAME})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/${SOUND_NAME}")
	file(COPY ${SOURCE_SOUND_PATH} DESTINATION ${CMAKE_BINARY_DIR})
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

#Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

if(UARTCOM)
    find_package(Qt6 REQUIRED COMPONENTS SerialPort)
    set(SERVER_UPORT "/dev/ttyUSB0")
    set(CLIENT_UPORT "/dev/ttyUSB1")
    add_compile_definitions(UARTCOM UART_SPORT="${SERVER_UPORT}" UART_CPORT="${CLIENT_UPORT}")

    set(CLIENT_HEADERS 
        ${CLIENT_HEADERS} 
        ${CLIENT_DIR}/include/uartservice.h
        ${CLIENT_DIR}/include/window.h
        ${CLIENT_DIR}/include/canvas.h
        ${CLIENT_DIR}/include/comservice.h)

    set(CLIENT_SOURCES 
        ${CLIENT_SOURCES} 
        ${CLIENT_DIR}/src/uartservice.cpp
        ${CLIENT_DIR}/src/window.cpp
        ${CLIENT_DIR}/src/canvas.cpp
        ${CLIENT_DIR}/src/comservice.cpp)

    set(CLIENT_LIBRARIES ${CLIENT_LIBRARIES} Qt6::SerialPort)

    set(SERVER_HEADERS
        ${SERVER_HEADERS}
        ${SERVER_DIR}/include/uartservice.h
        ${SERVER_DIR}/include/window.h
        ${SERVER_DIR}/include/comservice.h)

    set(SERVER_SOURCES
        ${SERVER_SOURCES}
        ${SERVER_DIR}/src/uartservice.cpp
        ${SERVER_DIR}/src/window.cpp
        ${SERVER_DIR}/src/comservice.cpp)

        set(SERVER_LIBRARIES ${SERVER_LIBRARIES} Qt6::SerialPort)

    add_custom_target(upload_client cd ${CMAKE_SOURCE_DIR}/esp32/client && pio run --target upload --upload-port ${CLIENT_UPORT})
    add_custom_target(upload_server cd ${CMAKE_SOURCE_DIR}/esp32/server && pio run --target upload --upload-port ${SERVER_UPORT})

else()
    set(CLIENT_HEADERS 
        ${CLIENT_HEADERS} 
        ${CLIENT_DIR}/include/tcpservice.h
        ${CLIENT_DIR}/include/window.h
        ${CLIENT_DIR}/include/canvas.h
        ${CLIENT_DIR}/include/comservice.h)

    set(CLIENT_SOURCES
        ${CLIENT_SOURCES}
        ${CLIENT_DIR}/src/tcpservice.cpp
        ${CLIENT_DIR}/src/window.cpp
        ${CLIENT_DIR}/src/canvas.cpp
        ${CLIENT_DIR}/src/comservice.cpp)

    set(SERVER_HEADERS
        ${SERVER_HEADERS}
        ${SERVER_DIR}/include/tcpservice.h
        ${SERVER_DIR}/include/window.h
        ${SERVER_DIR}/include/comservice.h)

    set(SERVER_SOURCES 
        ${SERVER_SOURCES}
        ${SERVER_DIR}/src/tcpservice.cpp
        ${SERVER_DIR}/src/window.cpp
        ${SERVER_DIR}/src/comservice.cpp)

endif()


add_executable(client_app 
    ${CLIENT_DIR}/main.cpp
    ${CLIENT_HEADERS}
    ${CLIENT_SOURCES})
target_link_libraries(client_app PUBLIC ${CLIENT_LIBRARIES} Qt6::Core Qt6::Widgets Qt6::Multimedia)
target_include_directories(client_app PUBLIC ${CLIENT_DIR}/include ${SHARED_DIR})

add_executable(server_app 
    ${SERVER_DIR}/main.cpp
    ${SERVER_HEADERS} 
    ${SERVER_SOURCES})
target_link_libraries(server_app PUBLIC ${SERVER_LIBRARIES} Qt6::Core Qt6::Widgets Qt6::Multimedia)
target_include_directories(server_app PUBLIC ${SERVER_DIR}/include ${SHARED_DIR})

add_custom_target(use_uart cd ${CMAKE_BINARY_DIR} && cmake .. -DUARTCOM:BOOL=ON && make)
add_custom_target(use_tcp cd ${CMAKE_BINARY_DIR} && cmake .. -DUARTCOM:BOOL=OFF && make)