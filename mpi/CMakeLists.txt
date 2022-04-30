# Multiple Precision Integer and Relevant Algorithms

CONFIGURE_FILE(mpi.h ${CMAKE_BINARY_DIR}/include/mpi/mpi.h COPYONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/include/mpi/mpi.h DESTINATION include/mpi)
ADD_LIBRARY(mpi mpi.c mpi-prime.c)
TARGET_CONFIGURE(mpi)
TARGET_LINK_LIBRARIES(mpi PUBLIC mpn)
INSTALL(TARGETS mpi ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

# RSA(Rivest–Shamir–Adleman) Algorithm
OPTION(MPI_NO_RSA "build without rsa algorithm" OFF)
IF (NOT MPI_NO_RSA)
  CONFIGURE_FILE(mpi-rsa.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-rsa.h COPYONLY)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/include/mpi/mpi-rsa.h
          DESTINATION include/mpi
  )
  TARGET_SOURCES(mpi PRIVATE mpi-rsa.c)
ENDIF ()
