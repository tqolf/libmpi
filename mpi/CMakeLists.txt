# Multiple Precision Integer and Relevant Algorithms

CONFIGURE_FILE(mpi.h ${CMAKE_BINARY_DIR}/include/mpi/mpi.h COPYONLY)
CONFIGURE_FILE(mpi-asm.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-asm.h COPYONLY)
CONFIGURE_FILE(mpi-conf.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-conf.h COPYONLY)
CONFIGURE_FILE(
  mpi-binary.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-binary.h COPYONLY
)
CONFIGURE_FILE(
  mpi-optimizer.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-optimizer.h COPYONLY
)
CONFIGURE_FILE(
  mpi-montgomery.h ${CMAKE_BINARY_DIR}/include/mpi/mpi-montgomery.h COPYONLY
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/include/mpi/mpi.h
              ${CMAKE_BINARY_DIR}/include/mpi/mpi-conf.h
              ${CMAKE_BINARY_DIR}/include/mpi/mpi-optimizer.h
              ${CMAKE_BINARY_DIR}/include/mpi/mpi-binary.h
              ${CMAKE_BINARY_DIR}/include/mpi/mpi-montgomery.h
        DESTINATION include/mpi
)
ADD_LIBRARY(
  mpi mpi.c mpi-binary.c mpi-asm.c mpi-optimizer.c mpi-montgomery.c mpi-prime.c
)
TARGET_STRIP_PATH_PREFIX(mpi)
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

OPTION(MPI_NO_ASM "disable asm for mpi" OFF)
IF ((NOT MPI_NO_ASM) AND (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  ENABLE_LANGUAGE(ASM_NASM)
  IF (NOT DEFINED ARCH)
    SET(ARCH ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF ()
  SET(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -D_L9 -DLINUX32E")
  IF (${ARCH} STREQUAL "x86_64")
    FILE(GLOB ASM_SOURCE asm/intel64/*.asm)
    TARGET_SOURCES(mpi PRIVATE ${ASM_SOURCE})
    TARGET_INCLUDE_DIRECTORIES(mpi PRIVATE asm asm/intel64)
    TARGET_COMPILE_DEFINITIONS(
      mpi
      PRIVATE -DMPI_UADD_SCHOOL_ASM
              -DMPI_USUB_SCHOOL_ASM
              -DMPI_UINC_SCHOOL_ASM
              -DMPI_UDEC_SCHOOL_ASM
              -DMPI_UDIV_ASM
              -DMPI_UMUL_ASM
              -DMPI_USQR_ASM
              -DMPI_UMUL_ADD_ASM
              -DMPI_MONT_REDC_ASM
    )
  ENDIF ()
ENDIF ()
