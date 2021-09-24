CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT("Cryptograph Algorithms Implementation")

IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  SET(CMAKE_COMPILER_IS_CLANG ON CACHE BOOL "Clang Compiler")
ENDIF ()

# libunwind: a portable and efficient C programming interface (API) to determine the call-chain of a program
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
  FIND_PACKAGE(PkgConfig QUIET)
  IF (PkgConfig_FOUND)
    PKG_CHECK_MODULES(LIBUNWIND libunwind-generic)
    IF (LIBUNWIND_FOUND)
      ADD_DEFINITIONS(-DHAVE_LIBUNWIND)
    ELSE ()
      MESSAGE(STATUS "libunwind not found. Disabling unwind tests.")
    ENDIF ()
  ELSE ()
    MESSAGE(STATUS "pkgconfig not found. Disabling unwind tests.")
  ENDIF ()
ENDIF ()

MACRO (APPEND_TO_LISTS)
  SET(options)
  SET(map_options)
  SET(multimap_options LISTS VALUES)

  CMAKE_PARSE_ARGUMENTS(
    PREFIX "${options}" "${map_options}" "${multimap_options}" ${ARGN}
  )
  FOREACH (dst ${PREFIX_LISTS})
    LIST(APPEND ${dst} ${PREFIX_VALUES})
  ENDFOREACH ()
ENDMACRO ()

IF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
  # Note clang-cl is odd and sets both CLANG and MSVC.
  # We base our configuration primarily on our normal Clang one.
  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
    VALUES -Wformat=2
           -Wsign-compare
           -Wmissing-field-initializers
           -Wwrite-strings
           -Wvla
           -Wcast-align
           -Wcast-qual
           -Wswitch-enum
           -Wundef
           -Wdouble-promotion
           -Wdate-time
           -Wfloat-equal
           -fno-strict-aliasing
           -pipe
           -Wunused-const-variable
           -Wall
           -Wextra
           -fno-common
           -fvisibility=default
  )

  IF (CMAKE_COMPILER_IS_GNUCC)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -freg-struct-return
             -Wtrampolines
             -Wl,-z,relro,-z,now
             -fstack-protector-strong
             -fdata-sections
             -ffunction-sections
             -Wl,--gc-sections
             -Wmissing-format-attribute
             -Wstrict-overflow=2
             -Wswitch-default
             -Wconversion
             -Wunused
             -Wpointer-arith
    )

    IF (CMAKE_C_COMPILER_VERSION VERSION_GREATER "4.3.0")
      APPEND_TO_LISTS(
        LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS VALUES -Wlogical-op
      )
    ENDIF ()
    IF (CMAKE_C_COMPILER_VERSION VERSION_GREATER "6.0.0")
      APPEND_TO_LISTS(
        LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
        VALUES -Wduplicated-cond -Wnull-dereference
      )
    ENDIF ()
    IF (CMAKE_C_COMPILER_VERSION VERSION_GREATER "7.0.0")
      APPEND_TO_LISTS(
        LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
        VALUES -Wduplicated-branches -Wrestrict
      )
    ENDIF ()
  ENDIF ()

  IF (CMAKE_COMPILER_IS_CLANG)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -Wmissing-variable-declarations -Wcomma -Wused-but-marked-unused
    )
  ENDIF ()

  IF (SMALL_FOOTPRINT)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -Os -D__RELEASE__ -DNDEBUG
    )
  ELSE ()
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -O3 -D__RELEASE__ -DNDEBUG
    )
  ENDIF ()

  IF (CMAKE_COMPILER_IS_CLANG)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -Wnewline-eof -fcolor-diagnostics
    )
  ELSE ()
    # GCC (at least 4.8.4) has a bug where it'll find unreachable free()
    # calls and declare that the code is trying to free a stack pointer.
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -Wno-free-nonheap-object
    )
  ENDIF ()

  IF (CMAKE_COMPILER_IS_CLANG OR NOT "7.0.0" VERSION_GREATER
                                 CMAKE_C_COMPILER_VERSION
  )
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
      VALUES -Wimplicit-fallthrough
    )
  ENDIF ()

  LIST(APPEND CMAKE_C_FLAGS -Wmissing-prototypes -Wold-style-definition
       -Wstrict-prototypes
  )
  LIST(APPEND CMAKE_CXX_FLAGS -Wmissing-declarations) # -Weffc++

  # In GCC, -Wmissing-declarations is the C++ spelling of -Wmissing-prototypes and using the wrong one is an error.
  # In Clang, -Wmissing-prototypes is the spelling for both and -Wmissing-declarations is some other warning.
  #
  # https://gcc.gnu.org/onlinedocs/gcc-7.1.0/gcc/Warning-Options.html#Warning-Options
  # https://clang.llvm.org/docs/DiagnosticsReference.html#wmissing-prototypes
  # https://clang.llvm.org/docs/DiagnosticsReference.html#wmissing-declarations
  IF (CMAKE_COMPILER_IS_CLANG)
    LIST(APPEND CMAKE_CXX_FLAGS -Wmissing-prototypes)
  ENDIF ()

  IF (CMAKE_COMPILER_IS_GNUCXX AND "4.8" VERSION_GREATER
                                   CMAKE_C_COMPILER_VERSION
  )
    LIST(APPEND CMAKE_C_FLAGS -Wno-array-bounds)
  ENDIF ()
ENDIF ()

IF (SMALL_FOOTPRINT)
  IF (CMAKE_COMPILER_IS_CLANG)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS VALUES -flto=thin
    )
  ENDIF ()
  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
    VALUES --specs=nosys.specs --specs=nano.specs
  )
ENDIF ()

IF (CMAKE_COMPILER_IS_GNUCXX)
  LIST(APPEND CMAKE_C_FLAGS -Wc++-compat)
ENDIF ()

IF ((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_C_COMPILER_VERSION VERSION_GREATER
                                  "4.7.99") OR CMAKE_COMPILER_IS_CLANG
)
  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS VALUES -Wshadow
  )
ENDIF ()

IF (CMAKE_COMPILER_IS_GNUCC)
  IF (NOT APPLE)
    SET(CMAKE_SHARED_LINKER_FLAGS
        "-Wl,--fatal-warnings -Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}"
    )
    SET(CMAKE_MODULE_LINKER_FLAGS
        "-Wl,--fatal-warnings -Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}"
    )
  ENDIF ()
ENDIF ()

IF (FUZZ)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "You need to build with Clang for fuzzing to work")
  ENDIF ()

  IF (CMAKE_C_COMPILER_VERSION VERSION_LESS "6.0.0")
    MESSAGE(FATAL_ERROR "You need Clang ≥ 6.0.0")
  ENDIF ()

  ADD_DEFINITIONS(-DUNSAFE_DETERMINISTIC_MODE)
  SET(RUNNER_ARGS "-deterministic")

  IF (NOT NO_FUZZER_MODE)
    ADD_DEFINITIONS(-DUNSAFE_FUZZER_MODE)
    SET(RUNNER_ARGS ${RUNNER_ARGS} "-fuzzer" "-shim-config" "fuzzer_mode.json")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS
    VALUES -fsanitize=address,fuzzer-no-link
           -fsanitize-coverage=edge,indirect-calls
  )
ENDIF ()

# Enable position-independent code globally.
# True by default for SHARED and MODULE library targets and False otherwise
# This is needed because some library targets are OBJECT libraries.
SET(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

IF (MSAN)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "Cannot enable MSAN unless using Clang")
  ENDIF ()

  IF (ASAN)
    MESSAGE(FATAL_ERROR "ASAN and MSAN are mutually exclusive")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_ASM_FLAGS
    VALUES -fsanitize=memory -fsanitize-memory-track-origins
           -fno-omit-frame-pointer
  )
ENDIF ()

IF (ASAN)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "Cannot enable ASAN unless using Clang")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS
    VALUES -fsanitize=address -fsanitize-address-use-after-scope
           -fno-omit-frame-pointer
  )
ENDIF ()

IF (DEB)
  APPEND_TO_LISTS(LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS VALUES -z,noexecstack)
ENDIF ()

# ROP(Return-oriented Programming) Attack
IF (CFI)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "Cannot enable CFI unless using Clang")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS
    VALUES -fsanitize=cfi -fno-sanitize-trap=cfi -flto=thin
  )
  # We use Chromium's copy of clang, which requires -fuse-ld=lld if building with -flto.
  # That, in turn, can't handle -ggdb.
  LIST(APPEND CMAKE_EXE_LINKER_FLAGS -fuse-ld=lld)
  STRING(REPLACE "-ggdb" "-g" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  STRING(REPLACE "-ggdb" "-g" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  # -flto causes object files to contain LLVM bitcode.
  # Mixing those with assembly output in the same static library breaks the linker.
  SET(NO_ASM ON FORCE)
ENDIF ()

IF (TSAN)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "Cannot enable TSAN unless using Clang")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS
    VALUES -fsanitize=thread
  )
ENDIF ()

IF (UBSAN)
  IF (NOT CMAKE_COMPILER_IS_CLANG)
    MESSAGE(FATAL_ERROR "Cannot enable UBSAN unless using Clang")
  ENDIF ()

  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS
    VALUES -fsanitize=undefined -fsanitize=float-divide-by-zero
           -fsanitize=float-cast-overflow -fsanitize=integer
  )

  IF (NOT UBSAN_RECOVER)
    APPEND_TO_LISTS(
      LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS
      VALUES -fno-sanitize-recover=undefined
    )
  ENDIF ()
ENDIF ()

# Coverage
IF (GCOV)
  APPEND_TO_LISTS(
    LISTS CMAKE_C_FLAGS CMAKE_CXX_FLAGS VALUES -fprofile-arcs -ftest-coverage
  )

  # cmake-format: off
  ADD_CUSTOM_TARGET(
    gcov
    COMMAND ${CMAKE_COMMAND} -E make_directory report/coverage
    COMMAND ${CMAKE_MAKE_PROGRAM} test
    COMMAND echo "Coverage Report ..."
    COMMAND gcovr -r ${CMAKE_SOURCE_DIR} --html --html-details
            ${CMAKE_BINARY_DIR}/report/coverage/full.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
  # cmake-format: on
ENDIF ()

IF (AUTO_STANDARD)
  SET(C_STANDARDS -std=gnu18 -std=c18 -std=gnu11 -std=c11 -std=gnu99 -std=c99)
  INCLUDE(CheckCCompilerFlag)
  FOREACH (std ${C_STANDARDS})
    CHECK_C_COMPILER_FLAG(${std} supported_${std})
    IF (supported_${std})
      LIST(APPEND CMAKE_C_FLAGS ${std})
      BREAK()
    ENDIF ()
  ENDFOREACH ()

  SET(CXX_STANDARDS -std=gnu++2a -std=c++2a -std=gnu++1z -std=c++1z
                    -std=gnu++14 -std=c++14 -std=gnu++11 -std=c++11
  )
  INCLUDE(CheckCXXCompilerFlag)
  FOREACH (std ${CXX_STANDARDS})
    CHECK_CXX_COMPILER_FLAG(${std} supported_${std})
    IF (supported_${std})
      LIST(APPEND CMAKE_CXX_FLAGS ${std})
      BREAK()
    ENDIF ()
  ENDFOREACH ()
ELSE ()
  LIST(APPEND CMAKE_C_FLAGS -std=gnu11)
  LIST(APPEND CMAKE_CXX_FLAGS -std=gnu++14)
ENDIF ()

STRING(REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
STRING(REPLACE ";" " " CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")
STRING(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# redefine __FILE__ after stripping project dir
FUNCTION (TARGET_STRIP_PATH_PREFIX target)
  TARGET_COMPILE_OPTIONS(${target} PRIVATE -Wno-builtin-macro-redefined)
  # Get source files of target
  GET_TARGET_PROPERTY(source_files ${target} SOURCES)
  FOREACH (srcfile ${source_files})
    # Get compile definitions in source file
    GET_PROPERTY(defs SOURCE "${srcfile}" PROPERTY COMPILE_DEFINITIONS)
    # Get absolute path of source file
    GET_FILENAME_COMPONENT(filepath "${srcfile}" ABSOLUTE)
    # Trim leading dir
    STRING(FIND "${filepath}" "${CMAKE_BINARY_DIR}" pos)
    IF (${pos} EQUAL 0)
      FILE(RELATIVE_PATH relpath ${CMAKE_BINARY_DIR} ${filepath})
    ELSE ()
      FILE(RELATIVE_PATH relpath ${CMAKE_SOURCE_DIR} ${filepath})
    ENDIF ()
    # Add __FILE__ definition to compile definitions
    LIST(APPEND defs "__FILE__=\"${relpath}\"")
    # Set compile definitions to property
    SET_PROPERTY(SOURCE "${srcfile}" PROPERTY COMPILE_DEFINITIONS ${defs})
  ENDFOREACH ()
ENDFUNCTION ()

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
  # ENABLE_LANGUAGE(ASM_NASM)
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

IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  LINK_DIRECTORIES(/usr/local/lib)
  INCLUDE_DIRECTORIES(/usr/local/include)
ENDIF ()

IF (EXISTS ${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i/export)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i/export/lib)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/vendor/openssl-1.1.1i/export/include)
ENDIF ()

IF (BUILD_VENDOR)
  INCLUDE(ExternalProject)
  IF (NOT EXISTS ${CMAKE_SOURCE_DIR}/vendor)
    FILE(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor)
  ENDIF ()

  INCLUDE_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR}/include)
  LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)
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
    CONFIGURE_COMMAND ./config no-shared -d --prefix=${CMAKE_BINARY_DIR}
    BUILD_COMMAND make depend && make -j${CONCURRENCY}
    INSTALL_COMMAND make install_sw
    BUILD_IN_SOURCE 1
  )
  # cmake-format: on
ENDIF ()

SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS})
SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS})
SET(CMAKE_ASM_FLAGS_DEBUG ${CMAKE_ASM_FLAGS})
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS})
SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS})
SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS})
SET(CMAKE_ASM_FLAGS_RELEASE ${CMAKE_ASM_FLAGS})
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)

# mpi
ADD_SUBDIRECTORY(mpi)

# tests
ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)
