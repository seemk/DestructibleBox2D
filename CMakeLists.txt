cmake_minimum_required(VERSION 2.8.8)
project(Box2DDestructibleTerrain)

option(UNIX_LIBCXXABI_EXTRAS_ENABLED OFF)

find_package(Boost 1.55 REQUIRED)
set(Boost_USE_STATIC_LIBS       ON)
set(Boost_USE_MULTITHREADED     ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(UNIX_LIBCXX_EXTRA_FLAGS "-nodefaultlibs -lc++ -lc++abi -lm -lc -lgcc_s -lgcc")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_definitions( -DBOOST_RESULT_OF_USE_DECLTYPE )
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
	endif()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:fast /Gy /Gw ")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions("-std=c++1y -stdlib=libc++ -Wno-narrowing")
    
    if(UNIX_LIBCXXABI_EXTRAS_ENABLED)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${UNIX_LIBCXX_EXTRA_FLAGS}")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions("-std=c++11")
endif()
add_definitions( -DSFML_STATIC )

include_directories(${Boost_INCLUDE_DIR})
set(BOX2D_PATH ${CMAKE_SOURCE_DIR}/third_party/Box2D)
set(SFML_PATH ${CMAKE_SOURCE_DIR}/third_party/SFML)
set(SFML_INCLUDE_DIR ${SFML_PATH}/include)

include_directories(${BOX2D_PATH})

add_subdirectory(src)
add_subdirectory(${SFML_PATH})
add_subdirectory(${BOX2D_PATH})
