#include <lui/SurfaceData.hpp>

#include <imgui.h>

using namespace lore;

void SurfaceData::draw(lore::LensSchema<float> &lens, bool &lensChanged) {
    if (!ImGui::Begin("Surface Data")) {
        ImGui::End();
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    lens.name.reserve(128);
    ImGui::InputText("Name", &lens.name.front(), lens.name.capacity());

    ImGui::PushItemWidth(80);
    lensChanged |= ImGui::InputFloat("Ent. beam radius", &lens.entranceBeamRadius);
    ImGui::SameLine();
    if (ImGui::InputFloat("Field angle", &lens.fieldAngle)) {
        lensChanged = true;

        auto &objectDistance = lens.surfaces.front().thickness;
        auto &objectHeight = lens.surfaces.front().aperture;
        objectHeight = objectDistance * std::tan(lens.fieldAngle * M_PI / 180);
    }
    ImGui::PopItemWidth();

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV |
        ImGuiTableFlags_RowBg;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 2));
    if (ImGui::BeginTable("table1", 5, flags)) {
        ImGui::TableSetupColumn("SRF");
        ImGui::TableSetupColumn("RADIUS");
        ImGui::TableSetupColumn("THICKNESS");
        ImGui::TableSetupColumn("APERTURE RADIUS");
        ImGui::TableSetupColumn("GLASS");
        ImGui::TableHeadersRow();

        for (int row = 0; row < lens.surfaces.size(); row++) {
            ImGui::PushID(row);

            auto &surface = lens.surfaces[row];
            ImGui::TableNextRow();

            // MARK: SRF
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
            if (row == lens.stopIndex) {
                ImGui::Text("AST");
            } else if (row == lens.surfaces.size() - 1) {
                ImGui::Text("IMS");
            } else if (row == 0) {
                ImGui::Text("OBJ");
            } else {
                ImGui::Text("%3d", row);
            }

            // MARK: RADIUS
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-1);
            lensChanged |= ImGui::DragFloat(
                "##radius",
                &surface.radius,
                abs(surface.radius) / 1000,
                0, 0,
                "%.6g");
            ImGui::PopItemWidth();

            // MARK: THICKNESS
            ImGui::TableSetColumnIndex(2);
            ImGui::PushItemWidth(-1);
            lensChanged |= ImGui::DragFloat(
                "##thickness",
                &surface.thickness,
                0.01f,
                0, 0,
                "%.6g");
            ImGui::PopItemWidth();

            // MARK: APERTURE RADIUS
            ImGui::TableSetColumnIndex(3);
            ImGui::PushItemWidth(-1);
            lensChanged |= ImGui::DragFloat(
                "##aperture",
                &surface.aperture,
                0.01f,
                0, 0,
                "%.6g");
            ImGui::PopItemWidth();

            // MARK: APERTURE RADIUS
            ImGui::TableSetColumnIndex(4);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
            ImGui::Text("%s", surface.glassName.c_str());

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(2);
    ImGui::PopFont();

    ImGui::End();
}
