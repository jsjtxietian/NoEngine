
cmake_minimum_required(VERSION 3.0)
project(sx)

# Enable Assembler
if (MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    enable_language(CXX ASM_MASM)
else()
    enable_language(CXX ASM)
endif()

set(SOURCE_FILES test.cpp src/fiber/fiber.cpp)

set(INCLUDE_FILES src/fiber/fiber.h)

####################################################################################################
# Assembly files for fcontext
if (WIN32)
    # Windows (x86/64)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CPU_ARCH "x86_64")
    endif()

    if (MSVC)
        set(ASM_EXT "ms_pe_masm.asm")
    endif()
endif()

set(ASM_SOURCES "src/fiber/asm/make_${CPU_ARCH}_${ASM_EXT}"
                "src/fiber/asm/jump_${CPU_ARCH}_${ASM_EXT}"
                "src/fiber/asm/ontop_${CPU_ARCH}_${ASM_EXT}")
set_source_files_properties(${ASM_SOURCES} PROPERTIES COMPILE_DEFINITIONS BOOST_CONTEXT_EXPORT=)

add_executable(test ${SOURCE_FILES} ${INCLUDE_FILES} ${ASM_SOURCES})
target_link_libraries(test PUBLIC psapi)
