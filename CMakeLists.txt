cmake_minimum_required(VERSION 3.5)

project(ServerPi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ServerPi main.cpp
    Server/ServerPi.h Server/ServerPi.cpp
    Protocols//EngineSensors.h Protocols/EngineSensors.cpp
    Protocols/VoltageRegulators.h Protocols/VoltageRegulators.cpp
    Protocols/Constants.h
    Can/Reader.h Can/Reader.cpp
    Can/Common.h)

include(GNUInstallDirs)
install(TARGETS ServerPi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
