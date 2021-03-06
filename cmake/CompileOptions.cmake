# ============================================================================ #
#                                Compiler setup                                #
# ============================================================================ #

# Get upper case system name
string(TOUPPER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_UPPER)

# Determine architecture (32/64 bit)
set(X64 OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 ON)
endif()

# Determine if we're building a shared (dynamic) library
# And set appropriate suffixes for the executables
if(BUILD_SHARED_LIBS)
    set(CMAKE_DEBUG_POSTFIX "_d")
else()
    add_definitions(-DSTATIC_LIB)
    set(CMAKE_DEBUG_POSTFIX "_d_s")
    set(CMAKE_RELEASE_POSTFIX "_s")
endif()

# Check if compiler support the c++14 standard
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else(COMPILER_SUPPORTS_CXX14)
    message(STATUS "The compiler '${CMAKE_CXX_COMPILER}' has no C++14 support. Please use a more recent C++ compiler!")
endif()

# Check compiler version
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # c++14 require at least gcc 5.0.0
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0.0)
        message(FATAL_ERROR "GCC version must be at least 5.0.0!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # c++14 require at least clang 3.4
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
        message(FATAL_ERROR "Clang version must be at least 3.4!")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Set optional build flags
set(EXTRA_FLAGS "")

# Increase max. template depth on GCC and Clang
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"
   OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  # Increase maximum number of instantiations and
  # reduce the number of template instantiations shown in backtrace.
  set(EXTRA_FLAGS "-ftemplate-depth=512 -ftemplate-backtrace-limit=0")
endif()

# GCC compiles large C++ programs faster if you increase its GC limit
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(EXTRA_FLAGS "${EXTRA_FLAGS} --param ggc-min-heapsize=524288")
endif()

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(WFLAGS ${WFLAGS}
        _SCL_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the Standard C++ Library
        _CRT_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the CRT Library
    )
    set(WFLAGS ${WFLAGS}
        /MP           # -> build with multiple processes
        /W4           # -> warning level 4
        # /WX         # -> treat warnings as errors

        /wd4251       # -> disable warning: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
        /wd4592       # -> disable warning: 'identifier': symbol will be dynamically initialized (implementation limitation)
        # /wd4201     # -> disable warning: nonstandard extension used: nameless struct/union (caused by GLM)
        # /wd4127     # -> disable warning: conditional expression is constant (caused by Qt)
        
        #$<$<CONFIG:Debug>:
        #/RTCc         # -> value is assigned to a smaller data type and results in a data loss
        #>

        $<$<CONFIG:Release>: 
        /Gw           # -> whole program global optimization
        /GS-          # -> buffer security check: no 
        /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
        /GF           # -> enable string pooling
        >
        
        # No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)
    )
endif ()

# Add "-Werror" flag if requested
if(OPTION_CXX_WARNINGS_AS_ERRORS)
  set(EXTRA_FLAGS "${EXTRA_FLAGS} -Werror")
endif()

# Set -fno-exception if requested
if(OPTION_FORCE_NO_EXCEPTIONS)
  set(EXTRA_FLAGS "${EXTRA_FLAGS} -fno-exceptions")
endif()

# Enable a ton of warnings if --more-clang-warnings is used
if(OPTION_MORE_WARNINGS)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(WFLAGS "-Weverything -Wno-c++98-compat -Wno-padded "
               "-Wno-documentation-unknown-command -Wno-exit-time-destructors "
               "-Wno-global-constructors -Wno-missing-prototypes "
               "-Wno-c++98-compat-pedantic -Wno-unused-member-function "
               "-Wno-unused-const-variable -Wno-switch-enum "
               "-Wno-missing-noreturn -Wno-covered-switch-default")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(WFLAGS "-Waddress -Wall -Warray-bounds "
               "-Wattributes -Wbuiltin-macro-redefined -Wcast-align "
               "-Wcast-qual -Wchar-subscripts -Wclobbered -Wcomment "
               "-Wconversion -Wconversion-null -Wcoverage-mismatch "
               "-Wcpp -Wdelete-non-virtual-dtor -Wdeprecated "
               "-Wdeprecated-declarations -Wdiv-by-zero -Wdouble-promotion "
               "-Wempty-body -Wendif-labels -Wenum-compare -Wextra "
               "-Wfloat-equal -Wformat -Wfree-nonheap-object "
               "-Wignored-qualifiers -Winit-self "
               "-Winline -Wint-to-pointer-cast -Winvalid-memory-model "
               "-Winvalid-offsetof -Wlogical-op -Wmain -Wmaybe-uninitialized "
               "-Wmissing-braces -Wmissing-field-initializers -Wmultichar "
               "-Wnarrowing -Wnoexcept -Wnon-template-friend "
               "-Wnon-virtual-dtor -Wnonnull -Woverflow "
               "-Woverlength-strings -Wparentheses "
               "-Wpmf-conversions -Wpointer-arith -Wreorder "
               "-Wreturn-type -Wsequence-point -Wshadow "
               "-Wsign-compare -Wswitch -Wtype-limits -Wundef "
               "-Wuninitialized -Wunused -Wvla -Wwrite-strings")
  endif()
  # Convert CMake list to a single string, erasing the ";" separators
  string(REPLACE ";" "" WFLAGS_STR ${WFLAGS})
  set(EXTRA_FLAGS "${EXTRA_FLAGS} ${WFLAGS_STR}")
endif()

# Add -stdlib=libc++ when using Clang if possible
if(NOT OPTION_NO_AUTO_LIBCPP AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CXXFLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "-std=c++1y -stdlib=libc++")
  try_run(ProgramResult
          CompilationSucceeded
          "${CMAKE_CURRENT_BINARY_DIR}"
          "${CMAKE_MODULE_PATH}/get_compiler_version.cpp"
          RUN_OUTPUT_VARIABLE CompilerVersion)
  if(NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
    message(STATUS "Use clang with GCC' libstdc++")
  else()
    message(STATUS "Automatically added '-stdlib=libc++' flag "
                   "(OPTION_NO_AUTO_LIBCPP not defined)")
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -stdlib=libc++")
  endif()
  # Restore CXX flags
  set(CMAKE_CXX_FLAGS "${CXXFLAGS_BACKUP}")
endif()

# Allow unused private fields when using clang
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(EXTRA_FLAGS "${EXTRA_FLAGS} -Wno-unused-private-field")
endif()

# Enable Position Independent Code (PIC) in shared libraries
if(NOT MINGW)
  set(EXTRA_FLAGS "${EXTRA_FLAGS} -fPIC")
endif()

# Use pthreads on mingw and linux
if(NOT APPLE AND NOT WIN32)
  set(EXTRA_FLAGS "${EXTRA_FLAGS} -pthread")
endif()

# Performance
set(EXTRA_FLAGS "${EXTRA_FLAGS} -march=native -mpopcnt")

# Performance
if (OPTION_ENABLE_ADDRESS_SANITIZER)
  set(CXXFLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
  try_run(program_result
          compilation_succeeded
          "${CMAKE_CURRENT_BINARY_DIR}"
          "${CMAKE_MODULE_PATH}/get_compiler_version.cpp")
  set(CMAKE_CXX_FLAGS "${CXXFLAGS_BACKUP}")
  if(NOT compilation_succeeded)
    message(STATUS "Address Sanitizer not available on selected compiler")
  else()
    message(STATUS "Enabling Address Sanitizer")
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(ASAN_FOUND true)
  endif()
endif(OPTION_ENABLE_ADDRESS_SANITIZER)

# Check if the user provided CXXFLAGS, set defaults otherwise
if(NOT CMAKE_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS                   "-Wextra -Wall -pedantic ${EXTRA_FLAGS}")
endif()
if(NOT CMAKE_CXX_FLAGS_DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG             "-O0 -g")
endif()
if(NOT CMAKE_CXX_FLAGS_MINSIZEREL)
  set(CMAKE_CXX_FLAGS_MINSIZEREL        "-Os")
endif()
if(NOT CMAKE_CXX_FLAGS_RELEASE)
  set(CMAKE_CXX_FLAGS_RELEASE           "-O3 -DNDEBUG")
endif()
if(NOT CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "-O2 -g")
endif()

# Set build default build type to RelWithDebInfo if not set
if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Needed by subprojects
set(LD_FLAGS ${LD_FLAGS} ${CMAKE_LD_LIBS})
