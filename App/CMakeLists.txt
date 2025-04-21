
# App

set(SOURCE_DIR "Source")

set(SOURCES
    Source/Main.cpp
    Source/Shader.h
    Source/Shader.cpp
    Source/Renderer.h
    Source/Renderer.cpp
    vendor/stb/stb_image.h
    vendor/stb/stb_image.cpp
)

# Add the executable target
add_executable(App ${SOURCES})

target_link_libraries(App glfw)
target_link_libraries(App glad)
target_link_libraries(App glm)

target_include_directories(App PRIVATE vendor/stb)