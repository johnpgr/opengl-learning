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
    SDL_GLContextState* gl_context = nullptr;
    bool running = true;
    i32 window_width = 800;
    i32 window_height = 600;

    bool init() {
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

        window = SDL_CreateWindow("SDL3 OpenGL Application",
                                  window_width,
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

        SDL_GL_SetSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            SDL_Log("Failed to initialize GLAD");
            return false;
        }

        auto version = (const char*)(glGetString(GL_VERSION));
        auto renderer = (const char*)(glGetString(GL_RENDERER));
        SDL_Log("OpenGL Version: %s", version);
        SDL_Log("Renderer: %s", renderer);

        glViewport(0, 0, window_width, window_height);

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
        const f32 color[] = {(float)sin(currentTime) * 0.5f + 0.5f,
                             (float)cos(currentTime) * 0.5f + 0.5f, 0.0f, 1.0f};

        glClearBufferfv(GL_COLOR, 0, color);
    }

    void run() {
        while (running) {
            handleEvents();
            render(SDL_GetTicks() / 1000.0);
            SDL_GL_SwapWindow(window);
        }
    }

    void cleanup() {
        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    ~Application() { cleanup(); }
};

int main() {
    Application app;

    if (!app.init()) {
        SDL_Log("Failed to initialize application");
        return -1;
    }

    SDL_Log("Application initialized successfully");
    SDL_Log("Press ESC to exit");

    app.run();

    return 0;
}
