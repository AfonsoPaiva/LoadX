#include "ui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "lighting.h"
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <chrono>
#include <algorithm>  // Add this for std::min, std::max
#include <vector>     // Add this for std::vector
#include <cfloat>     // Add this for FLT_MAX
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

// Lighting controls
extern DirectionalLight dirLight;
extern PointLight pointLight;
extern SpotLight spotLight;

namespace UI {
    std::string selectedModelPath;
    std::string selectedMtlPath;
    bool modelSelected = false;
    bool mtlSelected = false;
    bool resetCameraPosition = false;
    bool textureUpdated = false;
    bool cameraMovementEnabled = false;
    bool takeScreenshot = false;
    std::string selectedTextureFolder;
    bool textureFolderSelected = false;
    bool reloadModelWithMtl = false;

    // Debug console data
    static std::deque<std::string> debugMessages;
    static bool autoScrollDebug = true;
    static const size_t MAX_DEBUG_MESSAGES = 1000;

    // Performance stats data
    static float frameTime = 0.0f;
    static float fps = 0.0f;
    static float avgFrameTime = 0.0f;
    static float minFrameTime = FLT_MAX;
    static float maxFrameTime = 0.0f;
    static std::deque<float> frameTimeHistory;
    static std::deque<float> fpsHistory;
    static const size_t MAX_HISTORY = 100;

    // Memory stats
    static size_t memoryUsage = 0;
    static size_t peakMemoryUsage = 0;
    static size_t availableMemory = 0;

    // GPU stats
    static std::string gpuVendor = "Unknown";
    static std::string gpuRenderer = "Unknown";
    static std::string glVersion = "Unknown";
    static std::string glslVersion = "Unknown";

    // Model loading stats
    static float modelLoadingProgress = 0.0f;
    static std::string modelLoadingStage = "";
    static bool isModelLoading = false;
    static std::chrono::steady_clock::time_point modelLoadStartTime;
    static float modelLoadElapsedTime = 0.0f;

    // System stats
    static int cpuCores = 0;
    static float cpuUsage = 0.0f;

    // Rendering stats
    static int drawCalls = 0;
    static int vertices = 0;
    static int triangles = 0;
    static int textures = 0;

    void Init(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        // Initialize GPU info
        gpuVendor = (const char*)glGetString(GL_VENDOR);
        gpuRenderer = (const char*)glGetString(GL_RENDERER);
        glVersion = (const char*)glGetString(GL_VERSION);
        glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // Initialize system info
#ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cpuCores = static_cast<int>(sysInfo.dwNumberOfProcessors);
#else
        cpuCores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
#endif

        AddDebugMessage("UI System Initialized");
        AddDebugMessage("GPU: " + gpuRenderer);
        AddDebugMessage("OpenGL: " + glVersion);
    }

    size_t GetMemoryUsage() {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
#else
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            return static_cast<size_t>(usage.ru_maxrss) * 1024; // Convert from KB to bytes
        }
#endif
        return 0;
    }

    size_t GetAvailableMemory() {
#ifdef _WIN32
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            return static_cast<size_t>(memInfo.ullAvailPhys);
        }
#else
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        if (pages > 0 && page_size > 0) {
            return static_cast<size_t>(pages) * static_cast<size_t>(page_size);
        }
#endif
        return 0;
    }

    void UpdateStats(float deltaTime) {
        frameTime = deltaTime * 1000.0f; // Convert to milliseconds
        fps = 1.0f / deltaTime;

        // Update frame time statistics
        frameTimeHistory.push_back(frameTime);
        fpsHistory.push_back(fps);

        if (frameTimeHistory.size() > MAX_HISTORY) {
            frameTimeHistory.pop_front();
        }
        if (fpsHistory.size() > MAX_HISTORY) {
            fpsHistory.pop_front();
        }

        // Calculate averages and min/max - Fixed the std::min/max calls
        float sum = 0.0f;
        minFrameTime = FLT_MAX;
        maxFrameTime = 0.0f;

        for (float ft : frameTimeHistory) {
            sum += ft;
            minFrameTime = (std::min)(minFrameTime, ft);  // Parentheses to avoid macro conflicts
            maxFrameTime = (std::max)(maxFrameTime, ft);  // Parentheses to avoid macro conflicts
        }

        if (!frameTimeHistory.empty()) {
            avgFrameTime = sum / static_cast<float>(frameTimeHistory.size());  // Fixed size_t to float conversion
        }

        // Update memory stats - Fixed std::max calls
        memoryUsage = GetMemoryUsage();
        peakMemoryUsage = (std::max)(peakMemoryUsage, memoryUsage);  // Parentheses to avoid macro conflicts
        availableMemory = GetAvailableMemory();

        // Update model loading elapsed time
        if (isModelLoading) {
            auto now = std::chrono::steady_clock::now();
            modelLoadElapsedTime = std::chrono::duration<float>(now - modelLoadStartTime).count();
        }
    }

    void UpdateModelLoadingProgress(float progress, const std::string& stage) {
        modelLoadingProgress = progress;
        modelLoadingStage = stage;

        if (progress <= 0.0f) {
            isModelLoading = true;
            modelLoadStartTime = std::chrono::steady_clock::now();
            modelLoadElapsedTime = 0.0f;
        }
        else if (progress >= 1.0f) {
            isModelLoading = false;
        }
    }

    void AddDebugMessage(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::tm timeinfo;
#ifdef _WIN32
        localtime_s(&timeinfo, &time_t);
#else
        timeinfo = *std::localtime(&time_t);
#endif

        std::ostringstream oss;
        oss << "[" << std::setfill('0') << std::setw(2) << timeinfo.tm_hour
            << ":" << std::setw(2) << timeinfo.tm_min
            << ":" << std::setw(2) << timeinfo.tm_sec << "] " << message;

        debugMessages.push_back(oss.str());

        if (debugMessages.size() > MAX_DEBUG_MESSAGES) {
            debugMessages.pop_front();
        }
    }

    void ClearDebugConsole() {
        debugMessages.clear();
    }

    void BeginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void RenderUI(Transform& modelTransform, Model* currentModel) {
        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        const float windowPadding = 10.0f;
        const float leftColumnWidth = 300.0f;
        const float middleColumnWidth = 400.0f;
        const float rightColumnWidth = 350.0f;

        // Model loading window with enhanced progress bar
        ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftColumnWidth, 320), ImGuiCond_Always);
        ImGui::Begin("Model Loader", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::Text("Model File:");
        if (ImGui::Button("Select Model File", ImVec2(-1, 25)))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Model File", ".OBJ,.obj,.fbx,.gltf,.glb,.3ds,.dae,.blend,.x3d,.ply,.stl");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                selectedModelPath = ImGuiFileDialog::Instance()->GetFilePathName();
                modelSelected = true;
                AddDebugMessage("Model selected: " + selectedModelPath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (!selectedModelPath.empty()) {
            ImGui::TextWrapped("Model: %s", selectedModelPath.c_str());
        }

        // Enhanced loading progress display
        if (isModelLoading || (currentModel && currentModel->IsLoading())) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Loading Model...");

            float progress = currentModel ? currentModel->GetLoadingProgress() : modelLoadingProgress;

            // Progress bar with custom colors
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::ProgressBar(progress, ImVec2(-1, 20), "");
            ImGui::PopStyleColor();

            // Progress text overlay
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("%.1f%%", progress * 100.0f);

            // Loading stage and time
            if (!modelLoadingStage.empty()) {
                ImGui::Text("Stage: %s", modelLoadingStage.c_str());
            }

            ImGui::Text("Time: %.2fs", modelLoadElapsedTime);

            // Animated loading indicator
            const char* loadingChars = "|/-\\";
            static int loadingFrame = 0;
            loadingFrame = (loadingFrame + 1) % 4;
            ImGui::SameLine();
            ImGui::Text(" %c", loadingChars[loadingFrame]);
        }

        ImGui::Separator();

        // MTL file selection (only show if OBJ model is loaded)
        if (currentModel && currentModel->IsObjFile()) {
            ImGui::Text("MTL File (for OBJ materials):");

            if (ImGui::Button("Select MTL File", ImVec2(-1, 25))) {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseMtlDlgKey", "Choose MTL File", ".mtl");
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseMtlDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    selectedMtlPath = ImGuiFileDialog::Instance()->GetFilePathName();
                    mtlSelected = true;
                    reloadModelWithMtl = true;
                    AddDebugMessage("MTL file selected: " + selectedMtlPath);
                }
                ImGuiFileDialog::Instance()->Close();
            }

            if (!selectedMtlPath.empty()) {
                ImGui::TextWrapped("MTL: %s", selectedMtlPath.c_str());
            }

            if (ImGui::Button("Clear MTL File", ImVec2(-1, 20))) {
                selectedMtlPath = "";
                AddDebugMessage("MTL file cleared");
            }

            ImGui::Separator();

            if (currentModel->HasMtlFile()) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: OBJ with MTL materials");
            }
            else {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Status: OBJ with default materials");
                ImGui::TextWrapped("Load an MTL file for proper materials and textures");
            }
        }
        else if (currentModel && !currentModel->IsObjFile()) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Non-OBJ format (materials included)");
        }

        ImGui::End();

        // Camera Controls
        ImGui::SetNextWindowPos(ImVec2(windowPadding, 340), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftColumnWidth, 180), ImGuiCond_Always);
        ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        if (cameraMovementEnabled) {
            if (ImGui::Button("Disable Camera Movement", ImVec2(-1, 30))) {
                cameraMovementEnabled = false;
                AddDebugMessage("Camera movement disabled");
            }
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Camera Movement: ENABLED");
        }
        else {
            if (ImGui::Button("Enable Camera Movement", ImVec2(-1, 30))) {
                cameraMovementEnabled = true;
                AddDebugMessage("Camera movement enabled");
            }
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Camera Movement: DISABLED");
        }

        ImGui::Separator();

        if (ImGui::Button("Take Screenshot", ImVec2(-1, 35))) {
            takeScreenshot = true;
            AddDebugMessage("Screenshot requested");
        }

        ImGui::Text("Controls (when enabled):");
        ImGui::BulletText("WASD - Move camera");
        ImGui::BulletText("Mouse + Left Click - Look around");
        ImGui::BulletText("Scroll - Zoom in/out");

        if (ImGui::Button("Reset Camera Position")) {
            resetCameraPosition = true;
            AddDebugMessage("Camera position reset");
        }

        ImGui::End();

        // Object Transform Controls
        float transformWindowHeight = screenHeight - 540;
        ImGui::SetNextWindowPos(ImVec2(windowPadding, 530), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftColumnWidth, transformWindowHeight), ImGuiCond_Always);
        ImGui::Begin("Object Transform", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::Text("Position");
        ImGui::SliderFloat("X##pos", &modelTransform.position.x, -10.0f, 10.0f);
        ImGui::SliderFloat("Y##pos", &modelTransform.position.y, -10.0f, 10.0f);
        ImGui::SliderFloat("Z##pos", &modelTransform.position.z, -10.0f, 10.0f);

        ImGui::Separator();

        ImGui::Text("Rotation (degrees)");
        ImGui::SliderFloat("X##rot", &modelTransform.rotation.x, -180.0f, 180.0f);
        ImGui::SliderFloat("Y##rot", &modelTransform.rotation.y, -180.0f, 180.0f);
        ImGui::SliderFloat("Z##rot", &modelTransform.rotation.z, -180.0f, 180.0f);

        ImGui::Separator();

        ImGui::Text("Scale");
        static float uniformScale = 1.0f;
        if (ImGui::SliderFloat("Uniform Scale", &uniformScale, 0.01f, 5.0f)) {
            modelTransform.scale = glm::vec3(uniformScale);
        }

        ImGui::Separator();
        ImGui::Text("Individual Scale");
        ImGui::SliderFloat("X##scale", &modelTransform.scale.x, 0.01f, 5.0f);
        ImGui::SliderFloat("Y##scale", &modelTransform.scale.y, 0.01f, 5.0f);
        ImGui::SliderFloat("Z##scale", &modelTransform.scale.z, 0.01f, 5.0f);

        ImGui::Separator();

        if (ImGui::Button("Reset Transform")) {
            modelTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
            modelTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
            modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            uniformScale = 1.0f;
            AddDebugMessage("Transform reset to defaults");
        }

        ImGui::SameLine();

        if (ImGui::Button("Auto Size")) {
            if (currentModel) {
                float recommendedScale = currentModel->GetRecommendedScale();
                modelTransform.scale = glm::vec3(recommendedScale);
                uniformScale = recommendedScale;
                AddDebugMessage("Auto-sizing applied: scale = " + std::to_string(recommendedScale));
            }
        }

        if (ImGui::Button("Normal Size")) {
            modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            uniformScale = 1.0f;
            AddDebugMessage("Scale reset to normal size (1.0)");
        }

        ImGui::SameLine();

        if (ImGui::Button("Center Model")) {
            modelTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
            AddDebugMessage("Model centered at origin");
        }

        // Display model information if available
        if (currentModel && currentModel->GetModelSize() != glm::vec3(0.0f)) {
            ImGui::Separator();
            ImGui::Text("Model Information:");
            glm::vec3 size = currentModel->GetModelSize();
            glm::vec3 center = currentModel->GetModelCenter();
            float recommendedScale = currentModel->GetRecommendedScale();

            ImGui::Text("Size: %.2f x %.2f x %.2f", size.x, size.y, size.z);
            ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
            ImGui::Text("Recommended Scale: %.3f", recommendedScale);
        }

        ImGui::End();

        // Create tab bar for middle column with Stats tab
        float middleColumnX = leftColumnWidth + (2 * windowPadding);
        float middleWindowHeight = screenHeight - (2 * windowPadding);

        ImGui::SetNextWindowPos(ImVec2(middleColumnX, windowPadding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(middleColumnWidth, middleWindowHeight), ImGuiCond_Always);
        ImGui::Begin("Engine Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        if (ImGui::BeginTabBar("EngineTabBar")) {
            // Performance Stats Tab
            if (ImGui::BeginTabItem("Performance Stats")) {
                // Performance metrics
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Performance Metrics");
                ImGui::Separator();

                // FPS and Frame Time
                ImGui::Text("Current FPS: %.1f", fps);
                ImGui::Text("Frame Time: %.2f ms", frameTime);
                ImGui::Text("Avg Frame Time: %.2f ms", avgFrameTime);
                ImGui::Text("Min Frame Time: %.2f ms", minFrameTime);
                ImGui::Text("Max Frame Time: %.2f ms", maxFrameTime);

                // FPS Graph
                if (fpsHistory.size() > 1) {
                    std::vector<float> fpsArray(fpsHistory.begin(), fpsHistory.end());
                    ImGui::PlotLines("FPS History", fpsArray.data(), static_cast<int>(fpsArray.size()), 0, nullptr, 0.0f, 200.0f, ImVec2(0, 80));
                }

                // Frame Time Graph
                if (frameTimeHistory.size() > 1) {
                    std::vector<float> frameTimeArray(frameTimeHistory.begin(), frameTimeHistory.end());
                    ImGui::PlotLines("Frame Time (ms)", frameTimeArray.data(), static_cast<int>(frameTimeArray.size()), 0, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
                }

                ImGui::Spacing();

                // Memory Stats
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Memory Usage");
                ImGui::Separator();

                ImGui::Text("Current: %.2f MB", static_cast<float>(memoryUsage) / (1024.0f * 1024.0f));
                ImGui::Text("Peak: %.2f MB", static_cast<float>(peakMemoryUsage) / (1024.0f * 1024.0f));
                ImGui::Text("Available: %.2f GB", static_cast<float>(availableMemory) / (1024.0f * 1024.0f * 1024.0f));

                // Memory usage bar
                float memoryPercent = availableMemory > 0 ? static_cast<float>(memoryUsage) / static_cast<float>(availableMemory) : 0.0f;
                ImGui::ProgressBar(memoryPercent, ImVec2(-1, 0), "");
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Memory Usage");

                ImGui::Spacing();

                // System Information
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "System Information");
                ImGui::Separator();

                ImGui::Text("CPU Cores: %d", cpuCores);
                ImGui::Text("GPU: %s", gpuRenderer.c_str());
                ImGui::Text("OpenGL: %s", glVersion.c_str());
                ImGui::Text("GLSL: %s", glslVersion.c_str());

                ImGui::Spacing();

                // Rendering Stats (if model is loaded)
                if (currentModel) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.8f, 1.0f), "Rendering Statistics");
                    ImGui::Separator();

                    // These would need to be updated from the actual rendering code
                    ImGui::Text("Draw Calls: %d", drawCalls);
                    ImGui::Text("Vertices: %d", vertices);
                    ImGui::Text("Triangles: %d", triangles);
                    ImGui::Text("Textures: %d", textures);
                }

                ImGui::EndTabItem();
            }

            // Texture Manager Tab
            if (ImGui::BeginTabItem("Textures")) {
                if (currentModel) {
                    if (currentModel->IsObjFile()) {
                        if (currentModel->HasMtlFile()) {
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "OBJ Model with MTL Materials");
                        }
                        else {
                            ImGui::TextColored(ImVec4(1, 1, 0, 1), "OBJ Model (No MTL file loaded)");
                            ImGui::TextWrapped("Load an MTL file for proper materials and textures");
                        }
                    }
                    else {
                        ImGui::TextColored(ImVec4(0, 0.8f, 1, 1), "Non-OBJ Model (materials embedded)");
                    }

                    ImGui::Separator();

                    ImGui::Text("Auto-Load Textures from Folder");
                    ImGui::Separator();

                    if (ImGui::Button("Select Texture Folder", ImVec2(-1, 30))) {
                        IGFD::FileDialogConfig config;
                        config.path = ".";
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFolderDlgKey", "Choose Texture Folder", nullptr, config);
                        AddDebugMessage("Texture folder dialog opened");
                    }

                    if (ImGuiFileDialog::Instance()->Display("ChooseFolderDlgKey")) {
                        if (ImGuiFileDialog::Instance()->IsOk()) {
                            selectedTextureFolder = ImGuiFileDialog::Instance()->GetCurrentPath();
                            textureFolderSelected = true;
                            AddDebugMessage("Texture folder selected: " + selectedTextureFolder);
                        }
                        ImGuiFileDialog::Instance()->Close();
                    }

                    if (!selectedTextureFolder.empty()) {
                        ImGui::TextWrapped("Folder: %s", selectedTextureFolder.c_str());
                    }

                    ImGui::Text("Supported naming patterns:");
                    ImGui::BulletText("*diffuse*, *albedo*, *basecolor*, *color*");
                    ImGui::BulletText("*normal*, *norm*, *nrm*");
                    ImGui::BulletText("*specular*, *spec*");
                    ImGui::BulletText("*roughness*, *rough*");
                    ImGui::BulletText("*metallic*, *metal*, *met*");
                    ImGui::BulletText("*height*, *displacement*, *bump*");
                    ImGui::BulletText("*emission*, *emissive*, *glow*");
                    ImGui::BulletText("*ao*, *ambient*, *occlusion*");

                    ImGui::Separator();

                    if (ImGui::Button("Clear All Custom Textures", ImVec2(-1, 30))) {
                        currentModel->ClearCustomTextures();
                        textureUpdated = true;
                        AddDebugMessage("All custom textures cleared");
                    }

                    ImGui::Separator();
                    ImGui::Text("Current Material Textures:");
                    MaterialTextures matTextures = currentModel->GetMaterialTextures();

                    if (!matTextures.diffuse.empty()) ImGui::BulletText("Diffuse/BaseColor: %zu texture(s)", matTextures.diffuse.size());
                    if (!matTextures.specular.empty()) ImGui::BulletText("Specular: %zu texture(s)", matTextures.specular.size());
                    if (!matTextures.normal.empty()) ImGui::BulletText("Normal: %zu texture(s)", matTextures.normal.size());
                    if (!matTextures.height.empty()) ImGui::BulletText("Height: %zu texture(s)", matTextures.height.size());
                    if (!matTextures.emission.empty()) ImGui::BulletText("Emission: %zu texture(s)", matTextures.emission.size());
                    if (!matTextures.roughness.empty()) ImGui::BulletText("Roughness: %zu texture(s)", matTextures.roughness.size());
                    if (!matTextures.metallic.empty()) ImGui::BulletText("Metallic: %zu texture(s)", matTextures.metallic.size());
                    if (!matTextures.ao.empty()) ImGui::BulletText("AO: %zu texture(s)", matTextures.ao.size());
                }
                else {
                    ImGui::Text("No model loaded");
                }

                ImGui::EndTabItem();
            }

            // Debug Console Tab
            if (ImGui::BeginTabItem("Debug Console")) {
                if (ImGui::Button("Clear Console")) {
                    ClearDebugConsole();
                }
                ImGui::SameLine();
                ImGui::Checkbox("Auto-scroll", &autoScrollDebug);

                ImGui::Separator();

                ImGui::BeginChild("DebugScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                for (const auto& message : debugMessages) {
                    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                    if (message.find("ERROR") != std::string::npos || message.find("Failed") != std::string::npos) {
                        color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                    }
                    else if (message.find("WARNING") != std::string::npos || message.find("Warning") != std::string::npos) {
                        color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
                    }
                    else if (message.find("loaded") != std::string::npos || message.find("success") != std::string::npos) {
                        color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
                    }
                    else if (message.find("Loading") != std::string::npos || message.find("Scanning") != std::string::npos) {
                        color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
                    }

                    ImGui::TextColored(color, "%s", message.c_str());
                }

                if (autoScrollDebug && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        // Lighting Controls Window - Right side
        float rightColumnX = screenWidth - rightColumnWidth - windowPadding;
        float lightingWindowHeight = screenHeight - (2 * windowPadding);

        ImGui::SetNextWindowPos(ImVec2(rightColumnX, windowPadding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(rightColumnWidth, lightingWindowHeight), ImGuiCond_Always);
        ImGui::Begin("Lighting Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable##dir", &dirLight.enabled);
            if (dirLight.enabled) {
                ImGui::SliderFloat3("Direction", &dirLight.direction.x, -1.0f, 1.0f);
                ImGui::ColorEdit3("Ambient##dir", &dirLight.ambient.x);
                ImGui::ColorEdit3("Diffuse##dir", &dirLight.diffuse.x);
                ImGui::ColorEdit3("Specular##dir", &dirLight.specular.x);
            }
        }

        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable##point", &pointLight.enabled);
            if (pointLight.enabled) {
                ImGui::SliderFloat3("Position", &pointLight.position.x, -10.0f, 10.0f);
                ImGui::ColorEdit3("Ambient##point", &pointLight.ambient.x);
                ImGui::ColorEdit3("Diffuse##point", &pointLight.diffuse.x);
                ImGui::ColorEdit3("Specular##point", &pointLight.specular.x);
                ImGui::SliderFloat("Constant", &pointLight.constant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", &pointLight.linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", &pointLight.quadratic, 0.0f, 1.0f);
            }
        }

        if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable##spot", &spotLight.enabled);
            if (spotLight.enabled) {
                ImGui::SliderFloat3("Position##spot", &spotLight.position.x, -10.0f, 10.0f);
                ImGui::SliderFloat3("Direction##spot", &spotLight.direction.x, -1.0f, 1.0f);
                ImGui::ColorEdit3("Ambient##spot", &spotLight.ambient.x);
                ImGui::ColorEdit3("Diffuse##spot", &spotLight.diffuse.x);
                ImGui::ColorEdit3("Specular##spot", &spotLight.specular.x);

                float cutOffDegrees = glm::degrees(acos(spotLight.cutOff));
                float outerCutOffDegrees = glm::degrees(acos(spotLight.outerCutOff));

                if (ImGui::SliderFloat("Inner Cutoff (degrees)", &cutOffDegrees, 0.0f, 45.0f)) {
                    spotLight.cutOff = cos(glm::radians(cutOffDegrees));
                }
                if (ImGui::SliderFloat("Outer Cutoff (degrees)", &outerCutOffDegrees, 0.0f, 45.0f)) {
                    spotLight.outerCutOff = cos(glm::radians(outerCutOffDegrees));
                }

                ImGui::SliderFloat("Constant##spot", &spotLight.constant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear##spot", &spotLight.linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic##spot", &spotLight.quadratic, 0.0f, 1.0f);
            }
        }

        if (ImGui::CollapsingHeader("Lighting Presets")) {
            if (ImGui::Button("Sunny Day", ImVec2(-1, 25))) {
                dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
                dirLight.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
                dirLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
                dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                dirLight.enabled = true;
                AddDebugMessage("Applied 'Sunny Day' lighting preset");
            }

            if (ImGui::Button("Night Scene", ImVec2(-1, 25))) {
                dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.1f);
                dirLight.diffuse = glm::vec3(0.1f, 0.1f, 0.2f);
                AddDebugMessage("Applied 'Night Scene' lighting preset");
            }

            if (ImGui::Button("Studio Lighting", ImVec2(-1, 25))) {
                dirLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
                dirLight.ambient = glm::vec3(0.4f, 0.4f, 0.4f);
                dirLight.diffuse = glm::vec3(0.6f, 0.6f, 0.6f);
                dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                dirLight.enabled = true;
                pointLight.enabled = true;
                AddDebugMessage("Applied 'Studio Lighting' preset");
            }
        }

        ImGui::End();
    }
}