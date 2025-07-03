#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "render.h"
#include "window.h"
#include "ui.h"



int main() {
    Window::Init();
    UI::Init(Window::GetGLFWWindow());

    while (!Window::ShouldClose()) {
        Window::PollEvents();
        Window::ProcessInput();

        Render::ClearScreen();

        UI::BeginFrame();
        UI::RenderUI();
        UI::EndFrame();

        Window::SwapBuffers();
    }

    UI::Shutdown();
    Window::Shutdown();

    return 0;
}
