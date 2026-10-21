cmake_minimum_required(VERSION 3.14.0)
project(mailbox VERSION 1.2.0)

if(NOT TARGET devicetree)
    add_subdirectory(devicetree.resource EXCLUDE_FROM_ALL)
endif()

# What is ours comes after the project above: it defines a get_verstring() and an sfdc() of its own, and the last definition is the one that stays
include(cmake/bin2h.cmake)
include(cmake/verstring.cmake)
include(cmake/sfdc.cmake)

get_verstring(VERSTRING)
sfdc(sfd/mailbox_lib.sfd)

add_executable(mailbox.resource
    src/main.c
    src/init.c
    src/rawcommand.c
    src/stringcommand.c
    src/getclockrate.c
    src/setclockrate.c
    src/getclockstate.c
    src/setclockstate.c
    src/getpowerstate.c
    src/setpowerstate.c
    src/getgpiostate.c
    src/setgpiostate.c
    src/obtainlock.c
    src/releaselock.c
    src/mbox.c
)

bin_to_header(mailbox.resource)

target_compile_options(mailbox.resource PRIVATE -Os -m68040 -msmall-code -fomit-frame-pointer)
# One place for the version: project(), above. The id string, lib_Version and lib_Revision all come from it
target_compile_definitions(mailbox.resource PRIVATE
    VERSION_STRING="${VERSTRING}"
    MB_VERSION=${PROJECT_VERSION_MAJOR}
    MB_REVISION=${PROJECT_VERSION_MINOR}
)
target_include_directories(mailbox.resource PRIVATE include)
target_link_options(mailbox.resource PRIVATE -ffreestanding -nostdlib -nostartfiles -s -Wl,-e_code_start -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/ldscript.lds)
target_link_libraries(mailbox.resource devicetree)

set_target_properties(mailbox.resource PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ldscript.lds)

# no install for devicetree.resource itself, it goes to ROM
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sfd DESTINATION Developer)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Autodocs DESTINATION Developer)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/resources DESTINATION Developer/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/FD DESTINATION Developer)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include DESTINATION Developer)
