
include_directories(${SFML_INCLUDE_DIR}
                    ../include)

set(DEMO_HEADERS ../include/ShapeFactory.h
                 ../include/DebugDraw.h
                 ../include/Constants.h
                 ../include/Geometry.h
                 )

set(DEMO_SOURCES ../src/Geometry.cpp
                 ../src/DebugDraw.cpp
                 ../src/ShapeFactory.cpp
                 ../src/main.cpp
                 )

add_executable(Demo ${DEMO_HEADERS}
                    ${DEMO_SOURCES}
                    )                

target_link_libraries(Demo sfml-system
                           sfml-window
                           sfml-graphics
                           Box2D)

add_custom_command(TARGET Demo PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resource $<TARGET_FILE_DIR:Demo>)