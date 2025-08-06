#include "glad/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <math.h>

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

const char* getShaderTypeName(GLenum type) {
    switch(type) {
        case GL_VERTEX_SHADER: return "VERTEX";
        case GL_FRAGMENT_SHADER: return "FRAGMENT";
        case GL_GEOMETRY_SHADER: return "GEOMETRY";
        case GL_TESS_CONTROL_SHADER: return "TESSELLATION_CONTROL";
        case GL_TESS_EVALUATION_SHADER: return "TESSELLATION_EVALUATION";
        default: return nullptr;
    }
}

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
        SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE
        );
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        window = SDL_CreateWindow(
            "SDL3 OpenGL Application",
            window_width,
            window_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

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

        const auto version = (const char*)(glGetString(GL_VERSION));
        const auto renderer = (const char*)(glGetString(GL_RENDERER));
        SDL_Log("OpenGL Version: %s", version);
        SDL_Log("Renderer: %s", renderer);

        SDL_GL_SetSwapInterval(1);
        glViewport(0, 0, window_width, window_height);

        constexpr u8 vs_source[] = {
            #embed "shaders/vertex.glsl"
        };

        const auto vs = compileShader(
            (const GLchar*)vs_source,
            sizeof(vs_source),
            GL_VERTEX_SHADER
        );

        constexpr u8 tcs_source[] = {
            #embed "shaders/tessellation_control.glsl"
        };

        const auto tcs = compileShader(
            (const GLchar*)tcs_source,
            sizeof(tcs_source),
            GL_TESS_CONTROL_SHADER
        );

        constexpr u8 tes_source[] = {
            #embed "shaders/tessellation_evaluation.glsl"
        };

        const auto tes = compileShader(
            (const GLchar*)tes_source,
            sizeof(tes_source),
            GL_TESS_EVALUATION_SHADER
        );

        constexpr u8 gs_source[] = {
            #embed "shaders/geometry.glsl"
        };

        const auto gs = compileShader(
            (const GLchar*)gs_source,
            sizeof(gs_source),
            GL_GEOMETRY_SHADER
        );

        constexpr u8 fs_source[] = {
            #embed "shaders/fragment.glsl"
        };

        const auto fs = compileShader(
            (const GLchar*)fs_source,
            sizeof(fs_source),
            GL_FRAGMENT_SHADER
        );

        program = glCreateProgram();

        glAttachShader(program, vs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);
        glAttachShader(program, gs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        if (!checkProgramLinking()) {
            return false;
        }

        glDeleteShader(vs);
        glDeleteShader(tcs);
        glDeleteShader(tes);
        glDeleteShader(gs);
        glDeleteShader(fs);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            SDL_Log("OpenGL initialization error: %d", error);
            return false;
        }

        SDL_Log("Shaders compiled and linked successfully");

        return true;
    }

    GLuint compileShader(
        const GLchar* code,
        const GLint code_len,
        const GLenum shader_type
    ) {
        const auto shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &code, &code_len);
        glCompileShader(shader);
        
        const auto type_name = getShaderTypeName(shader_type);

        if (!type_name || !checkShaderCompilation(shader, type_name)) {
            return 0;
        }

        return shader;
    }

    bool checkShaderCompilation(GLuint shader, const char* type) {
        GLint success;
        GLchar log_msg[1024];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, sizeof(log_msg), nullptr, log_msg);
            SDL_Log("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s", type,
                    log_msg);
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
        const f32 color[] = {0.0, 0.0, 0.0, 0.0};
        glClearBufferfv(GL_COLOR, 0, color);

        glUseProgram(program);
        glPointSize(5.0);
        glDrawArrays(GL_PATCHES, 0, 3);

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
