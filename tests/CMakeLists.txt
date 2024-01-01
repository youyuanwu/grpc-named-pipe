set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS unit_test_framework )

file(GLOB SOURCES
*_test.cpp
*.hpp
)

set(test_name gnp_test)

add_executable(${test_name} ${SOURCES})
target_include_directories(${test_name} 
  PRIVATE .
)

# target_compile_definitions(${test_name} PRIVATE WINASIO_LOG) # enable logging

target_link_libraries(${test_name} PRIVATE Boost::unit_test_framework Boost::disable_autolinking grpc_named_pipe)
set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 17)
add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})