# Multiple-Precision-Natural-Number

CONFIGURE_FILE(mpn-asm.h ${CMAKE_BINARY_DIR}/include/mpn/mpn-asm.h COPYONLY)
CONFIGURE_FILE(mpn-conf.h ${CMAKE_BINARY_DIR}/include/mpn/mpn-conf.h COPYONLY)
CONFIGURE_FILE(
  mpn-binary.h ${CMAKE_BINARY_DIR}/include/mpn/mpn-binary.h COPYONLY
)
CONFIGURE_FILE(
  mpn-optimizer.h ${CMAKE_BINARY_DIR}/include/mpn/mpn-optimizer.h COPYONLY
)
CONFIGURE_FILE(
  mpn-montgomery.h ${CMAKE_BINARY_DIR}/include/mpn/mpn-montgomery.h COPYONLY
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/include/mpn/mpn-conf.h
              ${CMAKE_BINARY_DIR}/include/mpn/mpn-optimizer.h
              ${CMAKE_BINARY_DIR}/include/mpn/mpn-binary.h
              ${CMAKE_BINARY_DIR}/include/mpn/mpn-montgomery.h
        DESTINATION include/mpn
)

ADD_LIBRARY(mpn mpn-binary.c mpn-asm.c mpn-optimizer.c mpn-montgomery.c)
TARGET_CONFIGURE(mpn)
INSTALL(TARGETS mpn ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

OPTION(MPN_NO_ASM "disable asm for mpn" OFF)
IF ((NOT MPN_NO_ASM) AND (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  ENABLE_LANGUAGE(ASM_NASM)
  IF (NOT DEFINED ARCH)
    SET(ARCH ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF ()
  SET(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -D_L9 -DLINUX32E")
  IF (${ARCH} STREQUAL "x86_64")
    FILE(GLOB ASM_SOURCE asm/intel64/*.asm)
    TARGET_SOURCES(mpn PRIVATE ${ASM_SOURCE})
    TARGET_INCLUDE_DIRECTORIES(mpn PRIVATE asm asm/intel64)
    TARGET_COMPILE_DEFINITIONS(
      mpn
      PRIVATE -DMPN_UADD_VECTORIZED_ASM
              -DMPN_USUB_VECTORIZED_ASM
              -DMPN_UINC_VECTORIZED_ASM
              -DMPN_UDEC_VECTORIZED_ASM
              -DMPN_UDIV_ASM
              -DMPN_UMUL_ASM
              -DMPN_USQR_ASM
              -DMPN_UMUL_ADD_ASM
              -DMPN_MONT_REDC_ASM
    )
  ENDIF ()
ENDIF ()

IF (MPN_NO_INLINE_ASM)
  TARGET_COMPILE_DEFINITIONS(mpn PRIVATE -DMPN_NO_INLINE_ASM)
ENDIF()
