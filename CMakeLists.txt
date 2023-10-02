cmake_minimum_required(VERSION 3.25)
project(laar_test_task)

set(CMAKE_CXX_STANDARD 23)

# Search for CSP files in the views directory
FILE(GLOB SCP_LIST ${CMAKE_CURRENT_SOURCE_DIR}/views/*.csp)
foreach(cspFile ${SCP_LIST})
    message(STATUS "cspFile:" ${cspFile})
    EXEC_PROGRAM(basename ARGS "-s .csp ${cspFile}" OUTPUT_VARIABLE classname)
    message(STATUS "view classname:" ${classname})
    add_custom_command(OUTPUT ${classname}.h ${classname}.cc
            COMMAND drogon_ctl
            ARGS create view ${cspFile}
            DEPENDS ${cspFile}
            VERBATIM )
    set(VIEWSRC ${VIEWSRC} ${classname}.cc)
endforeach()

add_executable(laar_test_task
        main.cpp
        handlers/ping.cpp
        handlers/ping.h
        handlers/gmail.h
        handlers/gmail.cpp
        models/gmail_client.cpp
        models/gmail_client.h
        ${VIEWSRC})  # Add the generated view source files here

find_package(Drogon CONFIG REQUIRED)
target_link_libraries(laar_test_task PRIVATE Drogon::Drogon)
