# tests/CMakeLists.txt

add_library(unity
    ${CMAKE_SOURCE_DIR}/unity/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/unity
)


add_executable(unit_tests
    app/test_main.c

    app/protocol/test_protocol.c

    app/mocks/interface_stubs.c
    app/mocks/uart_mocks.c
)

target_sources(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/app/protocol/src/app_protocol.c
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/app/mocks/
    ${CMAKE_SOURCE_DIR}/tests/app/common/

   
    ${CMAKE_SOURCE_DIR}/app/protocol/inc/
    ${CMAKE_SOURCE_DIR}/app/interface/


    ${CMAKE_SOURCE_DIR}/unity/
)


target_link_libraries(unit_tests PRIVATE unity)

add_test(NAME unit_tests COMMAND unit_tests)
