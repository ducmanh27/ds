cmake_minimum_required(VERSION 3.5)

project(Pointer LANGUAGES C)

add_executable(Pointer main.c)

include(GNUInstallDirs)
install(TARGETS Pointer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
