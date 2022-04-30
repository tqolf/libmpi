MACRO (target_configure target)
  # libunwind: a portable and efficient C programming interface (API) to determine the call-chain of a program
  IF (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    FIND_PACKAGE(PkgConfig)
    IF (PkgConfig_FOUND)
      PKG_CHECK_MODULES(LIBUNWIND libunwind-generic)
      IF (LIBUNWIND_FOUND)
        TARGET_COMPILE_DEFINITIONS(${target} PRIVATE -DHAVE_LIBUNWIND)
      ELSE ()
        MESSAGE(STATUS "libunwind not found. Disabling unwind tests.")
      ENDIF ()
    ELSE ()
      MESSAGE(STATUS "pkgconfig not found. Disabling unwind tests.")
    ENDIF ()
  ENDIF ()

  # warnings
  IF (MSVC)
    TARGET_COMPILE_DEFINITIONS(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    TARGET_COMPILE_OPTIONS(${target} PRIVATE /Oy /W3)
  ELSE ()
    # Note clang-cl is odd and sets both CLANG and MSVC.
    # We base our configuration primarily on our normal Clang one.
    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANGUAGE:C,CXX,ASM>:-Wformat=2
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
              -fvisibility=default>
    )

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE
        $<$<AND:$<COMPILE_LANGUAGE:C,CXX,ASM>,$<CXX_COMPILER_ID:GNUCC,GNUCXX>>:-freg-struct-return
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
        -Wpointer-arith>
    )

    IF (CMAKE_COMPILER_IS_GNUCC)
      TARGET_COMPILE_OPTIONS(
        ${target}
        PRIVATE
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,4.3.0>:-Wlogical-op>
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,4.8.0>:-Wno-array-bounds>
          # GCC (at least 4.8.4) has a bug where it'll find unreachable free()
          # calls and declare that the code is trying to free a stack pointer.
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,4.8.4>:-Wno-free-nonheap-object>
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,6.0.0>:-Wduplicated-cond
          -Wnull-dereference>
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,7.0.0>:-Wduplicated-branches
          -Wrestrict>
      )

      # shared or module
      TARGET_LINK_OPTIONS(
        ${target} PRIVATE $<$<NOT:$<BOOL:${APPLE}>>:-Wl,--fatal-warnings
        -Wl,--no-undefined>
      )
    ENDIF ()

    IF (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
      TARGET_COMPILE_OPTIONS(
        ${target}
        PRIVATE $<$<COMPILE_LANGUAGE:C,CXX,ASM>:-Wmissing-variable-declarations
                -Wcomma -Wused-but-marked-unused -Wnewline-eof
                -fcolor-diagnostics>
      )
      TARGET_COMPILE_OPTIONS(
        ${target}
        PRIVATE
          $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,7.0.0>:-Wimplicit-fallthrough>
      )
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang,GNUCC,GNUCXX>:
              -Wmissing-prototypes -Wold-style-definition -Wstrict-prototypes>
    )
    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNUCC,GNUCXX>:-Wmissing-declarations
        -Weffc++>
    )

    # In GCC, -Wmissing-declarations is the C++ spelling of -Wmissing-prototypes and using the wrong one is an error.
    # In Clang, -Wmissing-prototypes is the spelling for both and -Wmissing-declarations is some other warning.
    #
    # https://gcc.gnu.org/onlinedocs/gcc-7.1.0/gcc/Warning-Options.html#Warning-Options
    # https://clang.llvm.org/docs/DiagnosticsReference.html#wmissing-prototypes
    # https://clang.llvm.org/docs/DiagnosticsReference.html#wmissing-declarations
    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wmissing-prototypes>
    )

    IF (SMALL_FOOTPRINT)
      TARGET_COMPILE_OPTIONS(
        ${target}
        PRIVATE -Os
                $<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang>:-flto=thin>
                $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-flto=thin>
                $<$<COMPILE_LANG_AND_ID:ASM,AppleClang,Clang>:-flto=thin>
                $<$<COMPILE_LANG_AND_ID:C,GNUCC,GNUCXX>:--specs=nosys.specs
                --specs=nano.specs>
                $<$<COMPILE_LANG_AND_ID:CXX,GNUCC,GNUCXX>:--specs=nosys.specs
                --specs=nano.specs>
                $<$<COMPILE_LANG_AND_ID:ASM,GNUCC,GNUCXX>:--specs=nosys.specs
                --specs=nano.specs>
      )
    ENDIF ()
    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<IF:$<CONFIG:Debug>,-O0 -g3,-O2 -g>
    )

    TARGET_COMPILE_DEFINITIONS(
      ${target} PRIVATE $<IF:$<CONFIG:Debug>,__DEBUG__,__RELEASE__ NDEBUG>
    )

    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<$<COMPILE_LANG_AND_ID:C,GNUCC>:-Wc++-compat>
    )
    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,4.7.99>:-Wshadow>
              $<$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.7.99>:-Wshadow>
              # $<$<VERSION_GREATER:$<ASM_COMPILER_VERSION>,4.7.99>:-Wshadow>
    )
  ENDIF ()

  # Enable position-independent code globally.
  # True by default for SHARED and MODULE library targets and False otherwise
  # This is needed because some library targets are OBJECT libraries.
  SET_PROPERTY(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)

  TARGET_COMPILE_OPTIONS(${target} PRIVATE -fomit-frame-pointer)

  IF (FUZZ)
    IF (NOT CMAKE_COMPILER_IS_CLANG)
      MESSAGE(FATAL_ERROR "You need to build with Clang for fuzzing to work")
    ENDIF ()

    IF (CMAKE_C_COMPILER_VERSION VERSION_LESS "6.0.0")
      MESSAGE(FATAL_ERROR "You need Clang ≥ 6.0.0")
    ENDIF ()

    TARGET_COMPILE_DEFINITIONS(${target} PRIVATE -DUNSAFE_DETERMINISTIC_MODE)
    SET(RUNNER_ARGS "-deterministic")

    IF (NOT NO_FUZZER_MODE)
      TARGET_COMPILE_DEFINITIONS(${target} PRIVATE -DUNSAFE_FUZZER_MODE)
      SET(RUNNER_ARGS ${RUNNER_ARGS} "-fuzzer" "-shim-config"
                      "fuzzer_mode.json"
      )
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,fuzzer-no-link
              -fsanitize-coverage=edge,indirect-calls>
    )
  ENDIF ()

  IF (MSAN)
    IF (NOT CMAKE_COMPILER_IS_CLANG)
      MESSAGE(FATAL_ERROR "Cannot enable MSAN unless using Clang")
    ENDIF ()

    IF (ASAN)
      MESSAGE(FATAL_ERROR "ASAN and MSAN are mutually exclusive")
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=memory
              -fsanitize-memory-track-origins -fno-omit-frame-pointer>
    )
  ENDIF ()

  IF (ASAN)
    IF (NOT CMAKE_COMPILER_IS_CLANG)
      MESSAGE(FATAL_ERROR "Cannot enable ASAN unless using Clang")
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address
              -fsanitize-address-use-after-scope -fno-omit-frame-pointer>
    )
  ENDIF ()

  IF (DEB)
    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<$<COMPILE_LANG:C,CXX>:-z,noexecstack>
    )
  ENDIF ()

  # ROP(Return-oriented Programming) Attack
  IF (CFI)
    IF (NOT CMAKE_COMPILER_IS_CLANG)
      MESSAGE(FATAL_ERROR "Cannot enable CFI unless using Clang")
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=cfi
                        -fno-sanitize-trap=cfi -flto=thin>
    )

    # We use Chromium's copy of clang, which requires -fuse-ld=lld if building with -flto.
    # That, in turn, can't handle -ggdb.
    TARGET_LINK_OPTIONS(
      $<$<STREQUAL:$<TARGET_PROPERTY:${target},TYPE>,"EXECUTABLE">:-fuse-ld=lld>
    )
    # FIXME
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

    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=thread>
    )

    TARGET_LINK_OPTIONS(
      $<$<STREQUAL:$<TARGET_PROPERTY:${target},TYPE>,"EXECUTABLE">:-fsanitize=thread>
    )
  ENDIF ()

  IF (UBSAN)
    IF (NOT CMAKE_COMPILER_IS_CLANG)
      MESSAGE(FATAL_ERROR "Cannot enable UBSAN unless using Clang")
    ENDIF ()

    TARGET_COMPILE_OPTIONS(
      ${target}
      PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=undefined
              -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow
              -fsanitize=integer>
    )

    TARGET_LINK_OPTIONS(
      $<$<STREQUAL:$<TARGET_PROPERTY:${target},TYPE>,"EXECUTABLE">:-fsanitize=undefined
      -fsanitize=float-divide-by-zero
      -fsanitize=float-cast-overflow
      -fsanitize=integer>
    )

    IF (NOT UBSAN_RECOVER)
      TARGET_COMPILE_OPTIONS(
        ${target}
        PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fno-sanitize-recover=undefined>
      )

      TARGET_LINK_OPTIONS(
        $<$<STREQUAL:$<TARGET_PROPERTY:${target},TYPE>,"EXECUTABLE">:
        -fno-sanitize-recover=undefined>
      )
    ENDIF ()
  ENDIF ()

  # Coverage
  IF (GCOV)
    TARGET_COMPILE_OPTIONS(
      ${target} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fprofile-arcs
                        -ftest-coverage>
    )
  ENDIF ()

  IF (AUTO_STANDARD)
    SET(C_STANDARDS -std=gnu18 -std=c18 -std=gnu11 -std=c11 -std=gnu99 -std=c99)
    INCLUDE(CheckCCompilerFlag)
    FOREACH (std ${C_STANDARDS})
      CHECK_C_COMPILER_FLAG(${std} supported_${std})
      IF (supported_${std})
        TARGET_COMPILE_OPTIONS(
          ${target}
          PUBLIC
            $<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang,GNUCC,GNUCXX>:${std}>
        )
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
        TARGET_COMPILE_OPTIONS(
          ${target}
          PUBLIC
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNUCC,GNUCXX>:${std}>
        )
        BREAK()
      ENDIF ()
    ENDFOREACH ()
  ELSE ()
    TARGET_COMPILE_OPTIONS(
      ${target}
      PUBLIC
        $<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang,GNUCC,GNUCXX>:-std=gnu11>
        $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNUCC,GNUCXX>:-std=gnu++14>
    )
  ENDIF ()

  IF (LINK_PTHREAD)
    FIND_PACKAGE(Threads)
    IF (Threads_FOUND)
      TARGET_LINK_LIBRARIES(
        ${target} PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
      )
    ENDIF ()
  ENDIF ()

  # redefine __FILE__ after stripping project dir
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
ENDMACRO ()

# Coverage
IF (GCOV)
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
