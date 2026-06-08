cmake_minimum_required(VERSION 3.5)

project(SingleLinkedList LANGUAGES C)

add_executable(SingleLinkedList main.c)

include(GNUInstallDirs)
install(TARGETS SingleLinkedList
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
