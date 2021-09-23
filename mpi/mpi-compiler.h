/**
 * Copyright 2021 Ethan.cr.yp.to
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * This is a single C/C++ header you can include in your project to enable
 * compiler-specific features while retaining compatibility with all compilers.
 * It contains dozens of macros to help make your code easier to use, harder to
 * misuse, safer, faster, and more portable.
 *
 * You can safely include this file in your public API, and it works with
 * virtually any C or C++ compiler
 *
 */

#ifdef COMPILER_VERSION_ENCODE
#undef COMPILER_VERSION_ENCODE
#endif
#define COMPILER_VERSION_ENCODE(major, minor, revision) (((major)*1000000) + ((minor)*1000) + (revision))

/** GNUC compiler */
#ifdef COMPILER_GNUC_VERSION
#undef COMPILER_GNUC_VERSION
#endif
#if defined(__GNUC__) && defined(__GNUC_PATCHLEVEL__)
#define COMPILER_GNUC_VERSION COMPILER_VERSION_ENCODE(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#elif defined(__GNUC__)
#define COMPILER_GNUC_VERSION COMPILER_VERSION_ENCODE(__GNUC__, __GNUC_MINOR__, 0)
#endif

#ifdef COMPILER_GNUC_VERSION_CHECK
#undef COMPILER_GNUC_VERSION_CHECK
#endif
#if defined(COMPILER_GNUC_VERSION)
#define COMPILER_GNUC_VERSION_CHECK(major, minor, patch) \
    (COMPILER_GNUC_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_GNUC_VERSION_CHECK(major, minor, patch) (0)
#endif

/** MSVC compiler */
#ifdef COMPILER_MSVC_VERSION
#undef COMPILER_MSVC_VERSION
#endif
#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 140000000) && !defined(__ICL)
#define COMPILER_MSVC_VERSION                                                              \
    COMPILER_VERSION_ENCODE(_MSC_FULL_VER / 10000000, (_MSC_FULL_VER % 10000000) / 100000, \
                            (_MSC_FULL_VER % 100000) / 100)
#elif defined(_MSC_FULL_VER) && !defined(__ICL)
#define COMPILER_MSVC_VERSION                                                           \
    COMPILER_VERSION_ENCODE(_MSC_FULL_VER / 1000000, (_MSC_FULL_VER % 1000000) / 10000, \
                            (_MSC_FULL_VER % 10000) / 10)
#elif defined(_MSC_VER) && !defined(__ICL)
#define COMPILER_MSVC_VERSION COMPILER_VERSION_ENCODE(_MSC_VER / 100, _MSC_VER % 100, 0)
#endif

#ifdef COMPILER_MSVC_VERSION_CHECK
#undef COMPILER_MSVC_VERSION_CHECK
#endif
#if !defined(COMPILER_MSVC_VERSION)
#define COMPILER_MSVC_VERSION_CHECK(major, minor, patch) (0)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#define COMPILER_MSVC_VERSION_CHECK(major, minor, patch) \
    (_MSC_FULL_VER >= ((major * 10000000) + (minor * 100000) + (patch)))
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
#define COMPILER_MSVC_VERSION_CHECK(major, minor, patch) \
    (_MSC_FULL_VER >= ((major * 1000000) + (minor * 10000) + (patch)))
#else
#define COMPILER_MSVC_VERSION_CHECK(major, minor, patch) (_MSC_VER >= ((major * 100) + (minor)))
#endif

/** INTEL compiler */
#ifdef COMPILER_INTEL_VERSION
#undef COMPILER_INTEL_VERSION
#endif
#if defined(__INTEL_COMPILER) && defined(__INTEL_COMPILER_UPDATE) && !defined(__ICL)
#define COMPILER_INTEL_VERSION \
    COMPILER_VERSION_ENCODE(__INTEL_COMPILER / 100, __INTEL_COMPILER % 100, __INTEL_COMPILER_UPDATE)
#elif defined(__INTEL_COMPILER) && !defined(__ICL)
#define COMPILER_INTEL_VERSION COMPILER_VERSION_ENCODE(__INTEL_COMPILER / 100, __INTEL_COMPILER % 100, 0)
#endif

#ifdef COMPILER_INTEL_VERSION_CHECK
#undef COMPILER_INTEL_VERSION_CHECK
#endif
#if defined(COMPILER_INTEL_VERSION)
#define COMPILER_INTEL_VERSION_CHECK(major, minor, patch) \
    (COMPILER_INTEL_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_INTEL_VERSION_CHECK(major, minor, patch) (0)
#endif

/** INTEL CL compiler */
#ifdef COMPILER_INTEL_CL_VERSION
#undef COMPILER_INTEL_CL_VERSION
#endif
#if defined(__INTEL_COMPILER) && defined(__INTEL_COMPILER_UPDATE) && defined(__ICL)
#define COMPILER_INTEL_CL_VERSION COMPILER_VERSION_ENCODE(__INTEL_COMPILER, __INTEL_COMPILER_UPDATE, 0)
#endif

#ifdef COMPILER_INTEL_CL_VERSION_CHECK
#undef COMPILER_INTEL_CL_VERSION_CHECK
#endif
#if defined(COMPILER_INTEL_CL_VERSION)
#define COMPILER_INTEL_CL_VERSION_CHECK(major, minor, patch) \
    (COMPILER_INTEL_CL_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_INTEL_CL_VERSION_CHECK(major, minor, patch) (0)
#endif

/** PGI compiler */
#ifdef COMPILER_PGI_VERSION
#undef COMPILER_PGI_VERSION
#endif
#if defined(__PGI) && defined(__PGIC__) && defined(__PGIC_MINOR__) && defined(__PGIC_PATCHLEVEL__)
#define COMPILER_PGI_VERSION COMPILER_VERSION_ENCODE(__PGIC__, __PGIC_MINOR__, __PGIC_PATCHLEVEL__)
#endif

#ifdef COMPILER_PGI_VERSION_CHECK
#undef COMPILER_PGI_VERSION_CHECK
#endif
#if defined(COMPILER_PGI_VERSION)
#define COMPILER_PGI_VERSION_CHECK(major, minor, patch) \
    (COMPILER_PGI_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_PGI_VERSION_CHECK(major, minor, patch) (0)
#endif

/** SUNPRO compiler */
#ifdef COMPILER_SUNPRO_VERSION
#undef COMPILER_SUNPRO_VERSION
#endif
#if defined(__SUNPRO_C) && (__SUNPRO_C > 0x1000)
#define COMPILER_SUNPRO_VERSION                                                             \
    COMPILER_VERSION_ENCODE((((__SUNPRO_C >> 16) & 0xf) * 10) + ((__SUNPRO_C >> 12) & 0xf), \
                            (((__SUNPRO_C >> 8) & 0xf) * 10) + ((__SUNPRO_C >> 4) & 0xf),   \
                            (__SUNPRO_C & 0xf) * 10)
#elif defined(__SUNPRO_C)
#define COMPILER_SUNPRO_VERSION \
    COMPILER_VERSION_ENCODE((__SUNPRO_C >> 8) & 0xf, (__SUNPRO_C >> 4) & 0xf, (__SUNPRO_C)&0xf)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x1000)
#define COMPILER_SUNPRO_VERSION                                                               \
    COMPILER_VERSION_ENCODE((((__SUNPRO_CC >> 16) & 0xf) * 10) + ((__SUNPRO_CC >> 12) & 0xf), \
                            (((__SUNPRO_CC >> 8) & 0xf) * 10) + ((__SUNPRO_CC >> 4) & 0xf),   \
                            (__SUNPRO_CC & 0xf) * 10)
#elif defined(__SUNPRO_CC)
#define COMPILER_SUNPRO_VERSION \
    COMPILER_VERSION_ENCODE((__SUNPRO_CC >> 8) & 0xf, (__SUNPRO_CC >> 4) & 0xf, (__SUNPRO_CC)&0xf)
#endif

#ifdef COMPILER_SUNPRO_VERSION_CHECK
#undef COMPILER_SUNPRO_VERSION_CHECK
#endif
#if defined(COMPILER_SUNPRO_VERSION)
#define COMPILER_SUNPRO_VERSION_CHECK(major, minor, patch) \
    (COMPILER_SUNPRO_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_SUNPRO_VERSION_CHECK(major, minor, patch) (0)
#endif

/** EMSCRIPTEN compiler */
#ifdef COMPILER_EMSCRIPTEN_VERSION
#undef COMPILER_EMSCRIPTEN_VERSION
#endif
#if defined(__EMSCRIPTEN__)
#define COMPILER_EMSCRIPTEN_VERSION \
    COMPILER_VERSION_ENCODE(__EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__)
#endif

#ifdef COMPILER_EMSCRIPTEN_VERSION_CHECK
#undef COMPILER_EMSCRIPTEN_VERSION_CHECK
#endif
#if defined(COMPILER_EMSCRIPTEN_VERSION)
#define COMPILER_EMSCRIPTEN_VERSION_CHECK(major, minor, patch) \
    (COMPILER_EMSCRIPTEN_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_EMSCRIPTEN_VERSION_CHECK(major, minor, patch) (0)
#endif

/** ARM compiler */
#ifdef COMPILER_ARM_VERSION
#undef COMPILER_ARM_VERSION
#endif
#if defined(__CC_ARM) && defined(__ARMCOMPILER_VERSION)
#define COMPILER_ARM_VERSION                                                                            \
    COMPILER_VERSION_ENCODE(__ARMCOMPILER_VERSION / 1000000, (__ARMCOMPILER_VERSION % 1000000) / 10000, \
                            (__ARMCOMPILER_VERSION % 10000) / 100)
#elif defined(__CC_ARM) && defined(__ARMCC_VERSION)
#define COMPILER_ARM_VERSION                                                                \
    COMPILER_VERSION_ENCODE(__ARMCC_VERSION / 1000000, (__ARMCC_VERSION % 1000000) / 10000, \
                            (__ARMCC_VERSION % 10000) / 100)
#endif

#ifdef COMPILER_ARM_VERSION_CHECK
#undef COMPILER_ARM_VERSION_CHECK
#endif
#if defined(COMPILER_ARM_VERSION)
#define COMPILER_ARM_VERSION_CHECK(major, minor, patch) \
    (COMPILER_ARM_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_ARM_VERSION_CHECK(major, minor, patch) (0)
#endif

/** IBM compiler */
#ifdef COMPILER_IBM_VERSION
#undef COMPILER_IBM_VERSION
#endif
#if defined(__ibmxl__)
#define COMPILER_IBM_VERSION \
    COMPILER_VERSION_ENCODE(__ibmxl_version__, __ibmxl_release__, __ibmxl_modification__)
#elif defined(__xlC__) && defined(__xlC_ver__)
#define COMPILER_IBM_VERSION \
    COMPILER_VERSION_ENCODE(__xlC__ >> 8, __xlC__ & 0xff, (__xlC_ver__ >> 8) & 0xff)
#elif defined(__xlC__)
#define COMPILER_IBM_VERSION COMPILER_VERSION_ENCODE(__xlC__ >> 8, __xlC__ & 0xff, 0)
#endif

#ifdef COMPILER_IBM_VERSION_CHECK
#undef COMPILER_IBM_VERSION_CHECK
#endif
#if defined(COMPILER_IBM_VERSION)
#define COMPILER_IBM_VERSION_CHECK(major, minor, patch) \
    (COMPILER_IBM_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_IBM_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI compiler */
#ifdef COMPILER_TI_VERSION
#undef COMPILER_TI_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) \
    && (defined(__TMS470__) || defined(__TI_ARM__) || defined(__MSP430__) || defined(__TMS320C2000__))
#if (__TI_COMPILER_VERSION__ >= 16000000)
#define COMPILER_TI_VERSION                                                                                \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif
#endif

#ifdef COMPILER_TI_VERSION_CHECK
#undef COMPILER_TI_VERSION_CHECK
#endif
#if defined(COMPILER_TI_VERSION)
#define COMPILER_TI_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_CL2000 compiler */
#ifdef COMPILER_TI_CL2000_VERSION
#undef COMPILER_TI_CL2000_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && defined(__TMS320C2000__)
#define COMPILER_TI_CL2000_VERSION                                                                         \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_CL2000_VERSION_CHECK
#undef COMPILER_TI_CL2000_VERSION_CHECK
#endif
#if defined(COMPILER_TI_CL2000_VERSION)
#define COMPILER_TI_CL2000_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_CL2000_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_CL2000_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_CL430 compiler */
#ifdef COMPILER_TI_CL430_VERSION
#undef COMPILER_TI_CL430_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && defined(__MSP430__)
#define COMPILER_TI_CL430_VERSION                                                                          \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_CL430_VERSION_CHECK
#undef COMPILER_TI_CL430_VERSION_CHECK
#endif
#if defined(COMPILER_TI_CL430_VERSION)
#define COMPILER_TI_CL430_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_CL430_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_CL430_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_ARMCL compiler */
#ifdef COMPILER_TI_ARMCL_VERSION
#undef COMPILER_TI_ARMCL_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && (defined(__TMS470__) || defined(__TI_ARM__))
#define COMPILER_TI_ARMCL_VERSION                                                                          \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_ARMCL_VERSION_CHECK
#undef COMPILER_TI_ARMCL_VERSION_CHECK
#endif
#if defined(COMPILER_TI_ARMCL_VERSION)
#define COMPILER_TI_ARMCL_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_ARMCL_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_ARMCL_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_CL6X compiler */
#ifdef COMPILER_TI_CL6X_VERSION
#undef COMPILER_TI_CL6X_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && defined(__TMS320C6X__)
#define COMPILER_TI_CL6X_VERSION                                                                           \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_CL6X_VERSION_CHECK
#undef COMPILER_TI_CL6X_VERSION_CHECK
#endif
#if defined(COMPILER_TI_CL6X_VERSION)
#define COMPILER_TI_CL6X_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_CL6X_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_CL6X_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_CL7X compiler */
#ifdef COMPILER_TI_CL7X_VERSION
#undef COMPILER_TI_CL7X_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && defined(__C7000__)
#define COMPILER_TI_CL7X_VERSION                                                                           \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_CL7X_VERSION_CHECK
#undef COMPILER_TI_CL7X_VERSION_CHECK
#endif
#if defined(COMPILER_TI_CL7X_VERSION)
#define COMPILER_TI_CL7X_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_CL7X_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_CL7X_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TI_CLPRU compiler */
#ifdef COMPILER_TI_CLPRU_VERSION
#undef COMPILER_TI_CLPRU_VERSION
#endif
#if defined(__TI_COMPILER_VERSION__) && defined(__PRU__)
#define COMPILER_TI_CLPRU_VERSION                                                                          \
    COMPILER_VERSION_ENCODE(__TI_COMPILER_VERSION__ / 1000000, (__TI_COMPILER_VERSION__ % 1000000) / 1000, \
                            (__TI_COMPILER_VERSION__ % 1000))
#endif

#ifdef COMPILER_TI_CLPRU_VERSION_CHECK
#undef COMPILER_TI_CLPRU_VERSION_CHECK
#endif
#if defined(COMPILER_TI_CLPRU_VERSION)
#define COMPILER_TI_CLPRU_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TI_CLPRU_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TI_CLPRU_VERSION_CHECK(major, minor, patch) (0)
#endif

/** CRAY compiler */
#ifdef COMPILER_CRAY_VERSION
#undef COMPILER_CRAY_VERSION
#endif
#if defined(_CRAYC)
#if defined(_RELEASE_PATCHLEVEL)
#define COMPILER_CRAY_VERSION COMPILER_VERSION_ENCODE(_RELEASE_MAJOR, _RELEASE_MINOR, _RELEASE_PATCHLEVEL)
#else
#define COMPILER_CRAY_VERSION COMPILER_VERSION_ENCODE(_RELEASE_MAJOR, _RELEASE_MINOR, 0)
#endif
#endif

#ifdef COMPILER_CRAY_VERSION_CHECK
#undef COMPILER_CRAY_VERSION_CHECK
#endif
#if defined(COMPILER_CRAY_VERSION)
#define COMPILER_CRAY_VERSION_CHECK(major, minor, patch) \
    (COMPILER_CRAY_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_CRAY_VERSION_CHECK(major, minor, patch) (0)
#endif

/** IAR compiler */
#ifdef COMPILER_IAR_VERSION
#undef COMPILER_IAR_VERSION
#endif
#if defined(__IAR_SYSTEMS_ICC__)
#if __VER__ > 1000
#define COMPILER_IAR_VERSION \
    COMPILER_VERSION_ENCODE((__VER__ / 1000000), ((__VER__ / 1000) % 1000), (__VER__ % 1000))
#else
#define COMPILER_IAR_VERSION COMPILER_VERSION_ENCODE(__VER__ / 100, __VER__ % 100, 0)
#endif
#endif

#ifdef COMPILER_IAR_VERSION_CHECK
#undef COMPILER_IAR_VERSION_CHECK
#endif
#if defined(COMPILER_IAR_VERSION)
#define COMPILER_IAR_VERSION_CHECK(major, minor, patch) \
    (COMPILER_IAR_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_IAR_VERSION_CHECK(major, minor, patch) (0)
#endif

/** TINYC compiler */
#ifdef COMPILER_TINYC_VERSION
#undef COMPILER_TINYC_VERSION
#endif
#if defined(__TINYC__)
#define COMPILER_TINYC_VERSION \
    COMPILER_VERSION_ENCODE(__TINYC__ / 1000, (__TINYC__ / 100) % 10, __TINYC__ % 100)
#endif

#ifdef COMPILER_TINYC_VERSION_CHECK
#undef COMPILER_TINYC_VERSION_CHECK
#endif
#if defined(COMPILER_TINYC_VERSION)
#define COMPILER_TINYC_VERSION_CHECK(major, minor, patch) \
    (COMPILER_TINYC_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_TINYC_VERSION_CHECK(major, minor, patch) (0)
#endif

/** DMC compiler */
#ifdef COMPILER_DMC_VERSION
#undef COMPILER_DMC_VERSION
#endif
#if defined(__DMC__)
#define COMPILER_DMC_VERSION COMPILER_VERSION_ENCODE(__DMC__ >> 8, (__DMC__ >> 4) & 0xf, __DMC__ & 0xf)
#endif

#ifdef COMPILER_DMC_VERSION_CHECK
#undef COMPILER_DMC_VERSION_CHECK
#endif
#if defined(COMPILER_DMC_VERSION)
#define COMPILER_DMC_VERSION_CHECK(major, minor, patch) \
    (COMPILER_DMC_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_DMC_VERSION_CHECK(major, minor, patch) (0)
#endif

/** COMPCERT compiler */
#ifdef COMPILER_COMPCERT_VERSION
#undef COMPILER_COMPCERT_VERSION
#endif
#if defined(__COMPCERT_VERSION__)
#define COMPILER_COMPCERT_VERSION                                                             \
    COMPILER_VERSION_ENCODE(__COMPCERT_VERSION__ / 10000, (__COMPCERT_VERSION__ / 100) % 100, \
                            __COMPCERT_VERSION__ % 100)
#endif

#ifdef COMPILER_COMPCERT_VERSION_CHECK
#undef COMPILER_COMPCERT_VERSION_CHECK
#endif
#if defined(COMPILER_COMPCERT_VERSION)
#define COMPILER_COMPCERT_VERSION_CHECK(major, minor, patch) \
    (COMPILER_COMPCERT_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_COMPCERT_VERSION_CHECK(major, minor, patch) (0)
#endif

/** PELLES compiler */
#ifdef COMPILER_PELLES_VERSION
#undef COMPILER_PELLES_VERSION
#endif
#if defined(__POCC__)
#define COMPILER_PELLES_VERSION COMPILER_VERSION_ENCODE(__POCC__ / 100, __POCC__ % 100, 0)
#endif

#ifdef COMPILER_PELLES_VERSION_CHECK
#undef COMPILER_PELLES_VERSION_CHECK
#endif
#if defined(COMPILER_PELLES_VERSION)
#define COMPILER_PELLES_VERSION_CHECK(major, minor, patch) \
    (COMPILER_PELLES_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_PELLES_VERSION_CHECK(major, minor, patch) (0)
#endif

/** MCST_LCC compiler */
#ifdef COMPILER_MCST_LCC_VERSION
#undef COMPILER_MCST_LCC_VERSION
#endif
#if defined(__LCC__) && defined(__LCC_MINOR__)
#define COMPILER_MCST_LCC_VERSION COMPILER_VERSION_ENCODE(__LCC__ / 100, __LCC__ % 100, __LCC_MINOR__)
#endif

#ifdef COMPILER_MCST_LCC_VERSION_CHECK
#undef COMPILER_MCST_LCC_VERSION_CHECK
#endif
#if defined(COMPILER_MCST_LCC_VERSION)
#define COMPILER_MCST_LCC_VERSION_CHECK(major, minor, patch) \
    (COMPILER_MCST_LCC_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_MCST_LCC_VERSION_CHECK(major, minor, patch) (0)
#endif

/** GCC compiler */
#ifdef COMPILER_GCC_VERSION
#undef COMPILER_GCC_VERSION
#endif
#if defined(COMPILER_GNUC_VERSION) && !defined(__clang__) && !defined(COMPILER_INTEL_VERSION)              \
    && !defined(COMPILER_PGI_VERSION) && !defined(COMPILER_ARM_VERSION) && !defined(COMPILER_CRAY_VERSION) \
    && !defined(COMPILER_TI_VERSION) && !defined(COMPILER_TI_ARMCL_VERSION)                                \
    && !defined(COMPILER_TI_CL430_VERSION) && !defined(COMPILER_TI_CL2000_VERSION)                         \
    && !defined(COMPILER_TI_CL6X_VERSION) && !defined(COMPILER_TI_CL7X_VERSION)                            \
    && !defined(COMPILER_TI_CLPRU_VERSION) && !defined(__COMPCERT__)                                       \
    && !defined(COMPILER_MCST_LCC_VERSION)
#define COMPILER_GCC_VERSION COMPILER_GNUC_VERSION
#endif

#ifdef COMPILER_GCC_VERSION_CHECK
#undef COMPILER_GCC_VERSION_CHECK
#endif
#if defined(COMPILER_GCC_VERSION)
#define COMPILER_GCC_VERSION_CHECK(major, minor, patch) \
    (COMPILER_GCC_VERSION >= COMPILER_VERSION_ENCODE(major, minor, patch))
#else
#define COMPILER_GCC_VERSION_CHECK(major, minor, patch) (0)
#endif



/** has attribute */
#ifdef COMPILER_HAS_ATTRIBUTE
#undef COMPILER_HAS_ATTRIBUTE
#endif
#if defined(__has_attribute) && ((!defined(COMPILER_IAR_VERSION) || COMPILER_IAR_VERSION_CHECK(8, 5, 9)))
#define COMPILER_HAS_ATTRIBUTE(attribute) __has_attribute(attribute)
#else
#define COMPILER_HAS_ATTRIBUTE(attribute) (0)
#endif

#ifdef COMPILER_GNUC_HAS_ATTRIBUTE
#undef COMPILER_GNUC_HAS_ATTRIBUTE
#endif
#if defined(__has_attribute)
#define COMPILER_GNUC_HAS_ATTRIBUTE(attribute, major, minor, patch) COMPILER_HAS_ATTRIBUTE(attribute)
#else
#define COMPILER_GNUC_HAS_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_ATTRIBUTE
#undef COMPILER_GCC_HAS_ATTRIBUTE
#endif
#if defined(__has_attribute)
#define COMPILER_GCC_HAS_ATTRIBUTE(attribute, major, minor, patch) COMPILER_HAS_ATTRIBUTE(attribute)
#else
#define COMPILER_GCC_HAS_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_HAS_CPP_ATTRIBUTE
#undef COMPILER_HAS_CPP_ATTRIBUTE
#endif
#if defined(__has_cpp_attribute) && defined(__cplusplus) \
    && (!defined(COMPILER_SUNPRO_VERSION) || COMPILER_SUNPRO_VERSION_CHECK(5, 15, 0))
#define COMPILER_HAS_CPP_ATTRIBUTE(attribute) __has_cpp_attribute(attribute)
#else
#define COMPILER_HAS_CPP_ATTRIBUTE(attribute) (0)
#endif

#ifdef COMPILER_HAS_CPP_ATTRIBUTE_NS
#undef COMPILER_HAS_CPP_ATTRIBUTE_NS
#endif
#if !defined(__cplusplus) || !defined(__has_cpp_attribute)
#define COMPILER_HAS_CPP_ATTRIBUTE_NS(ns, attribute) (0)
#elif !defined(COMPILER_PGI_VERSION) && !defined(COMPILER_IAR_VERSION)                \
    && (!defined(COMPILER_SUNPRO_VERSION) || COMPILER_SUNPRO_VERSION_CHECK(5, 15, 0)) \
    && (!defined(COMPILER_MSVC_VERSION) || COMPILER_MSVC_VERSION_CHECK(19, 20, 0))
#define COMPILER_HAS_CPP_ATTRIBUTE_NS(ns, attribute) COMPILER_HAS_CPP_ATTRIBUTE(ns::attribute)
#else
#define COMPILER_HAS_CPP_ATTRIBUTE_NS(ns, attribute) (0)
#endif

#ifdef COMPILER_GNUC_HAS_CPP_ATTRIBUTE
#undef COMPILER_GNUC_HAS_CPP_ATTRIBUTE
#endif
#if defined(__has_cpp_attribute) && defined(__cplusplus)
#define COMPILER_GNUC_HAS_CPP_ATTRIBUTE(attribute, major, minor, patch) __has_cpp_attribute(attribute)
#else
#define COMPILER_GNUC_HAS_CPP_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_CPP_ATTRIBUTE
#undef COMPILER_GCC_HAS_CPP_ATTRIBUTE
#endif
#if defined(__has_cpp_attribute) && defined(__cplusplus)
#define COMPILER_GCC_HAS_CPP_ATTRIBUTE(attribute, major, minor, patch) __has_cpp_attribute(attribute)
#else
#define COMPILER_GCC_HAS_CPP_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif


/** has builtin */
#ifdef COMPILER_HAS_BUILTIN
#undef COMPILER_HAS_BUILTIN
#endif
#if defined(__has_builtin)
#define COMPILER_HAS_BUILTIN(builtin) __has_builtin(builtin)
#else
#define COMPILER_HAS_BUILTIN(builtin) (0)
#endif

#ifdef COMPILER_GNUC_HAS_BUILTIN
#undef COMPILER_GNUC_HAS_BUILTIN
#endif
#if defined(__has_builtin)
#define COMPILER_GNUC_HAS_BUILTIN(builtin, major, minor, patch) __has_builtin(builtin)
#else
#define COMPILER_GNUC_HAS_BUILTIN(builtin, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_BUILTIN
#undef COMPILER_GCC_HAS_BUILTIN
#endif
#if defined(__has_builtin)
#define COMPILER_GCC_HAS_BUILTIN(builtin, major, minor, patch) __has_builtin(builtin)
#else
#define COMPILER_GCC_HAS_BUILTIN(builtin, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif


/** has feature */
#ifdef COMPILER_HAS_FEATURE
#undef COMPILER_HAS_FEATURE
#endif
#if defined(__has_feature)
#define COMPILER_HAS_FEATURE(feature) __has_feature(feature)
#else
#define COMPILER_HAS_FEATURE(feature) (0)
#endif

#ifdef COMPILER_GNUC_HAS_FEATURE
#undef COMPILER_GNUC_HAS_FEATURE
#endif
#if defined(__has_feature)
#define COMPILER_GNUC_HAS_FEATURE(feature, major, minor, patch) __has_feature(feature)
#else
#define COMPILER_GNUC_HAS_FEATURE(feature, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_FEATURE
#undef COMPILER_GCC_HAS_FEATURE
#endif
#if defined(__has_feature)
#define COMPILER_GCC_HAS_FEATURE(feature, major, minor, patch) __has_feature(feature)
#else
#define COMPILER_GCC_HAS_FEATURE(feature, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif

/** has extension */
#ifdef COMPILER_HAS_EXTENSION
#undef COMPILER_HAS_EXTENSION
#endif
#if defined(__has_extension)
#define COMPILER_HAS_EXTENSION(extension) __has_extension(extension)
#else
#define COMPILER_HAS_EXTENSION(extension) (0)
#endif

#ifdef COMPILER_GNUC_HAS_EXTENSION
#undef COMPILER_GNUC_HAS_EXTENSION
#endif
#if defined(__has_extension)
#define COMPILER_GNUC_HAS_EXTENSION(extension, major, minor, patch) __has_extension(extension)
#else
#define COMPILER_GNUC_HAS_EXTENSION(extension, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_EXTENSION
#undef COMPILER_GCC_HAS_EXTENSION
#endif
#if defined(__has_extension)
#define COMPILER_GCC_HAS_EXTENSION(extension, major, minor, patch) __has_extension(extension)
#else
#define COMPILER_GCC_HAS_EXTENSION(extension, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif

/** has declspec attribute */
#ifdef COMPILER_HAS_DECLSPEC_ATTRIBUTE
#undef COMPILER_HAS_DECLSPEC_ATTRIBUTE
#endif
#if defined(__has_declspec_attribute)
#define COMPILER_HAS_DECLSPEC_ATTRIBUTE(attribute) __has_declspec_attribute(attribute)
#else
#define COMPILER_HAS_DECLSPEC_ATTRIBUTE(attribute) (0)
#endif

#ifdef COMPILER_GNUC_HAS_DECLSPEC_ATTRIBUTE
#undef COMPILER_GNUC_HAS_DECLSPEC_ATTRIBUTE
#endif
#if defined(__has_declspec_attribute)
#define COMPILER_GNUC_HAS_DECLSPEC_ATTRIBUTE(attribute, major, minor, patch) \
    __has_declspec_attribute(attribute)
#else
#define COMPILER_GNUC_HAS_DECLSPEC_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_DECLSPEC_ATTRIBUTE
#undef COMPILER_GCC_HAS_DECLSPEC_ATTRIBUTE
#endif
#if defined(__has_declspec_attribute)
#define COMPILER_GCC_HAS_DECLSPEC_ATTRIBUTE(attribute, major, minor, patch) \
    __has_declspec_attribute(attribute)
#else
#define COMPILER_GCC_HAS_DECLSPEC_ATTRIBUTE(attribute, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif


/** has warning */
#ifdef COMPILER_HAS_WARNING
#undef COMPILER_HAS_WARNING
#endif
#if defined(__has_warning)
#define COMPILER_HAS_WARNING(warning) __has_warning(warning)
#else
#define COMPILER_HAS_WARNING(warning) (0)
#endif

#ifdef COMPILER_GNUC_HAS_WARNING
#undef COMPILER_GNUC_HAS_WARNING
#endif
#if defined(__has_warning)
#define COMPILER_GNUC_HAS_WARNING(warning, major, minor, patch) __has_warning(warning)
#else
#define COMPILER_GNUC_HAS_WARNING(warning, major, minor, patch) \
    COMPILER_GNUC_VERSION_CHECK(major, minor, patch)
#endif

#ifdef COMPILER_GCC_HAS_WARNING
#undef COMPILER_GCC_HAS_WARNING
#endif
#if defined(__has_warning)
#define COMPILER_GCC_HAS_WARNING(warning, major, minor, patch) __has_warning(warning)
#else
#define COMPILER_GCC_HAS_WARNING(warning, major, minor, patch) \
    COMPILER_GCC_VERSION_CHECK(major, minor, patch)
#endif

/** pragma */
#ifdef COMPILER_PRAGMA
#undef COMPILER_PRAGMA
#endif
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || defined(__clang__)      \
    || COMPILER_GCC_VERSION_CHECK(3, 0, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)        \
    || COMPILER_IAR_VERSION_CHECK(8, 0, 0) || COMPILER_PGI_VERSION_CHECK(18, 4, 0)          \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_TI_VERSION_CHECK(15, 12, 0)          \
    || COMPILER_TI_ARMCL_VERSION_CHECK(4, 7, 0) || COMPILER_TI_CL430_VERSION_CHECK(2, 0, 1) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 1, 0) || COMPILER_TI_CL6X_VERSION_CHECK(7, 0, 0) \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0) || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0)  \
    || COMPILER_CRAY_VERSION_CHECK(5, 0, 0) || COMPILER_TINYC_VERSION_CHECK(0, 9, 17)       \
    || COMPILER_SUNPRO_VERSION_CHECK(8, 0, 0)                                               \
    || (COMPILER_IBM_VERSION_CHECK(10, 1, 0) && defined(__C99_PRAGMA_OPERATOR))
#define COMPILER_PRAGMA(value) _Pragma(#value)
#elif COMPILER_MSVC_VERSION_CHECK(15, 0, 0)
#define COMPILER_PRAGMA(value) __pragma(value)
#else
#define COMPILER_PRAGMA(value)
#endif

#ifdef COMPILER_DIAGNOSTIC_PUSH
#undef COMPILER_DIAGNOSTIC_PUSH
#endif
#ifdef COMPILER_DIAGNOSTIC_POP
#undef COMPILER_DIAGNOSTIC_POP
#endif
#if defined(__clang__)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("clang diagnostic pop")
#elif COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("warning(push)")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("warning(pop)")
#elif COMPILER_GCC_VERSION_CHECK(4, 6, 0)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("GCC diagnostic pop")
#elif COMPILER_MSVC_VERSION_CHECK(15, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DIAGNOSTIC_PUSH __pragma(warning(push))
#define COMPILER_DIAGNOSTIC_POP  __pragma(warning(pop))
#elif COMPILER_ARM_VERSION_CHECK(5, 6, 0)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("push")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("pop")
#elif COMPILER_TI_VERSION_CHECK(15, 12, 0) || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)     \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 4, 0) || COMPILER_TI_CL6X_VERSION_CHECK(8, 1, 0) \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0) || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("diag_push")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("diag_pop")
#elif COMPILER_PELLES_VERSION_CHECK(2, 90, 0)
#define COMPILER_DIAGNOSTIC_PUSH _Pragma("warning(push)")
#define COMPILER_DIAGNOSTIC_POP  _Pragma("warning(pop)")
#else
#define COMPILER_DIAGNOSTIC_PUSH
#define COMPILER_DIAGNOSTIC_POP
#endif


// FIXME
/* COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_ is for
   HEDLEY INTERNAL USE ONLY.  API subject to change without notice. */
#if defined(__cplusplus)
#if COMPILER_HAS_WARNING("-Wc++98-compat")
#if COMPILER_HAS_WARNING("-Wc++17-extensions")
#if COMPILER_HAS_WARNING("-Wc++1z-extensions")
#define COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(xpr)        \
    COMPILER_DIAGNOSTIC_PUSH                                       \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"")         \
        _Pragma("clang diagnostic ignored \"-Wc++17-extensions\"") \
            _Pragma("clang diagnostic ignored \"-Wc++1z-extensions\"") xpr COMPILER_DIAGNOSTIC_POP
#else
#define COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(xpr) \
    COMPILER_DIAGNOSTIC_PUSH                                \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"")  \
        _Pragma("clang diagnostic ignored \"-Wc++17-extensions\"") xpr COMPILER_DIAGNOSTIC_POP
#endif
#else
#define COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(xpr) \
    COMPILER_DIAGNOSTIC_PUSH                                \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"") xpr COMPILER_DIAGNOSTIC_POP
#endif
#endif
#endif
#if !defined(COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_)
#define COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(x) x
#endif

/** const cast */
#ifdef COMPILER_CONST_CAST
#undef COMPILER_CONST_CAST
#endif
#if defined(__cplusplus)
#define COMPILER_CONST_CAST(T, expr) (const_cast<T>(expr))
#elif COMPILER_HAS_WARNING("-Wcast-qual") || COMPILER_GCC_VERSION_CHECK(4, 6, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_CONST_CAST(T, expr)                      \
    (__extension__({                                      \
        COMPILER_DIAGNOSTIC_PUSH                          \
        COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL((T)(expr)); \
        COMPILER_DIAGNOSTIC_POP                           \
    }))
#else
#define COMPILER_CONST_CAST(T, expr) ((T)(expr))
#endif

/** static cast */
#ifdef COMPILER_STATIC_CAST
#undef COMPILER_STATIC_CAST
#endif
#if defined(__cplusplus)
#define COMPILER_STATIC_CAST(T, expr) (static_cast<T>(expr))
#else
#define COMPILER_STATIC_CAST(T, expr) ((T)(expr))
#endif

/** reinterpret cast */
#ifdef COMPILER_REINTERPRET_CAST
#undef COMPILER_REINTERPRET_CAST
#endif
#if defined(__cplusplus)
#define COMPILER_REINTERPRET_CAST(T, expr) (reinterpret_cast<T>(expr))
#else
#define COMPILER_REINTERPRET_CAST(T, expr) ((T)(expr))
#endif

// FIXME
#if defined(__cplusplus)
#if COMPILER_HAS_WARNING("-Wold-style-cast")
#define COMPILER_CPP_CAST(T, expr) \
    COMPILER_DIAGNOSTIC_PUSH       \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")((T)(expr)) COMPILER_DIAGNOSTIC_POP
#elif COMPILER_IAR_VERSION_CHECK(8, 3, 0)
#define COMPILER_CPP_CAST(T, expr) \
    COMPILER_DIAGNOSTIC_PUSH       \
    _Pragma("diag_suppress=Pe137") COMPILER_DIAGNOSTIC_POP
#else
#define COMPILER_CPP_CAST(T, expr) ((T)(expr))
#endif
#else
#define COMPILER_CPP_CAST(T, expr) (expr)
#endif


/** diagnostic */
#ifdef COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED
#undef COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED
#endif
#if COMPILER_HAS_WARNING("-Wdeprecated-declarations")
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#elif COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("warning(disable:1478 1786)")
#elif COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED __pragma(warning(disable : 1478 1786))
#elif COMPILER_PGI_VERSION_CHECK(20, 7, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("diag_suppress 1215,1216,1444,1445")
#elif COMPILER_PGI_VERSION_CHECK(17, 10, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("diag_suppress 1215,1444")
#elif COMPILER_GCC_VERSION_CHECK(4, 3, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#elif COMPILER_MSVC_VERSION_CHECK(15, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED __pragma(warning(disable : 4996))
#elif COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("diag_suppress 1215,1444")
#elif COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                  \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("diag_suppress 1291,1718")
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 13, 0) && !defined(__cplusplus)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED \
    _Pragma("error_messages(off,E_DEPRECATED_ATT,E_DEPRECATED_ATT_MESS)")
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 13, 0) && defined(__cplusplus)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("error_messages(off,symdeprecated,symdeprecated2)")
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("diag_suppress=Pe1444,Pe1215")
#elif COMPILER_PELLES_VERSION_CHECK(2, 90, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED _Pragma("warn(disable:2241)")
#else
#define COMPILER_DIAGNOSTIC_DISABLE_DEPRECATED
#endif

#ifdef COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS
#undef COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS
#endif
#if COMPILER_HAS_WARNING("-Wunknown-pragmas")
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS \
    _Pragma("clang diagnostic ignored \"-Wunknown-pragmas\"")
#elif COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("warning(disable:161)")
#elif COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS __pragma(warning(disable : 161))
#elif COMPILER_PGI_VERSION_CHECK(17, 10, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("diag_suppress 1675")
#elif COMPILER_GCC_VERSION_CHECK(4, 3, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("GCC diagnostic ignored \"-Wunknown-pragmas\"")
#elif COMPILER_MSVC_VERSION_CHECK(15, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS __pragma(warning(disable : 4068))
#elif COMPILER_TI_VERSION_CHECK(16, 9, 0) || COMPILER_TI_CL6X_VERSION_CHECK(8, 0, 0) \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0) || COMPILER_TI_CLPRU_VERSION_CHECK(2, 3, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("diag_suppress 163")
#elif COMPILER_TI_CL6X_VERSION_CHECK(8, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("diag_suppress 163")
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("diag_suppress=Pe161")
#elif COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS _Pragma("diag_suppress 161")
#else
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS
#endif

#ifdef COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES
#undef COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES
#endif
#if COMPILER_HAS_WARNING("-Wunknown-attributes")
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES \
    _Pragma("clang diagnostic ignored \"-Wunknown-attributes\"")
#elif COMPILER_GCC_VERSION_CHECK(4, 6, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#elif COMPILER_INTEL_VERSION_CHECK(17, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("warning(disable:1292)")
#elif COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES __pragma(warning(disable : 1292))
#elif COMPILER_MSVC_VERSION_CHECK(19, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES __pragma(warning(disable : 5030))
#elif COMPILER_PGI_VERSION_CHECK(20, 7, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("diag_suppress 1097,1098")
#elif COMPILER_PGI_VERSION_CHECK(17, 10, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("diag_suppress 1097")
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 14, 0) && defined(__cplusplus)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("error_messages(off,attrskipunsup)")
#elif COMPILER_TI_VERSION_CHECK(18, 1, 0) || COMPILER_TI_CL6X_VERSION_CHECK(8, 3, 0) \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("diag_suppress 1173")
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("diag_suppress=Pe1097")
#elif COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES _Pragma("diag_suppress 1097")
#else
#define COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_CPP_ATTRIBUTES
#endif

#ifdef COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL
#undef COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL
#endif
#if COMPILER_HAS_WARNING("-Wcast-qual")
#define COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL _Pragma("clang diagnostic ignored \"-Wcast-qual\"")
#elif COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL _Pragma("warning(disable:2203 2331)")
#elif COMPILER_GCC_VERSION_CHECK(3, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#else
#define COMPILER_DIAGNOSTIC_DISABLE_CAST_QUAL
#endif

#ifdef COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION
#undef COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION
#endif
#if COMPILER_HAS_WARNING("-Wunused-function")
#define COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION \
    _Pragma("clang diagnostic ignored \"-Wunused-function\"")
#elif COMPILER_GCC_VERSION_CHECK(3, 4, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#elif COMPILER_MSVC_VERSION_CHECK(1, 0, 0)
#define COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION __pragma(warning(disable : 4505))
#elif COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION _Pragma("diag_suppress 3142")
#else
#define COMPILER_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION
#endif

/** deprecated */
#ifdef COMPILER_DEPRECATED
#undef COMPILER_DEPRECATED
#endif
#ifdef COMPILER_DEPRECATED_FOR
#undef COMPILER_DEPRECATED_FOR
#endif
#if COMPILER_MSVC_VERSION_CHECK(14, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DEPRECATED(since) __declspec(deprecated("Since " #since))
#define COMPILER_DEPRECATED_FOR(since, replacement) \
    __declspec(deprecated("Since " #since "; use " #replacement))
#elif (COMPILER_HAS_EXTENSION(attribute_deprecated_with_message) && !defined(COMPILER_IAR_VERSION)) \
    || COMPILER_GCC_VERSION_CHECK(4, 5, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)                \
    || COMPILER_ARM_VERSION_CHECK(5, 6, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 13, 0)               \
    || COMPILER_PGI_VERSION_CHECK(17, 10, 0) || COMPILER_TI_VERSION_CHECK(18, 1, 0)                 \
    || COMPILER_TI_ARMCL_VERSION_CHECK(18, 1, 0) || COMPILER_TI_CL6X_VERSION_CHECK(8, 3, 0)         \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0) || COMPILER_TI_CLPRU_VERSION_CHECK(2, 3, 0)          \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_DEPRECATED(since) __attribute__((__deprecated__("Since " #since)))
#define COMPILER_DEPRECATED_FOR(since, replacement) \
    __attribute__((__deprecated__("Since " #since "; use " #replacement)))
#elif defined(__cplusplus) && (__cplusplus >= 201402L)
#define COMPILER_DEPRECATED(since) \
    COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[deprecated("Since " #since)]])
#define COMPILER_DEPRECATED_FOR(since, replacement) \
    COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[deprecated("Since " #since "; use " #replacement)]])
#elif COMPILER_HAS_ATTRIBUTE(deprecated) || COMPILER_GCC_VERSION_CHECK(3, 1, 0)               \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_TI_VERSION_CHECK(15, 12, 0)            \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                               \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                              \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                               \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))     \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)     \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10) \
    || COMPILER_IAR_VERSION_CHECK(8, 10, 0)
#define COMPILER_DEPRECATED(since)                  __attribute__((__deprecated__))
#define COMPILER_DEPRECATED_FOR(since, replacement) __attribute__((__deprecated__))
#elif COMPILER_MSVC_VERSION_CHECK(13, 10, 0) || COMPILER_PELLES_VERSION_CHECK(6, 50, 0) \
    || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_DEPRECATED(since)                  __declspec(deprecated)
#define COMPILER_DEPRECATED_FOR(since, replacement) __declspec(deprecated)
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_DEPRECATED(since)                  _Pragma("deprecated")
#define COMPILER_DEPRECATED_FOR(since, replacement) _Pragma("deprecated")
#else
#define COMPILER_DEPRECATED(since)
#define COMPILER_DEPRECATED_FOR(since, replacement)
#endif

/** unavailable */
#ifdef COMPILER_UNAVAILABLE
#undef COMPILER_UNAVAILABLE
#endif
#if COMPILER_HAS_ATTRIBUTE(warning) || COMPILER_GCC_VERSION_CHECK(4, 3, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_UNAVAILABLE(available_since) \
    __attribute__((__warning__("Not available until " #available_since)))
#else
#define COMPILER_UNAVAILABLE(available_since)
#endif

/** warn: unused result */
#ifdef COMPILER_WARN_UNUSED_RESULT
#undef COMPILER_WARN_UNUSED_RESULT
#endif
#ifdef COMPILER_WARN_UNUSED_RESULT_MSG
#undef COMPILER_WARN_UNUSED_RESULT_MSG
#endif
#if COMPILER_HAS_ATTRIBUTE(warn_unused_result) || COMPILER_GCC_VERSION_CHECK(3, 4, 0)       \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_TI_VERSION_CHECK(15, 12, 0)       \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0)                                             \
    || (COMPILER_SUNPRO_VERSION_CHECK(5, 15, 0) && defined(__cplusplus))                    \
    || COMPILER_PGI_VERSION_CHECK(17, 10, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_WARN_UNUSED_RESULT          __attribute__((__warn_unused_result__))
#define COMPILER_WARN_UNUSED_RESULT_MSG(msg) __attribute__((__warn_unused_result__))
#elif (COMPILER_HAS_CPP_ATTRIBUTE(nodiscard) >= 201907L)
#define COMPILER_WARN_UNUSED_RESULT COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[nodiscard]])
#define COMPILER_WARN_UNUSED_RESULT_MSG(msg) \
    COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[nodiscard(msg)]])
#elif COMPILER_HAS_CPP_ATTRIBUTE(nodiscard)
#define COMPILER_WARN_UNUSED_RESULT          COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[nodiscard]])
#define COMPILER_WARN_UNUSED_RESULT_MSG(msg) COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[nodiscard]])
#elif defined(_Check_return_) /* SAL */
#define COMPILER_WARN_UNUSED_RESULT          _Check_return_
#define COMPILER_WARN_UNUSED_RESULT_MSG(msg) _Check_return_
#else
#define COMPILER_WARN_UNUSED_RESULT
#define COMPILER_WARN_UNUSED_RESULT_MSG(msg)
#endif

/** sentinel */
#ifdef COMPILER_SENTINEL
#undef COMPILER_SENTINEL
#endif
#if COMPILER_HAS_ATTRIBUTE(sentinel) || COMPILER_GCC_VERSION_CHECK(4, 0, 0)          \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_ARM_VERSION_CHECK(5, 4, 0) \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_SENTINEL(position) __attribute__((__sentinel__(position)))
#else
#define COMPILER_SENTINEL(position)
#endif

/** no return */
#ifdef COMPILER_NO_RETURN
#undef COMPILER_NO_RETURN
#endif
#if COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_NO_RETURN __noreturn
#elif COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_NO_RETURN __attribute__((__noreturn__))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define COMPILER_NO_RETURN _Noreturn
#elif defined(__cplusplus) && (__cplusplus >= 201103L)
#define COMPILER_NO_RETURN COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[noreturn]])
#elif COMPILER_HAS_ATTRIBUTE(noreturn) || COMPILER_GCC_VERSION_CHECK(3, 2, 0)               \
    || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0) || COMPILER_ARM_VERSION_CHECK(4, 1, 0)       \
    || COMPILER_IBM_VERSION_CHECK(10, 1, 0) || COMPILER_TI_VERSION_CHECK(15, 12, 0)         \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_IAR_VERSION_CHECK(8, 10, 0)
#define COMPILER_NO_RETURN __attribute__((__noreturn__))
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 10, 0)
#define COMPILER_NO_RETURN _Pragma("does_not_return")
#elif COMPILER_MSVC_VERSION_CHECK(13, 10, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_NO_RETURN __declspec(noreturn)
#elif COMPILER_TI_CL6X_VERSION_CHECK(6, 0, 0) && defined(__cplusplus)
#define COMPILER_NO_RETURN _Pragma("FUNC_NEVER_RETURNS;")
#elif COMPILER_COMPCERT_VERSION_CHECK(3, 2, 0)
#define COMPILER_NO_RETURN __attribute((noreturn))
#elif COMPILER_PELLES_VERSION_CHECK(9, 0, 0)
#define COMPILER_NO_RETURN __declspec(noreturn)
#else
#define COMPILER_NO_RETURN
#endif

/** no escape */
#ifdef COMPILER_NO_ESCAPE
#undef COMPILER_NO_ESCAPE
#endif
#if COMPILER_HAS_ATTRIBUTE(noescape)
#define COMPILER_NO_ESCAPE __attribute__((__noescape__))
#else
#define COMPILER_NO_ESCAPE
#endif

/** assume */
#ifdef COMPILER_ASSUME
#undef COMPILER_ASSUME
#endif
#if COMPILER_MSVC_VERSION_CHECK(13, 10, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) \
    || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_ASSUME(expr) __assume(expr)
#elif COMPILER_HAS_BUILTIN(__builtin_assume)
#define COMPILER_ASSUME(expr) __builtin_assume(expr)
#elif COMPILER_TI_CL2000_VERSION_CHECK(6, 2, 0) || COMPILER_TI_CL6X_VERSION_CHECK(4, 0, 0)
#if defined(__cplusplus)
#define COMPILER_ASSUME(expr) std::_nassert(expr)
#else
#define COMPILER_ASSUME(expr) _nassert(expr)
#endif
#endif

/** unreachable */
#ifdef COMPILER_UNREACHABLE
#undef COMPILER_UNREACHABLE
#endif
#ifdef COMPILER_UNREACHABLE_RETURN
#undef COMPILER_UNREACHABLE_RETURN
#endif
#if (COMPILER_HAS_BUILTIN(__builtin_unreachable) && (!defined(COMPILER_ARM_VERSION))) \
    || COMPILER_GCC_VERSION_CHECK(4, 5, 0) || COMPILER_PGI_VERSION_CHECK(18, 10, 0)   \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_IBM_VERSION_CHECK(13, 1, 5) \
    || COMPILER_CRAY_VERSION_CHECK(10, 0, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_UNREACHABLE() __builtin_unreachable()
#elif defined(COMPILER_ASSUME)
#define COMPILER_UNREACHABLE() COMPILER_ASSUME(0)
#endif
#if !defined(COMPILER_ASSUME)
#if defined(COMPILER_UNREACHABLE)
#define COMPILER_ASSUME(expr) COMPILER_STATIC_CAST(void, ((expr) ? 1 : (COMPILER_UNREACHABLE(), 1)))
#else
#define COMPILER_ASSUME(expr) COMPILER_STATIC_CAST(void, expr)
#endif
#endif
#if defined(COMPILER_UNREACHABLE)
#if COMPILER_TI_CL2000_VERSION_CHECK(6, 2, 0) || COMPILER_TI_CL6X_VERSION_CHECK(4, 0, 0)
#define COMPILER_UNREACHABLE_RETURN(value) return (COMPILER_STATIC_CAST(void, COMPILER_ASSUME(0)), (value))
#else
#define COMPILER_UNREACHABLE_RETURN(value) COMPILER_UNREACHABLE()
#endif
#else
#define COMPILER_UNREACHABLE_RETURN(value) return (value)
#endif
#if !defined(COMPILER_UNREACHABLE)
#define COMPILER_UNREACHABLE() COMPILER_ASSUME(0)
#endif

/** non null */
#ifdef COMPILER_NON_NULL
#undef COMPILER_NON_NULL
#endif
COMPILER_DIAGNOSTIC_PUSH
#if COMPILER_HAS_WARNING("-Wpedantic")
#pragma clang diagnostic ignored "-Wpedantic"
#endif
#if COMPILER_HAS_WARNING("-Wc++98-compat-pedantic") && defined(__cplusplus)
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif
#if COMPILER_GCC_HAS_WARNING("-Wvariadic-macros", 4, 0, 0)
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wvariadic-macros"
#elif defined(COMPILER_GCC_VERSION)
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif
#endif
#if COMPILER_HAS_ATTRIBUTE(nonnull) || COMPILER_GCC_VERSION_CHECK(3, 3, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_ARM_VERSION_CHECK(4, 1, 0)
#define COMPILER_NON_NULL(...) __attribute__((__nonnull__(__VA_ARGS__)))
#else
#define COMPILER_NON_NULL(...)
#endif
COMPILER_DIAGNOSTIC_POP

/** printf format */
#ifdef COMPILER_PRINTF_FORMAT
#undef COMPILER_PRINTF_FORMAT
#endif
#if defined(__MINGW32__) && COMPILER_GCC_HAS_ATTRIBUTE(format, 4, 4, 0) && !defined(__USE_MINGW_ANSI_STDIO)
#define COMPILER_PRINTF_FORMAT(string_idx, first_to_check) \
    __attribute__((__format__(ms_printf, string_idx, first_to_check)))
#elif defined(__MINGW32__) && COMPILER_GCC_HAS_ATTRIBUTE(format, 4, 4, 0) && defined(__USE_MINGW_ANSI_STDIO)
#define COMPILER_PRINTF_FORMAT(string_idx, first_to_check) \
    __attribute__((__format__(gnu_printf, string_idx, first_to_check)))
#elif COMPILER_HAS_ATTRIBUTE(format) || COMPILER_GCC_VERSION_CHECK(3, 1, 0)                 \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_ARM_VERSION_CHECK(5, 6, 0)        \
    || COMPILER_IBM_VERSION_CHECK(10, 1, 0) || COMPILER_TI_VERSION_CHECK(15, 12, 0)         \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_PRINTF_FORMAT(string_idx, first_to_check) \
    __attribute__((__format__(__printf__, string_idx, first_to_check)))
#elif COMPILER_PELLES_VERSION_CHECK(6, 0, 0)
#define COMPILER_PRINTF_FORMAT(string_idx, first_to_check) \
    __declspec(vaformat(printf, string_idx, first_to_check))
#else
#define COMPILER_PRINTF_FORMAT(string_idx, first_to_check)
#endif

/** constexpr */
#ifdef COMPILER_CONSTEXPR
#undef COMPILER_CONSTEXPR
#endif
#if defined(__cplusplus)
#if __cplusplus >= 201103L
#define COMPILER_CONSTEXPR COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(constexpr)
#endif
#endif
#if !defined(COMPILER_CONSTEXPR)
#define COMPILER_CONSTEXPR
#endif

/** branch predict */
#ifdef COMPILER_UNPREDICTABLE
#undef COMPILER_UNPREDICTABLE
#endif
#ifdef COMPILER_PREDICT
#undef COMPILER_PREDICT
#endif
#ifdef COMPILER_PREDICT_TRUE
#undef COMPILER_PREDICT_TRUE
#endif
#ifdef COMPILER_PREDICT_FALSE
#undef COMPILER_PREDICT_FALSE
#endif
#ifdef COMPILER_LIKELY
#undef COMPILER_LIKELY
#endif
#ifdef COMPILER_UNLIKELY
#undef COMPILER_UNLIKELY
#endif
#if COMPILER_HAS_BUILTIN(__builtin_unpredictable)
#define COMPILER_UNPREDICTABLE(expr) __builtin_unpredictable((expr))
#endif
#if (COMPILER_HAS_BUILTIN(__builtin_expect_with_probability) && !defined(COMPILER_PGI_VERSION)) \
    || COMPILER_GCC_VERSION_CHECK(9, 0, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_PREDICT(expr, value, probability) \
    __builtin_expect_with_probability((expr), (value), (probability))
#define COMPILER_PREDICT_TRUE(expr, probability) \
    __builtin_expect_with_probability(!!(expr), 1, (probability))
#define COMPILER_PREDICT_FALSE(expr, probability) \
    __builtin_expect_with_probability(!!(expr), 0, (probability))
#define COMPILER_LIKELY(expr)   __builtin_expect(!!(expr), 1)
#define COMPILER_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#elif (COMPILER_HAS_BUILTIN(__builtin_expect) && !defined(COMPILER_INTEL_CL_VERSION))        \
    || COMPILER_GCC_VERSION_CHECK(3, 0, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)         \
    || (COMPILER_SUNPRO_VERSION_CHECK(5, 15, 0) && defined(__cplusplus))                     \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)           \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0) || COMPILER_TI_ARMCL_VERSION_CHECK(4, 7, 0)      \
    || COMPILER_TI_CL430_VERSION_CHECK(3, 1, 0) || COMPILER_TI_CL2000_VERSION_CHECK(6, 1, 0) \
    || COMPILER_TI_CL6X_VERSION_CHECK(6, 1, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)    \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_TINYC_VERSION_CHECK(0, 9, 27)    \
    || COMPILER_CRAY_VERSION_CHECK(8, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_PREDICT(expr, expected, probability)              \
    (((probability) >= 0.9) ? __builtin_expect((expr), (expected)) \
                            : (COMPILER_STATIC_CAST(void, expected), (expr)))
#define COMPILER_PREDICT_TRUE(expr, probability)                                           \
    (__extension__({                                                                       \
        double hedley_probability_ = (probability);                                        \
        ((hedley_probability_ >= 0.9)                                                      \
             ? __builtin_expect(!!(expr), 1)                                               \
             : ((hedley_probability_ <= 0.1) ? __builtin_expect(!!(expr), 0) : !!(expr))); \
    }))
#define COMPILER_PREDICT_FALSE(expr, probability)                                          \
    (__extension__({                                                                       \
        double hedley_probability_ = (probability);                                        \
        ((hedley_probability_ >= 0.9)                                                      \
             ? __builtin_expect(!!(expr), 0)                                               \
             : ((hedley_probability_ <= 0.1) ? __builtin_expect(!!(expr), 1) : !!(expr))); \
    }))
#define COMPILER_LIKELY(expr)   __builtin_expect(!!(expr), 1)
#define COMPILER_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#define COMPILER_PREDICT(expr, expected, probability) (COMPILER_STATIC_CAST(void, expected), (expr))
#define COMPILER_PREDICT_TRUE(expr, probability)      (!!(expr))
#define COMPILER_PREDICT_FALSE(expr, probability)     (!!(expr))
#define COMPILER_LIKELY(expr)                         (!!(expr))
#define COMPILER_UNLIKELY(expr)                       (!!(expr))
#endif
#if !defined(COMPILER_UNPREDICTABLE)
#define COMPILER_UNPREDICTABLE(expr) COMPILER_PREDICT(expr, 1, 0.5)
#endif

/** malloc */
#ifdef COMPILER_MALLOC
#undef COMPILER_MALLOC
#endif
#if COMPILER_HAS_ATTRIBUTE(malloc) || COMPILER_GCC_VERSION_CHECK(3, 1, 0)                   \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0)    \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(12, 1, 0)          \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                 \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_MALLOC __attribute__((__malloc__))
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 10, 0)
#define COMPILER_MALLOC _Pragma("returns_new_memory")
#elif COMPILER_MSVC_VERSION_CHECK(14, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_MALLOC __declspec(restrict)
#else
#define COMPILER_MALLOC
#endif

/** pure */
#ifdef COMPILER_PURE
#undef COMPILER_PURE
#endif
#if COMPILER_HAS_ATTRIBUTE(pure) || COMPILER_GCC_VERSION_CHECK(2, 96, 0)                    \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0)    \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)          \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                 \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_PGI_VERSION_CHECK(17, 10, 0)    \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_PURE __attribute__((__pure__))
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 10, 0)
#define COMPILER_PURE _Pragma("does_not_write_global_data")
#elif defined(__cplusplus)                                                                  \
    && (COMPILER_TI_CL430_VERSION_CHECK(2, 0, 1) || COMPILER_TI_CL6X_VERSION_CHECK(4, 0, 0) \
        || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0))
#define COMPILER_PURE _Pragma("FUNC_IS_PURE;")
#else
#define COMPILER_PURE
#endif

/** const */
#ifdef COMPILER_CONST
#undef COMPILER_CONST
#endif
#if COMPILER_HAS_ATTRIBUTE(const) || COMPILER_GCC_VERSION_CHECK(2, 5, 0)                    \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0)    \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)          \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                 \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                             \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                            \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))  \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                             \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)   \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_PGI_VERSION_CHECK(17, 10, 0)    \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_CONST __attribute__((__const__))
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 10, 0)
#define COMPILER_CONST _Pragma("no_side_effect")
#else
#define COMPILER_CONST COMPILER_PURE
#endif

/** restrict */
#ifdef COMPILER_RESTRICT
#undef COMPILER_RESTRICT
#endif
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && !defined(__cplusplus)
#define COMPILER_RESTRICT restrict
#elif COMPILER_GCC_VERSION_CHECK(3, 1, 0) || COMPILER_MSVC_VERSION_CHECK(14, 0, 0)           \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0) \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)           \
    || COMPILER_PGI_VERSION_CHECK(17, 10, 0) || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)     \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 2, 4) || COMPILER_TI_CL6X_VERSION_CHECK(8, 1, 0)  \
    || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)                                               \
    || (COMPILER_SUNPRO_VERSION_CHECK(5, 14, 0) && defined(__cplusplus))                     \
    || COMPILER_IAR_VERSION_CHECK(8, 0, 0) || defined(__clang__)                             \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_RESTRICT __restrict
#elif COMPILER_SUNPRO_VERSION_CHECK(5, 3, 0) && !defined(__cplusplus)
#define COMPILER_RESTRICT _Restrict
#else
#define COMPILER_RESTRICT
#endif

/** inline */
#ifdef COMPILER_INLINE
#undef COMPILER_INLINE
#endif
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) \
    || (defined(__cplusplus) && (__cplusplus >= 199711L))
#define COMPILER_INLINE inline
#elif defined(COMPILER_GCC_VERSION) || COMPILER_ARM_VERSION_CHECK(6, 2, 0)
#define COMPILER_INLINE __inline__
#elif COMPILER_MSVC_VERSION_CHECK(12, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)   \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_TI_ARMCL_VERSION_CHECK(5, 1, 0)       \
    || COMPILER_TI_CL430_VERSION_CHECK(3, 1, 0) || COMPILER_TI_CL2000_VERSION_CHECK(6, 2, 0) \
    || COMPILER_TI_CL6X_VERSION_CHECK(8, 0, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)    \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_INLINE __inline
#else
#define COMPILER_INLINE
#endif

/** always inline */
#ifdef COMPILER_ALWAYS_INLINE
#undef COMPILER_ALWAYS_INLINE
#endif
#if COMPILER_HAS_ATTRIBUTE(always_inline) || COMPILER_GCC_VERSION_CHECK(4, 0, 0)              \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0)      \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)            \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                   \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                               \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                              \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                               \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))     \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)     \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10) \
    || COMPILER_IAR_VERSION_CHECK(8, 10, 0)
#define COMPILER_ALWAYS_INLINE __attribute__((__always_inline__)) COMPILER_INLINE
#elif COMPILER_MSVC_VERSION_CHECK(12, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_ALWAYS_INLINE __forceinline
#elif defined(__cplusplus)                                                                      \
    && (COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0) || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)    \
        || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0) || COMPILER_TI_CL6X_VERSION_CHECK(6, 1, 0) \
        || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0) || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0))
#define COMPILER_ALWAYS_INLINE _Pragma("FUNC_ALWAYS_INLINE;")
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_ALWAYS_INLINE _Pragma("inline=forced")
#else
#define COMPILER_ALWAYS_INLINE COMPILER_INLINE
#endif

/** never inline */
#ifdef COMPILER_NEVER_INLINE
#undef COMPILER_NEVER_INLINE
#endif
#if COMPILER_HAS_ATTRIBUTE(noinline) || COMPILER_GCC_VERSION_CHECK(4, 0, 0)                   \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0)      \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(10, 1, 0)            \
    || COMPILER_TI_VERSION_CHECK(15, 12, 0)                                                   \
    || (COMPILER_TI_ARMCL_VERSION_CHECK(4, 8, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_ARMCL_VERSION_CHECK(5, 2, 0)                                               \
    || (COMPILER_TI_CL2000_VERSION_CHECK(6, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))   \
    || COMPILER_TI_CL2000_VERSION_CHECK(6, 4, 0)                                              \
    || (COMPILER_TI_CL430_VERSION_CHECK(4, 0, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))    \
    || COMPILER_TI_CL430_VERSION_CHECK(4, 3, 0)                                               \
    || (COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__))     \
    || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0) || COMPILER_TI_CL7X_VERSION_CHECK(1, 2, 0)     \
    || COMPILER_TI_CLPRU_VERSION_CHECK(2, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10) \
    || COMPILER_IAR_VERSION_CHECK(8, 10, 0)
#define COMPILER_NEVER_INLINE __attribute__((__noinline__))
#elif COMPILER_MSVC_VERSION_CHECK(13, 10, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_NEVER_INLINE __declspec(noinline)
#elif COMPILER_PGI_VERSION_CHECK(10, 2, 0)
#define COMPILER_NEVER_INLINE _Pragma("noinline")
#elif COMPILER_TI_CL6X_VERSION_CHECK(6, 0, 0) && defined(__cplusplus)
#define COMPILER_NEVER_INLINE _Pragma("FUNC_CANNOT_INLINE;")
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_NEVER_INLINE _Pragma("inline=never")
#elif COMPILER_COMPCERT_VERSION_CHECK(3, 2, 0)
#define COMPILER_NEVER_INLINE __attribute((noinline))
#elif COMPILER_PELLES_VERSION_CHECK(9, 0, 0)
#define COMPILER_NEVER_INLINE __declspec(noinline)
#else
#define COMPILER_NEVER_INLINE
#endif

/** visibility */
#ifdef COMPILER_HIDDEN
#undef COMPILER_HIDDEN
#endif
#ifdef COMPILER_PUBLIC
#undef COMPILER_PUBLIC
#endif
#ifdef COMPILER_IMPORT
#undef COMPILER_IMPORT
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
#define COMPILER_HIDDEN
#define COMPILER_PUBLIC __declspec(dllexport)
#define COMPILER_IMPORT __declspec(dllimport)
#else
#if COMPILER_HAS_ATTRIBUTE(visibility) || COMPILER_GCC_VERSION_CHECK(3, 3, 0)                  \
    || COMPILER_SUNPRO_VERSION_CHECK(5, 11, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)       \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(13, 1, 0)             \
    || (defined(__TI_EABI__)                                                                   \
        && ((COMPILER_TI_CL6X_VERSION_CHECK(7, 2, 0) && defined(__TI_GNU_ATTRIBUTE_SUPPORT__)) \
            || COMPILER_TI_CL6X_VERSION_CHECK(7, 5, 0)))                                       \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_HIDDEN __attribute__((__visibility__("hidden")))
#define COMPILER_PUBLIC __attribute__((__visibility__("default")))
#else
#define COMPILER_HIDDEN
#define COMPILER_PUBLIC
#endif
#define COMPILER_IMPORT extern
#endif

/** no throw */
#ifdef COMPILER_NO_THROW
#undef COMPILER_NO_THROW
#endif
#if COMPILER_HAS_ATTRIBUTE(nothrow) || COMPILER_GCC_VERSION_CHECK(3, 3, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_NO_THROW __attribute__((__nothrow__))
#elif COMPILER_MSVC_VERSION_CHECK(13, 1, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0) \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0)
#define COMPILER_NO_THROW __declspec(nothrow)
#else
#define COMPILER_NO_THROW
#endif

/** fall-through */
#ifdef COMPILER_FALL_THROUGH
#undef COMPILER_FALL_THROUGH
#endif
#if COMPILER_HAS_ATTRIBUTE(fallthrough) || COMPILER_GCC_VERSION_CHECK(7, 0, 0) \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_FALL_THROUGH __attribute__((__fallthrough__))
#elif COMPILER_HAS_CPP_ATTRIBUTE_NS(clang, fallthrough)
#define COMPILER_FALL_THROUGH COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[clang::fallthrough]])
#elif COMPILER_HAS_CPP_ATTRIBUTE(fallthrough)
#define COMPILER_FALL_THROUGH COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_([[fallthrough]])
#elif defined(__fallthrough) /* SAL */
#define COMPILER_FALL_THROUGH __fallthrough
#else
#define COMPILER_FALL_THROUGH
#endif

/** returns_nonnull */
#ifdef COMPILER_RETURNS_NON_NULL
#undef COMPILER_RETURNS_NON_NULL
#endif
#if COMPILER_HAS_ATTRIBUTE(returns_nonnull) || COMPILER_GCC_VERSION_CHECK(4, 9, 0) \
    || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_RETURNS_NON_NULL __attribute__((__returns_nonnull__))
#elif defined(_Ret_notnull_) /* SAL */
#define COMPILER_RETURNS_NON_NULL _Ret_notnull_
#else
#define COMPILER_RETURNS_NON_NULL
#endif

// FIXME
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && !defined(__STDC_NO_VLA__) \
    && !defined(__cplusplus) && !defined(COMPILER_PGI_VERSION) && !defined(COMPILER_TINYC_VERSION)
#define COMPILER_ARRAY_PARAM(name) (name)
#else
#define COMPILER_ARRAY_PARAM(name)
#endif

/* COMPILER_IS_CONSTEXPR_ is for
   HEDLEY INTERNAL USE ONLY.  API subject to change without notice. */
#ifdef COMPILER_IS_CONSTEXPR_
#undef COMPILER_IS_CONSTEXPR_
#endif
#if COMPILER_HAS_BUILTIN(__builtin_constant_p) || COMPILER_GCC_VERSION_CHECK(3, 4, 0)   \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_TINYC_VERSION_CHECK(0, 9, 19) \
    || COMPILER_ARM_VERSION_CHECK(4, 1, 0) || COMPILER_IBM_VERSION_CHECK(13, 1, 0)      \
    || COMPILER_TI_CL6X_VERSION_CHECK(6, 1, 0)                                          \
    || (COMPILER_SUNPRO_VERSION_CHECK(5, 10, 0) && !defined(__cplusplus))               \
    || COMPILER_CRAY_VERSION_CHECK(8, 1, 0) || COMPILER_MCST_LCC_VERSION_CHECK(1, 25, 10)
#define COMPILER_IS_CONSTANT(expr) __builtin_constant_p(expr)
#endif
#if !defined(__cplusplus)
#if COMPILER_HAS_BUILTIN(__builtin_types_compatible_p) || COMPILER_GCC_VERSION_CHECK(3, 4, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0) || COMPILER_IBM_VERSION_CHECK(13, 1, 0)         \
    || COMPILER_CRAY_VERSION_CHECK(8, 1, 0) || COMPILER_ARM_VERSION_CHECK(5, 4, 0)            \
    || COMPILER_TINYC_VERSION_CHECK(0, 9, 24)
#if defined(__INTPTR_TYPE__)
#define COMPILER_IS_CONSTEXPR_(expr) \
    __builtin_types_compatible_p(__typeof__((1 ? (void *)((__INTPTR_TYPE__)((expr)*0)) : (int *)0)), int *)
#else
#include <stdint.h>
#define COMPILER_IS_CONSTEXPR_(expr) \
    __builtin_types_compatible_p(__typeof__((1 ? (void *)((intptr_t)((expr)*0)) : (int *)0)), int *)
#endif
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(COMPILER_SUNPRO_VERSION) \
       && !defined(COMPILER_PGI_VERSION) && !defined(COMPILER_IAR_VERSION))                            \
    || (COMPILER_HAS_EXTENSION(c_generic_selections) && !defined(COMPILER_IAR_VERSION))                \
    || COMPILER_GCC_VERSION_CHECK(4, 9, 0) || COMPILER_INTEL_VERSION_CHECK(17, 0, 0)                   \
    || COMPILER_IBM_VERSION_CHECK(12, 1, 0) || COMPILER_ARM_VERSION_CHECK(5, 3, 0)
#if defined(__INTPTR_TYPE__)
#define COMPILER_IS_CONSTEXPR_(expr) \
    _Generic((1 ? (void *)((__INTPTR_TYPE__)((expr)*0)) : (int *)0), int * : 1, void * : 0)
#else
#include <stdint.h>
#define COMPILER_IS_CONSTEXPR_(expr) \
    _Generic((1 ? (void *)((intptr_t)*0) : (int *)0), int * : 1, void * : 0)
#endif
#elif defined(COMPILER_GCC_VERSION) || defined(COMPILER_INTEL_VERSION) || defined(COMPILER_TINYC_VERSION) \
    || defined(COMPILER_TI_ARMCL_VERSION) || COMPILER_TI_CL430_VERSION_CHECK(18, 12, 0)                   \
    || defined(COMPILER_TI_CL2000_VERSION) || defined(COMPILER_TI_CL6X_VERSION)                           \
    || defined(COMPILER_TI_CL7X_VERSION) || defined(COMPILER_TI_CLPRU_VERSION) || defined(__clang__)
#define COMPILER_IS_CONSTEXPR_(expr) \
    (sizeof(void) != sizeof(*(1 ? ((void *)((expr)*0L)) : ((struct { char v[sizeof(void) * 2]; } *)1))))
#endif
#endif

#ifdef COMPILER_IS_CONSTANT
#undef COMPILER_IS_CONSTANT
#endif
#if defined(COMPILER_IS_CONSTEXPR_)
#if !defined(COMPILER_IS_CONSTANT)
#define COMPILER_IS_CONSTANT(expr) COMPILER_IS_CONSTEXPR_(expr)
#endif
#define COMPILER_REQUIRE_CONSTEXPR(expr) (COMPILER_IS_CONSTEXPR_(expr) ? (expr) : (-1))
#else
#if !defined(COMPILER_IS_CONSTANT)
#define COMPILER_IS_CONSTANT(expr) (0)
#endif
#define COMPILER_REQUIRE_CONSTEXPR(expr) (expr)
#endif

/** static assert */
#ifdef COMPILER_STATIC_ASSERT
#undef COMPILER_STATIC_ASSERT
#endif
#if !defined(__cplusplus)                                                                 \
    && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))                      \
        || (COMPILER_HAS_FEATURE(c_static_assert) && !defined(COMPILER_INTEL_CL_VERSION)) \
        || COMPILER_GCC_VERSION_CHECK(6, 0, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)  \
        || defined(_Static_assert))
#define COMPILER_STATIC_ASSERT(expr, message) _Static_assert(expr, message)
#elif (defined(__cplusplus) && (__cplusplus >= 201103L)) || COMPILER_MSVC_VERSION_CHECK(16, 0, 0) \
    || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_STATIC_ASSERT(expr, message) \
    COMPILER_DIAGNOSTIC_DISABLE_CPP98_COMPAT_WRAP_(static_assert(expr, message))
#else
#define COMPILER_STATIC_ASSERT(expr, message)
#endif

#ifdef COMPILER_MESSAGE
#undef COMPILER_MESSAGE
#endif
#if COMPILER_HAS_WARNING("-Wunknown-pragmas")
#define COMPILER_MESSAGE(msg)                   \
    COMPILER_DIAGNOSTIC_PUSH                    \
    COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS \
    COMPILER_PRAGMA(message msg)                \
    COMPILER_DIAGNOSTIC_POP
#elif COMPILER_GCC_VERSION_CHECK(4, 4, 0) || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_MESSAGE(msg) COMPILER_PRAGMA(message msg)
#elif COMPILER_CRAY_VERSION_CHECK(5, 0, 0)
#define COMPILER_MESSAGE(msg) COMPILER_PRAGMA(_CRI message msg)
#elif COMPILER_IAR_VERSION_CHECK(8, 0, 0)
#define COMPILER_MESSAGE(msg) COMPILER_PRAGMA(message(msg))
#elif COMPILER_PELLES_VERSION_CHECK(2, 0, 0)
#define COMPILER_MESSAGE(msg) COMPILER_PRAGMA(message(msg))
#else
#define COMPILER_MESSAGE(msg)
#endif

#ifdef COMPILER_WARNING
#undef COMPILER_WARNING
#endif
#if COMPILER_HAS_WARNING("-Wunknown-pragmas")
#define COMPILER_WARNING(msg)                   \
    COMPILER_DIAGNOSTIC_PUSH                    \
    COMPILER_DIAGNOSTIC_DISABLE_UNKNOWN_PRAGMAS \
    COMPILER_PRAGMA(clang warning msg)          \
    COMPILER_DIAGNOSTIC_POP
#elif COMPILER_GCC_VERSION_CHECK(4, 8, 0) || COMPILER_PGI_VERSION_CHECK(18, 4, 0) \
    || COMPILER_INTEL_VERSION_CHECK(13, 0, 0)
#define COMPILER_WARNING(msg) COMPILER_PRAGMA(GCC warning msg)
#elif COMPILER_MSVC_VERSION_CHECK(15, 0, 0) || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_WARNING(msg) COMPILER_PRAGMA(message(msg))
#else
#define COMPILER_WARNING(msg) COMPILER_MESSAGE(msg)
#endif

#if defined(COMPILER_REQUIRE)
#undef COMPILER_REQUIRE
#endif
#if defined(COMPILER_REQUIRE_MSG)
#undef COMPILER_REQUIRE_MSG
#endif
#if COMPILER_HAS_ATTRIBUTE(diagnose_if)
#if COMPILER_HAS_WARNING("-Wgcc-compat")
#define COMPILER_REQUIRE(expr)                           \
    COMPILER_DIAGNOSTIC_PUSH                             \
    _Pragma("clang diagnostic ignored \"-Wgcc-compat\"") \
        __attribute__((diagnose_if(!(expr), #expr, "error"))) COMPILER_DIAGNOSTIC_POP
#define COMPILER_REQUIRE_MSG(expr, msg)                  \
    COMPILER_DIAGNOSTIC_PUSH                             \
    _Pragma("clang diagnostic ignored \"-Wgcc-compat\"") \
        __attribute__((diagnose_if(!(expr), msg, "error"))) COMPILER_DIAGNOSTIC_POP
#else
#define COMPILER_REQUIRE(expr)          __attribute__((diagnose_if(!(expr), #expr, "error")))
#define COMPILER_REQUIRE_MSG(expr, msg) __attribute__((diagnose_if(!(expr), msg, "error")))
#endif
#else
#define COMPILER_REQUIRE(expr)
#define COMPILER_REQUIRE_MSG(expr, msg)
#endif

#if defined(COMPILER_FLAGS)
#undef COMPILER_FLAGS
#endif
#if COMPILER_HAS_ATTRIBUTE(flag_enum) \
    && (!defined(__cplusplus) || COMPILER_HAS_WARNING("-Wbitfield-enum-conversion"))
#define COMPILER_FLAGS __attribute__((__flag_enum__))
#else
#define COMPILER_FLAGS
#endif

#if defined(COMPILER_FLAGS_CAST)
#undef COMPILER_FLAGS_CAST
#endif
#if COMPILER_INTEL_VERSION_CHECK(19, 0, 0)
#define COMPILER_FLAGS_CAST(T, expr)                \
    (__extension__({                                \
        COMPILER_DIAGNOSTIC_PUSH                    \
        _Pragma("warning(disable:188)")((T)(expr)); \
        COMPILER_DIAGNOSTIC_POP                     \
    }))
#else
#define COMPILER_FLAGS_CAST(T, expr) COMPILER_STATIC_CAST(T, expr)
#endif

#if defined(COMPILER_EMPTY_BASES)
#undef COMPILER_EMPTY_BASES
#endif
#if (COMPILER_MSVC_VERSION_CHECK(19, 0, 23918) && !COMPILER_MSVC_VERSION_CHECK(20, 0, 0)) \
    || COMPILER_INTEL_CL_VERSION_CHECK(2021, 1, 0)
#define COMPILER_EMPTY_BASES __declspec(empty_bases)
#else
#define COMPILER_EMPTY_BASES
#endif
