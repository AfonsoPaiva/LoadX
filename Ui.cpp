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

    void RenderUI() {
        // Model loading window
        ImGui::SetNextWindowPos(ImVec2(58, 48), ImGuiCond_Once);
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
        ImGui::End();

        // Lighting controls window
        ImGui::SetNextWindowPos(ImVec2(58, 200), ImGuiCond_Once);
        ImGui::Begin("Lighting Controls");

        if (ImGui::CollapsingHeader("Directional Light")) {
            ImGui::SliderFloat3("Direction", &dirLight.direction.x, -1.0f, 1.0f);
            ImGui::ColorEdit3("Ambient", &dirLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &dirLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &dirLight.specular.x);
        }

        if (ImGui::CollapsingHeader("Point Light")) {
            ImGui::SliderFloat3("Position", &pointLight.position.x, -10.0f, 10.0f);
            ImGui::ColorEdit3("Ambient", &pointLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &pointLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &pointLight.specular.x);
            ImGui::SliderFloat("Constant", &pointLight.constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &pointLight.linear, 0.0f, 1.0f);
            ImGui::SliderFloat("Quadratic", &pointLight.quadratic, 0.0f, 1.0f);
        }

        if (ImGui::CollapsingHeader("Spot Light")) {
            ImGui::SliderFloat3("Position", &spotLight.position.x, -10.0f, 10.0f);
            ImGui::SliderFloat3("Direction", &spotLight.direction.x, -1.0f, 1.0f);
            ImGui::ColorEdit3("Ambient", &spotLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &spotLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &spotLight.specular.x);
            ImGui::SliderFloat("Cutoff", &spotLight.cutOff, 0.0f, 45.0f);
            ImGui::SliderFloat("Outer Cutoff", &spotLight.outerCutOff, 0.0f, 45.0f);
            ImGui::SliderFloat("Constant", &spotLight.constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &spotLight.linear, 0.0f, 1.0f);
            ImGui::SliderFloat("Quadratic", &spotLight.quadratic, 0.0f, 1.0f);
        }

        if (ImGui::CollapsingHeader("Material")) {
            ImGui::ColorEdit3("Ambient", &material.ambient.x);
            ImGui::ColorEdit3("Diffuse", &material.diffuse.x);
            ImGui::ColorEdit3("Specular", &material.specular.x);
            ImGui::SliderFloat("Shininess", &material.shininess, 1.0f, 256.0f);
        }

        ImGui::End();
    }
}