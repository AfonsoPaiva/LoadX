#include "ui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "lighting.h"
#include <ctime>

// Lighting controls
extern DirectionalLight dirLight;
extern PointLight pointLight;
extern SpotLight spotLight;
extern Material material;

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

    void Init(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
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
        // Get current viewport size
        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        const float windowPadding = 10.0f;
        const float leftColumnWidth = 300.0f;
        const float middleColumnWidth = 400.0f;
        const float rightColumnWidth = 350.0f;

        // Model loading window
        ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftColumnWidth, 250), ImGuiCond_Always);
        ImGui::Begin("Model Loader", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Model file selection
        ImGui::Text("Model File:");
        if (ImGui::Button("Select Model File", ImVec2(-1, 25)))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Model File", ".obj,.fbx,.gltf,.glb,.3ds,.dae,.blend,.x3d,.ply,.stl");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                selectedModelPath = ImGuiFileDialog::Instance()->GetFilePathName();
                modelSelected = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (!selectedModelPath.empty()) {
            ImGui::TextWrapped("Model: %s", selectedModelPath.c_str());
        }

        // Show loading progress if model is loading
        if (currentModel && currentModel->IsLoading()) {
            ImGui::Separator();
            ImGui::Text("Loading model...");
            ImGui::ProgressBar(currentModel->GetLoadingProgress(), ImVec2(-1, 0));
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
                }
                ImGuiFileDialog::Instance()->Close();
            }

            if (!selectedMtlPath.empty()) {
                ImGui::TextWrapped("MTL: %s", selectedMtlPath.c_str());
            }

            if (ImGui::Button("Clear MTL File", ImVec2(-1, 20))) {
                selectedMtlPath = "";
            }

            ImGui::Separator();

            // Show status
            if (currentModel->HasMtlFile()) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: OBJ with MTL materials");
            }
            else {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Status: OBJ with default materials");
                ImGui::TextWrapped("Load an MTL file for proper materials and textures");
            }
            ImGui::Separator();
            ImGui::Text("UV Coordinate Debugging:");

        }
        else if (currentModel && !currentModel->IsObjFile()) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Non-OBJ format (materials included)");
        }

        ImGui::End();

        // Camera Controls - Adjusted position for larger model loader window
        ImGui::SetNextWindowPos(ImVec2(windowPadding, 270), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftColumnWidth, 200), ImGuiCond_Always);
        ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Camera movement toggle button
        if (cameraMovementEnabled) {
            if (ImGui::Button("Disable Camera Movement", ImVec2(-1, 30))) {
                cameraMovementEnabled = false;
            }
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Camera Movement: ENABLED");
        }
        else {
            if (ImGui::Button("Enable Camera Movement", ImVec2(-1, 30))) {
                cameraMovementEnabled = true;
            }
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Camera Movement: DISABLED");
        }

        ImGui::Separator();

        // Screenshot button
        if (ImGui::Button("Take Screenshot", ImVec2(-1, 35))) {
            takeScreenshot = true;
        }

        static std::string lastScreenshotPath = "";
        if (!lastScreenshotPath.empty()) {
            ImGui::TextWrapped("Last screenshot: %s", lastScreenshotPath.c_str());
        }

        ImGui::Separator();

        ImGui::Text("Controls (when enabled):");
        ImGui::BulletText("WASD - Move camera");
        ImGui::BulletText("Mouse + Left Click - Look around");
        ImGui::BulletText("Scroll - Zoom in/out");

        if (ImGui::Button("Reset Camera Position")) {
            resetCameraPosition = true;
        }

        ImGui::End();

        // Object Transform Controls - Adjusted for new window positions
        float transformWindowHeight = screenHeight - 490; // Adjusted for larger model loader
        ImGui::SetNextWindowPos(ImVec2(windowPadding, 480), ImGuiCond_Always);
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
        // Uniform scale slider
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
        }

        ImGui::SameLine();

        if (ImGui::Button("Normal Size")) {
            modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            uniformScale = 1.0f;
        }

        ImGui::End();

        // Texture Management Window - Middle column (rest of the code remains the same)
        if (currentModel) {
            float middleColumnX = leftColumnWidth + (2 * windowPadding);
            float textureWindowHeight = screenHeight - (2 * windowPadding);

            ImGui::SetNextWindowPos(ImVec2(middleColumnX, windowPadding), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(middleColumnWidth, textureWindowHeight), ImGuiCond_Always);
            ImGui::Begin("Texture Manager", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

            // Show model type information
            if (currentModel->IsObjFile()) {
                if (currentModel->HasMtlFile()) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "OBJ Model with MTL materials loaded");
                }
                else {
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "OBJ Model without MTL file");
                    ImGui::Text("Consider loading an MTL file for proper materials");
                }
            }
            else {
                ImGui::TextColored(ImVec4(0, 0.8f, 1, 1), "Non-OBJ Model (materials embedded)");
            }

            ImGui::Separator();

            // Auto-load textures from folder section
            ImGui::Text("Auto-Load Textures from Folder");
            ImGui::Separator();

            if (ImGui::Button("Select Texture Folder", ImVec2(-1, 30))) {
                IGFD::FileDialogConfig config;
                config.path = ".";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFolderDlgKey", "Choose Texture Folder", nullptr, config);
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseFolderDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    selectedTextureFolder = ImGuiFileDialog::Instance()->GetCurrentPath();
                    textureFolderSelected = true;
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

            ImGui::Text("Manual Texture Loading");
            ImGui::Separator();

            // Helper lambda for texture loading buttons
            auto CreateTextureLoadButton = [&](const char* label, const char* dialogKey, const char* textureType) {
                if (ImGui::Button(label, ImVec2(-1, 25))) {
                    ImGuiFileDialog::Instance()->OpenDialog(dialogKey, "Choose Texture", ".png,.jpg,.jpeg,.tga,.bmp,.hdr");
                }

                if (ImGuiFileDialog::Instance()->Display(dialogKey)) {
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string texturePath = ImGuiFileDialog::Instance()->GetFilePathName();
                        currentModel->AddCustomTexture(texturePath, textureType);
                        textureUpdated = true;
                    }
                    ImGuiFileDialog::Instance()->Close();
                }
                };

            // Diffuse/Albedo/BaseColor
            ImGui::Text("Diffuse/Albedo/BaseColor Map:");
            CreateTextureLoadButton("Load Diffuse", "DiffuseTexture", "texture_diffuse");

            ImGui::Separator();

            // Specular
            ImGui::Text("Specular Map:");
            CreateTextureLoadButton("Load Specular", "SpecularTexture", "texture_specular");

            ImGui::Separator();

            // Normal
            ImGui::Text("Normal Map:");
            CreateTextureLoadButton("Load Normal", "NormalTexture", "texture_normal");

            ImGui::Separator();

            // Height/Displacement
            ImGui::Text("Height/Displacement Map:");
            CreateTextureLoadButton("Load Height", "HeightTexture", "texture_height");

            ImGui::Separator();

            // Emission
            ImGui::Text("Emission Map:");
            CreateTextureLoadButton("Load Emission", "EmissionTexture", "texture_emission");

            ImGui::Separator();

            // Roughness
            ImGui::Text("Roughness Map:");
            CreateTextureLoadButton("Load Roughness", "RoughnessTexture", "texture_roughness");

            ImGui::Separator();

            // Metallic
            ImGui::Text("Metallic Map:");
            CreateTextureLoadButton("Load Metallic", "MetallicTexture", "texture_metallic");

            ImGui::Separator();

            // Ambient Occlusion
            ImGui::Text("Ambient Occlusion Map:");
            CreateTextureLoadButton("Load AO", "AOTexture", "texture_ao");

            ImGui::Separator();

            if (ImGui::Button("Clear All Custom Textures", ImVec2(-1, 30))) {
                currentModel->ClearCustomTextures();
                textureUpdated = true;
            }

            // Display current textures info
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

            ImGui::End();
        }

        // Lighting Controls Window - Sticky to right side of screen
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

        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Ambient##mat", &material.ambient.x);
            ImGui::ColorEdit3("Diffuse##mat", &material.diffuse.x);
            ImGui::ColorEdit3("Specular##mat", &material.specular.x);
            ImGui::SliderFloat("Shininess", &material.shininess, 1.0f, 256.0f);
        }

        // Add lighting presets section
        if (ImGui::CollapsingHeader("Lighting Presets")) {
            if (ImGui::Button("Sunny Day", ImVec2(-1, 25))) {
                dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
                dirLight.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
                dirLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
                dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                dirLight.enabled = true;
            }

            if (ImGui::Button("Night Scene", ImVec2(-1, 25))) {
                dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.1f);
                dirLight.diffuse = glm::vec3(0.1f, 0.1f, 0.2f);
                dirLight.specular = glm::vec3(0.2f, 0.2f, 0.3f);
                pointLight.ambient = glm::vec3(0.1f, 0.1f, 0.05f);
                pointLight.diffuse = glm::vec3(0.8f, 0.6f, 0.2f);
                pointLight.enabled = true;
            }

            if (ImGui::Button("Studio Lighting", ImVec2(-1, 25))) {
                dirLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
                dirLight.ambient = glm::vec3(0.4f, 0.4f, 0.4f);
                dirLight.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
                dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                dirLight.enabled = true;
                pointLight.enabled = false;
                spotLight.enabled = false;
            }
        }

        ImGui::End();

        // Update last screenshot path for display
        if (takeScreenshot) {
            // This will be handled in main, just show the feedback here
            lastScreenshotPath = "screenshots/screenshot_" + std::to_string(time(nullptr)) + ".png";
        }
    }
}