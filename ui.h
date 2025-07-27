#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Transform.h"
#include "Model.h"

struct GLFWwindow;

namespace UI {
    void Init(GLFWwindow* window);
    void BeginFrame();
    void RenderUI(Transform& modelTransform, Model* currentModel = nullptr);
    void EndFrame();
    void Shutdown();

    // Debug console functions
    void AddDebugMessage(const std::string& message);
    void ClearDebugConsole();

    // Stats functions
    void UpdateStats(float deltaTime);
    void UpdateModelLoadingProgress(float progress, const std::string& stage = "");

    // Expose variables for external access
    extern std::string selectedModelPath;
    extern std::string selectedMtlPath;
    extern bool modelSelected;
    extern bool mtlSelected;
    extern bool resetCameraPosition;
    extern bool textureUpdated;
    extern bool cameraMovementEnabled;
    extern bool takeScreenshot;
    extern std::string selectedTextureFolder;
    extern bool textureFolderSelected;
    extern bool reloadModelWithMtl;
}