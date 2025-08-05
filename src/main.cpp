#include "glad/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef size_t usize;

constexpr u8 vertex_shader_source[] = {
#embed "shaders/vertex.glsl"
};

constexpr u8 frag_shader_source[] = {
#embed "shaders/frag.glsl"
};

struct Application {
    SDL_Window* window = nullptr;
    SDL_GLContextState* gl_context = nullptr;
    GLuint program;
    GLuint vao;

    bool running = true;
    i32 window_width = 800;
    i32 window_height = 600;

    bool initialize() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        window = SDL_CreateWindow("SDL3 OpenGL Application", window_width,
                                  window_height,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (!window) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            return false;
        }

        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context) {
            SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
            return false;
        }

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            SDL_Log("Failed to initialize GLAD");
            return false;
        }

        auto version = (const char*)(glGetString(GL_VERSION));
        auto renderer = (const char*)(glGetString(GL_RENDERER));
        SDL_Log("OpenGL Version: %s", version);
        SDL_Log("Renderer: %s", renderer);

        SDL_GL_SetSwapInterval(1);
        glViewport(0, 0, window_width, window_height);

        // Compile shaders
        const GLchar* vertex_source_ptr = (const GLchar*)vertex_shader_source;
        const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_source_ptr, nullptr);
        glCompileShader(vertex_shader);

        const GLchar* frag_source_ptr = (const GLchar*)frag_shader_source;
        const GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_shader, 1, &frag_source_ptr, nullptr);
        glCompileShader(frag_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);

        glDeleteShader(vertex_shader);
        glDeleteShader(frag_shader);

        glCreateVertexArrays(1, &vao);
        glBindVertexArray(vao);

        return true;
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                window_width = event.window.data1;
                window_height = event.window.data2;
                glViewport(0, 0, window_width, window_height);
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            }
        }
    }

    void render(f64 currentTime) {
        const GLfloat color[] = { 0.0f, 0.2f, 0.0f, 1.0f };

        glClearBufferfv(GL_COLOR, 0, color);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void run() {
        while (running) {
            handleEvents();
            render(SDL_GetTicks() / 1000.0);
            SDL_GL_SwapWindow(window);
        }
    }

    void shutdown() {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);

        if (window) {
            SDL_DestroyWindow(window);
        }

        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
        }

        SDL_Quit();
    }

    ~Application() { shutdown(); }
};

int main() {
    Application app;

    if (!app.initialize()) {
        SDL_Log("Failed to initialize application");
        return -1;
    }

    SDL_Log("Application initialized successfully");
    SDL_Log("Press ESC to exit");

    app.run();

    return 0;
}
