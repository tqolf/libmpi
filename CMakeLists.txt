CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT("Cryptograph Algorithms Implementation")

INCLUDE(CMakePackageConfigHelpers)
INCLUDE(cmake/ConfigureTarget.cmake)

ADD_COMPILE_OPTIONS(-Wno-deprecated-declarations)

IF (NOT DEFINED ARCH)
  # MATCHES "^(os|ios|android|linux|win32)$
  IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(x86_64|amd64)$")
    SET(ARCH "x86_64")
  ELSEIF (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    # cmake reports AMD64 on Windows, but we might be building for 32-bit.
    IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(ARCH "x86_64")
    ELSE ()
      SET(ARCH "x86")
    ENDIF ()
  ELSEIF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(x86|i386|i386)$")
    SET(ARCH "x86")
  ELSEIF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(aarch64|arm64|arm64e)$")
    SET(ARCH "aarch64")
  ELSEIF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm*")
    SET(ARCH "arm")
  ELSE ()
    SET(ARCH "generic")
    MESSAGE(STATUE "Unknown processor:" ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF ()
ENDIF ()

IF (UNIX)
  IF (${ARCH} STREQUAL "aarch64")
    IF (APPLE)
      SET(PERLASM_STYLE ios64)
    ELSE ()
      SET(PERLASM_STYLE linux64)
    ENDIF ()
  ELSEIF (${ARCH} STREQUAL "arm")
    IF (APPLE)
      SET(PERLASM_STYLE ios32)
    ELSE ()
      SET(PERLASM_STYLE linux32)
    ENDIF ()
  ELSE ()
    IF (${ARCH} STREQUAL "x86")
      SET(PERLASM_FLAGS "-fPIC -DCRYPTO_IA32_SSE2")
    ENDIF ()
    IF (APPLE)
      SET(PERLASM_STYLE macosx)
    ELSE ()
      SET(PERLASM_STYLE elf)
    ENDIF ()
  ENDIF ()
  SET(ASM_EXT S)
  ENABLE_LANGUAGE(ASM)
  SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wa,--noexecstack")

  # Clang's integerated assembler does not support debug symbols.
  IF (NOT CMAKE_ASM_COMPILER_ID MATCHES "Clang")
    SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wa,-g")
  ENDIF ()

  # CMake does not add -isysroot and -arch flags to assembly.
  IF (APPLE)
    IF (CMAKE_OSX_SYSROOT)
      SET(CMAKE_ASM_FLAGS
          "${CMAKE_ASM_FLAGS} -isysroot \"${CMAKE_OSX_SYSROOT}\""
      )
    ENDIF ()
    FOREACH (arch ${CMAKE_OSX_ARCHITECTURES})
      SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -arch ${arch}")
    ENDFOREACH ()
  ENDIF ()
ELSE ()
  IF (${ARCH} STREQUAL "x86_64")
    SET(PERLASM_STYLE nasm)
  ELSE ()
    SET(PERLASM_STYLE win32n)
    SET(PERLASM_FLAGS "-DCRYPTO_IA32_SSE2")
  ENDIF ()
  SET(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -gcv8")

  # On Windows, we use the NASM output, specifically built with Yasm.
  SET(ASM_EXT asm)
ENDIF ()

FIND_PACKAGE(Perl REQUIRED)
MACRO (PERLASM dest src)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${dest}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND CC=${CMAKE_C_COMPILER} ${PERL_EXECUTABLE} ${src} ${PERLASM_STYLE}
            ${PERLASM_FLAGS} ${ARGN} ${CMAKE_CURRENT_BINARY_DIR}/${dest}
    DEPENDS ${src} ${CMAKE_SOURCE_DIR}/perlasm/arm-xlate.pl
            ${CMAKE_SOURCE_DIR}/perlasm/x86_64-xlate.pl
            ${CMAKE_SOURCE_DIR}/perlasm/x86asm.pl
            ${CMAKE_SOURCE_DIR}/perlasm/x86gas.pl
            ${CMAKE_SOURCE_DIR}/perlasm/x86masm.pl
            ${CMAKE_SOURCE_DIR}/perlasm/x86nasm.pl
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
ENDMACRO ()

IF (BUILD_VENDOR)
  INCLUDE(ExternalProject)
  IF (NOT EXISTS ${CMAKE_SOURCE_DIR}/vendor)
    FILE(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor)
  ENDIF ()

  INCLUDE_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR}/include)
  LINK_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR}/lib)
  SET(ENV{PATH} ${CMAKE_BINARY_DIR}/bin:$ENV{PATH})
  SET(ENV{PKG_CONFIG_PATH} ${CMAKE_BINARY_DIR}/lib/pkgconfig)

  # cmake-format: off
  SET(NASM_LOCAL_FILE ${CMAKE_SOURCE_DIR}/vendor/nasm-2.15.05.tar.gz)
  SET(NASM_DOWNLOAD_URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.15.05.tar.gz)
  IF (NOT EXISTS ${NASM_LOCAL_FILE})
    FILE(
      DOWNLOAD ${NASM_DOWNLOAD_URL} ${NASM_LOCAL_FILE}
      TIMEOUT 60
      TLS_VERIFY ON
    )
  ENDIF ()
  EXTERNALPROJECT_ADD(
    nasm
    URL ${NASM_LOCAL_FILE}
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}
    BUILD_COMMAND make -j${CONCURRENCY}
    BUILD_IN_SOURCE 1
  )

  SET(OPENSSL_LOCAL_FILE ${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i.tar.gz)
  SET(OPENSSL_DOWNLOAD_URL https://www.openssl.org/source/old/1.1.1/openssl-1.1.1i.tar.gz)
  IF (NOT EXISTS ${OPENSSL_LOCAL_FILE})
    FILE(
      DOWNLOAD ${OPENSSL_DOWNLOAD_URL} ${OPENSSL_LOCAL_FILE}
      TIMEOUT 60
      TLS_VERIFY ON
    )
  ENDIF ()
  EXTERNALPROJECT_ADD(
    openssl
    URL ${OPENSSL_LOCAL_FILE}
    CONFIGURE_COMMAND ./config no-shared no-asm -d --prefix=${CMAKE_BINARY_DIR}
    BUILD_COMMAND make depend && make -j${CONCURRENCY}
    INSTALL_COMMAND make install_sw
    BUILD_IN_SOURCE 1
  )
  # cmake-format: on
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i/export/lib)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i/export/include)
ELSE ()
  FIND_PROGRAM(OPENSSL openssl REQUIRED)
  GET_FILENAME_COMPONENT(OPENSSL_DIR ${OPENSSL} DIRECTORY CACHE)
  LINK_DIRECTORIES(${OPENSSL_DIR}/../lib)
  INCLUDE_DIRECTORIES(${OPENSSL_DIR}/../include)
ENDIF ()

LINK_DIRECTORIES(/usr/local/lib)
INCLUDE_DIRECTORIES(/usr/local/include ${CMAKE_BINARY_DIR}/include)

# mpn
ADD_SUBDIRECTORY(mpn)

# mpi
ADD_SUBDIRECTORY(mpi)

# tests
ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)
