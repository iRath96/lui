#include <lui/SurfaceData.hpp>

#include <lore/lens/GlassCatalog.h>

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

    ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV |
        ImGuiTableFlags_RowBg;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    if (ImGui::BeginTable("table1", 5, flags)) {
        apertureFlags.reserve(lens.surfaces.size());

        ImGui::TableSetupColumn("SRF");
        ImGui::TableSetupColumn("RADIUS");
        ImGui::TableSetupColumn("THICKNESS");
        ImGui::TableSetupColumn("APERTURE RADIUS");
        ImGui::TableSetupColumn("GLASS");
        ImGui::TableHeadersRow();

        int addSurfaceIndex = -1;
        int removeSurfaceIndex = -1;

        for (int row = 0; row < lens.surfaces.size(); row++) {
            ImGui::PushID(row);

            auto &surface = lens.surfaces[row];
            const bool surfaceIsStop = row == lens.stopIndex;
            const bool surfaceIsObj = row == 0;
            const bool surfaceIsImage = row == int(lens.surfaces.size()) - 1;
            ImGui::TableNextRow();

            // MARK: SRF
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
            if (surfaceIsStop) ImGui::Text("AST");
            else if (surfaceIsImage) ImGui::Text("IMS");
            else if (surfaceIsObj) ImGui::Text("OBJ");
            else ImGui::Text("%3d", row);

            if (ImGui::BeginPopupContextItem("row_actions")) {
                if (!surfaceIsObj && ImGui::MenuItem("Insert Before")) {
                    addSurfaceIndex = row;
                }
                if (!surfaceIsImage && ImGui::MenuItem("Insert After")) {
                    addSurfaceIndex = row + 1;
                }
                if (!surfaceIsImage && !surfaceIsObj && ImGui::MenuItem("Delete")) {
                    removeSurfaceIndex = row;
                }
                ImGui::EndPopup();
            }

            // MARK: RADIUS
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-30);
            lensChanged |= ImGui::DragFloat(
                "##radius",
                &surface.radius,
                abs(surface.radius) / 1000 + 0.001f,
                0, 0,
                "%.6g");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Button("##radiusFlags", ImVec2(30, 0));

            // MARK: THICKNESS
            ImGui::TableSetColumnIndex(2);
            ImGui::PushItemWidth(-30);
            lensChanged |= ImGui::DragFloat(
                "##thickness",
                &surface.thickness,
                0.01f,
                0, 0,
                "%.6g");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Button("##thicknessFlags", ImVec2(30, 0));

            // MARK: APERTURE RADIUS
            ImGui::TableSetColumnIndex(3);
            ImGui::PushItemWidth(-30);
            lensChanged |= ImGui::DragFloat(
                "##aperture",
                &surface.aperture,
                0.01f,
                0, 1e+20,
                "%.6g");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            if (row > 0) {
                auto &aFlags = apertureFlags[row];
                aFlags = "  ##aflags";
                char *index = aFlags.data() + 2;
                if (row == lens.stopIndex) *--index = 'A';
                if (surface.checkAperture) *--index = 'K';

                if (ImGui::Button(aFlags.c_str(), ImVec2(30, 0))) {
                    ImGui::OpenPopup("apertureFlags");
                }
            }

            if (ImGui::BeginPopup("apertureFlags")) {
                lensChanged |= ImGui::MenuItem("Checked (K)", "", &surface.checkAperture);
                if (ImGui::MenuItem("Aperture Stop (A)", "", row == lens.stopIndex)) {
                    lens.stopIndex = row;
                    lensChanged = true;
                }
                ImGui::EndPopup();
            }

            // MARK: GLASS
            ImGui::TableSetColumnIndex(4);
            if (row < int(lens.surfaces.size()) - 1) {
                ImGui::Button(surface.glassName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                ImGui::OpenPopupOnItemClick("glass_selector", ImGuiPopupFlags_MouseButtonLeft);
                if (ImGui::BeginPopupContextItem("glass_selector")) {
                    if (ImGui::MenuItem("AIR")) {
                        surface.glassName = "AIR";
                        surface.glass = Glass<float>::air();
                        lensChanged = true;
                    }

                    for (auto &catalog : lore::GlassCatalog::shared.byCatalog) {
                        if (ImGui::BeginMenu(catalog.first.c_str())) {
                            for (auto &glassName : catalog.second) {
                                if (ImGui::MenuItem(glassName.c_str())) {
                                    surface.glassName = glassName;
                                    surface.glass = lore::GlassCatalog::shared.glass(glassName);
                                    lensChanged = true;
                                }
                            }
                            ImGui::EndMenu();
                        }
                    }

                    ImGui::EndPopup();
                }
            }

            ImGui::PopID();
        }
        ImGui::EndTable();

        if (addSurfaceIndex >= 0) {
            if (addSurfaceIndex <= lens.stopIndex) lens.stopIndex++;

            SurfaceSchema<float> surface;
            surface.glassName = "AIR";
            surface.checkAperture = false;
            surface.thickness = 0;
            surface.radius = 0;
            surface.aperture = 0;
            surface.glass = Glass<float>::air();

            lens.surfaces.insert(lens.surfaces.begin() + addSurfaceIndex, surface);
        }

        if (removeSurfaceIndex >= 0) {
            if (removeSurfaceIndex < lens.stopIndex) lens.stopIndex--;
            lens.surfaces.erase(lens.surfaces.begin() + removeSurfaceIndex);
        }
    }
    ImGui::PopStyleVar(3);
    ImGui::PopFont();

    ImGui::End();
}
