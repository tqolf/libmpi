# unit-test
ADD_EXECUTABLE(unittest-mpi unittest-mpi.cpp)
TARGET_LINK_LIBRARIES(unittest-mpi mpi crypto dl gtest pthread)
ADD_TEST(NAME unittest-mpi COMMAND unittest-mpi
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
TARGET_STRIP_PATH_PREFIX(unittest-mpi)

IF (BUILD_VENDOR)
  ADD_DEPENDENCIES(unittest-mpi openssl)
ENDIF ()

# benchmark
ADD_EXECUTABLE(benchmark benchmark.cpp)
TARGET_LINK_LIBRARIES(benchmark mpi crypto pthread dl)
TARGET_COMPILE_OPTIONS(benchmark PRIVATE -std=gnu++17)
TARGET_STRIP_PATH_PREFIX(benchmark)

IF (BUILD_VENDOR)
  ADD_DEPENDENCIES(benchmark openssl)
ENDIF ()

INSTALL(TARGETS benchmark unittest-mpi RUNTIME DESTINATION bin)
