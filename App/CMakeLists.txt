
# App

set(SOURCE_DIR "Source")

set(SOURCES
    Source/Main.cpp
    Source/Shader.h
    Source/Shader.cpp
    Source/Renderer.h
    Source/Renderer.cpp
)

# Add the executable target
add_executable(App ${SOURCES})
target_link_libraries(App glfw)
target_link_libraries(App glad)
