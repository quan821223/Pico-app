cmake_minimum_required(VERSION 3.16)

project(pico_app_host_tests C)

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_library(protocol_stream_parser STATIC
    ../../src/protocol/protocol_stream_parser.c
)

target_include_directories(protocol_stream_parser PUBLIC
    ../../src/protocol
)

target_compile_features(protocol_stream_parser PUBLIC c_std_11)

if(MSVC)
    target_compile_options(protocol_stream_parser PRIVATE /W4 /WX)
else()
    target_compile_options(protocol_stream_parser PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

add_library(application_protocol STATIC
    ../../src/application/app_protocol.c
    ../../src/application/protocol_service.c
)

target_include_directories(application_protocol PUBLIC
    ../../src/application
    ../../src/protocol
)

target_compile_features(application_protocol PUBLIC c_std_11)
target_link_libraries(application_protocol PUBLIC protocol_stream_parser)

if(MSVC)
    target_compile_options(application_protocol PRIVATE /W4 /WX)
else()
    target_compile_options(application_protocol PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

add_library(board_profile STATIC
    ../../src/board/board_profile.c
)
target_include_directories(board_profile PUBLIC ../../src/board)
target_compile_features(board_profile PUBLIC c_std_11)
target_compile_definitions(board_profile PRIVATE APP_ACTIVE_BOARD_ID=0)

if(MSVC)
    target_compile_options(board_profile PRIVATE /W4 /WX)
else()
    target_compile_options(board_profile PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

add_executable(test_protocol_stream_parser
    test_protocol_stream_parser.c
)

target_link_libraries(test_protocol_stream_parser PRIVATE
    protocol_stream_parser
)

if(MSVC)
    target_compile_options(test_protocol_stream_parser PRIVATE /W4 /WX)
else()
    target_compile_options(test_protocol_stream_parser PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

add_test(NAME protocol_stream_parser COMMAND test_protocol_stream_parser)

add_executable(test_application_protocol
    test_application_protocol.c
)
target_link_libraries(test_application_protocol PRIVATE application_protocol)

add_executable(test_protocol_service
    test_protocol_service.c
)
target_link_libraries(test_protocol_service PRIVATE application_protocol)

add_executable(protocol_contract_runner
    protocol_contract_runner.c
)
target_link_libraries(protocol_contract_runner PRIVATE application_protocol)

add_executable(test_board_profile test_board_profile.c)
target_link_libraries(test_board_profile PRIVATE board_profile)

if(MSVC)
    target_compile_options(test_application_protocol PRIVATE /W4 /WX)
    target_compile_options(test_protocol_service PRIVATE /W4 /WX)
    target_compile_options(protocol_contract_runner PRIVATE /W4 /WX)
    target_compile_options(test_board_profile PRIVATE /W4 /WX)
else()
    target_compile_options(test_application_protocol PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(test_protocol_service PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(protocol_contract_runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
    target_compile_options(test_board_profile PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

add_test(NAME application_protocol COMMAND test_application_protocol)
add_test(NAME protocol_service COMMAND test_protocol_service)
add_test(NAME board_profile COMMAND test_board_profile)
add_test(
    NAME protocol_characterization_contract
    COMMAND ${CMAKE_COMMAND} -E env
        PROTOCOL_CONTRACT_RUNNER=$<TARGET_FILE:protocol_contract_runner>
        ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../characterization/test_protocol_contract.py
)
