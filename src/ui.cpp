#include <lui/ui.hpp>
#include <imgui.h>
#include <implot.h>

void UI::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Demos")) {
            ImGui::MenuItem("ImGui demo", nullptr, &showImguiDemo);
            ImGui::MenuItem("ImPlot demo", nullptr, &showImplotDemo);
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }

    if (showImguiDemo) ImGui::ShowDemoWindow(&showImguiDemo);
    if (showImplotDemo) ImPlot::ShowDemoWindow();

    if (ImGui::Begin("lui")) {
        ImGui::Text("Hello");
        ImGui::End();
    }
}