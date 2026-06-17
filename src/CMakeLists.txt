
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src *.cpp)

add_executable(fyra ${SOURCES})

target_include_directories(
    fyra
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)