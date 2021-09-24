# unit-test
ADD_EXECUTABLE(unittest-mpi unittest-mpi.cpp)
TARGET_LINK_LIBRARIES(unittest-mpi mpi crypto dl gtest pthread)
ADD_TEST(NAME unittest-mpi COMMAND unittest-mpi
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
TARGET_STRIP_PATH_PREFIX(unittest-mpi)

# benchmark
ADD_EXECUTABLE(benchmark benchmark.cpp)
TARGET_LINK_LIBRARIES(benchmark mpi crypto pthread dl)
TARGET_COMPILE_OPTIONS(benchmark PRIVATE -std=gnu++17)
TARGET_STRIP_PATH_PREFIX(benchmark)

INSTALL(TARGETS benchmark RUNTIME DESTINATION bin)
