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

struct Application {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    GLuint program;
    GLuint vao;
    GLuint vbo;

    bool running = true;
    i32 window_width = 800;
    i32 window_height = 600;

    bool initialize() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
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

        constexpr u8 vertex_shader_source[] = {
            #embed "shaders/vertex.glsl"
        };
        const GLchar* vertex_source_ptr = (const GLchar*)vertex_shader_source;
        const GLint vertex_source_len = sizeof(vertex_shader_source);
        const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_source_ptr, &vertex_source_len);
        glCompileShader(vertex_shader);

        if(!checkShaderCompilation(vertex_shader, "VERTEX")) {
            return false;
        }

        constexpr u8 frag_shader_source[] = {
            #embed "shaders/frag.glsl"
        };
        const GLchar* frag_source_ptr = (const GLchar*)frag_shader_source;
        const GLint frag_source_len = sizeof(frag_shader_source);
        const GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_shader, 1, &frag_source_ptr, &frag_source_len);
        glCompileShader(frag_shader);

        if(!checkShaderCompilation(frag_shader, "FRAGMENT")) {
            return false;
        }

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);

        if(!checkProgramLinking()) {
            return false;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(frag_shader);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            SDL_Log("OpenGL initialization error: %d", error);
            return false;
        }

        SDL_Log("Shaders compiled and linked successfully");

        return true;
    }

    bool checkShaderCompilation(GLuint shader, const char* type) {
        GLint success;
        GLchar log_msg[1024];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, sizeof(log_msg), nullptr, log_msg);
            SDL_Log("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s", type, log_msg);
            return false;
        }

        return true;
    }

    bool checkProgramLinking() {
        GLint success;
        GLchar log_msg[1024];

        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(program, sizeof(log_msg), nullptr, log_msg);
            SDL_Log("ERROR::PROGRAM_LINKING_ERROR\n%s", log_msg);
            return false;
        }

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

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            SDL_Log("OpenGL render error: %d at frame: %f", error, currentTime);
        }
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
