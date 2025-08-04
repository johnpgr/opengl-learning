#include "glad/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <print>

using std::println;

struct Application {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool running = true;
    int window_width = 800;
    int window_height = 600;

    bool initialize() {
        // Initialize SDL
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }

        // Set OpenGL attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create window
        window = SDL_CreateWindow(
            "SDL3 OpenGL Application",
            window_width, window_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

        if (!window) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            return false;
        }

        // Create OpenGL context
        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context) {
            SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
            return false;
        }

        // Enable VSync
        SDL_GL_SetSwapInterval(1);

        // Load OpenGL functions with GLAD
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            println("Failed to initialize GLAD");
            return false;
        }

        // Print OpenGL version
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        println("OpenGL Version: {}", version);
        println("Renderer: {}", renderer);

        // Set initial viewport
        glViewport(0, 0, window_width, window_height);

        return true;
    }

    void handle_events() {
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

    void render() {
        // Clear the screen with red color
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    void run() {
        while (running) {
            handle_events();
            render();
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

    ~Application() {
        cleanup();
    }
};

int main(void) {
    Application app;
    
    if (!app.initialize()) {
        println("Failed to initialize application");
        return -1;
    }
    
    println("Application initialized successfully");
    println("Press ESC to exit");
    
    app.run();
    
    return 0;
}
