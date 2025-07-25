#include "ui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "lighting.h"

// Lighting controls
extern DirectionalLight dirLight;
extern PointLight pointLight;
extern SpotLight spotLight;
extern Material material;

namespace UI {
    std::string selectedModelPath;
    bool modelSelected = false;
    bool resetCameraPosition = false;

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

    void RenderUI(Transform& modelTransform) {
        // Model loading window
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
        ImGui::Begin("Model Loader");

        if (ImGui::Button("Open File Dialog"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Model File", ".obj,.fbx,.gltf,.glb,.3ds,.dae,.blend,.x3d,.ply,.stl");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                selectedModelPath = ImGuiFileDialog::Instance()->GetFilePathName();
                modelSelected = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (!selectedModelPath.empty()) {
            ImGui::Text("Loaded: %s", selectedModelPath.c_str());
        }

        ImGui::End();

        // Object Transform Controls
        ImGui::SetNextWindowPos(ImVec2(10, 170), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_Once);
        ImGui::Begin("Object Transform");

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
            modelTransform.scale = glm::vec3(0.1f, 0.1f, 0.1f);
            uniformScale = 0.1f;
        }

        ImGui::SameLine();

        if (ImGui::Button("Normal Size")) {
            modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            uniformScale = 1.0f;
        }

        ImGui::End();

        // Lighting controls window
        ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(350, 600), ImGuiCond_Once);
        ImGui::Begin("Lighting Controls");

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

        ImGui::End();

        // Camera Controls Info
        ImGui::SetNextWindowPos(ImVec2(680, 10), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Once);
        ImGui::Begin("Camera Controls");

        ImGui::Text("Controls:");
        ImGui::BulletText("WASD - Move camera");
        ImGui::BulletText("Mouse + Left Click - Look around");
        ImGui::BulletText("Scroll - Zoom in/out");

        ImGui::Separator();

        if (ImGui::Button("Reset Camera Position")) {
            resetCameraPosition = true;
        }

        ImGui::End();
    }
}