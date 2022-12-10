#include <lui/UI.hpp>
#include <imgui.h>
#include <implot.h>

#include <lore/io/LensReader.h>
#include <lore/rt/GeometricalIntersector.h>
#include <lore/rt/SequentialTrace.h>

#include <fstream>

UI::UI() {
    lore::GlassCatalog::shared.read("../ext/lore/data/glass/schott.glc");
    lore::GlassCatalog::shared.read("../ext/lore/data/glass/obsolete001.glc");
    lore::GlassCatalog::shared.read("../ext/lore/data/glass/hoya.glc");

    std::ifstream file("../ext/lore/data/lenses/wideangle.len");
    lore::io::LensReader reader;
    auto result = reader.read(file);
    lens = result.front();
    lensChanged = true;
}

struct ImTransform2 {
    ImVec2 offset;
    ImVec2 scale;

    ImVec2 operator()(const ImVec2 &p) const {
        return ImVec2 {
            offset.x + p.x * scale.x,
            offset.y + p.y * scale.y
        };
    }

    ImVec2 operator()(float x, float y) const {
        return (*this)(ImVec2(x, y));
    }
};

void drawStop(
    float aperture,
    const ImTransform2 &transform
) {
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    for (int sign = -1; sign <= 1; sign += 2) {
        drawList->AddLine(
            transform(0, float(sign) * aperture),
            transform(0, float(sign) * (aperture + 4)),
            IM_COL32_WHITE
        );
        drawList->AddLine(
            transform(-2, float(sign) * aperture),
            transform(+2, float(sign) * aperture),
            IM_COL32_WHITE
        );
    }
    //drawList->AddLine(
    //    transform(0, -aperture),
    //    transform(0, +aperture),
    //    IM_COL32(255, 255, 255, 64)
    //);
}

void drawElement(
    const lore::SurfaceSchema<float> &left,
    const lore::SurfaceSchema<float> &right,
    const ImTransform2 &transform
) {
    static std::vector<ImVec2> buffer;
    buffer.clear();

    const float maxAperture = std::max(left.aperture, right.aperture);

    // MARK: - left surface
    if (left.isFlat()) {
        buffer.push_back(transform(0, 0));
        buffer.push_back(transform(0, maxAperture));
    } else {
        buffer.push_back(transform(0, 0));

        const float absRadius = std::abs(left.radius);

        ImTransform2 transform2;
        transform2.offset = transform.offset;
        transform2.scale = ImVec2 { transform.scale.x * left.radius, transform.scale.y * absRadius };

        const int N = 32;
        const float angle = std::asin(left.aperture / absRadius);
        const float step = angle / float(N);
        for (int i = 1; i < N; i++) {
            const float a = float(i) * step;
            buffer.push_back(transform2(1 - std::cos(a), std::sin(a)));
        }

        const float sag = left.getSag();
        buffer.push_back(transform(sag, left.aperture));
        if (left.aperture < maxAperture) {
            buffer.push_back(transform(sag, maxAperture));
        }
    }

    // MARK: - right surface
    const float sag = left.thickness + right.getSag();
    buffer.push_back(transform(sag, maxAperture));
    if (right.isFlat()) {
        buffer.push_back(transform(sag, 0));
    } else {
        if (right.aperture < maxAperture) {
            buffer.push_back(transform(sag, right.aperture));
        }

        const float absRadius = std::abs(right.radius);

        ImTransform2 transform2;
        transform2.offset = transform.offset;
        transform2.offset.x += left.thickness * transform.scale.x;
        transform2.scale = ImVec2 { transform.scale.x * right.radius, transform.scale.y * absRadius };

        const int N = 32;
        const float angle = std::asin(right.aperture / absRadius);
        const float step = angle / float(N);
        for (int i = N - 1; i > 0; i--) {
            const float a = float(i) * step;
            buffer.push_back(transform2(1 - std::cos(a), std::sin(a)));
        }

        buffer.push_back(transform2(0, 0));
    }

    // MARK: - mirror
    const int N = buffer.size();
    buffer.reserve(2 * (N - 1));

    for (int i = N - 2; i > 0; i--) {
        const auto &p = buffer[i];
        buffer.emplace_back(p.x, 2 * transform.offset.y - p.y);
    }

    // MARK: - draw
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->AddPolyline(buffer.data(), buffer.size(), IM_COL32_WHITE, ImDrawFlags_Closed, 1);
}

void UI::drawLens(lore::LensSchema<float> &lens) {
    ImGui::Text("Lens: %s", lens.name.c_str());

    float trackLength = 0;
    float maxAperture = 0;
    for (int i = 1; i < lens.surfaces.size() - 1; i++) {
        const auto &s = lens.surfaces[i];
        trackLength += s.thickness;
        maxAperture = std::max(maxAperture, s.aperture);
    }

    auto &objectDistance = lens.surfaces.front().thickness;
    auto &objectHeight = lens.surfaces.front().aperture;
    lensChanged |= ImGui::DragFloat(
        "OD",
        &objectDistance,
        abs(objectDistance) / 1000,
        -1e+20,
        +1e+20,
        "OD: %.3gmm"
    );
    objectHeight = objectDistance * std::tan(lens.fieldAngle * M_PI / 180);

    lensChanged |= ImGui::DragFloat(
        "BFL",
        &lens.surfaces[lens.surfaces.size()-2].thickness,
        0.01f,
        -1000,
        +1000,
        "BFL: %.2fmm"
    );

    ImGui::Text("TTL: %f", trackLength);

    const ImVec2 targetSize = { 500, 300 };
    const float padding = 4;
    const ImVec2 lensSize = {
        trackLength + 2 * padding,
        2 * maxAperture + 2 * padding
    };
    const float scale = std::min(targetSize.x / lensSize.x, targetSize.y / lensSize.y);
    const ImVec2 actualSize = { lensSize.x * scale, lensSize.y * scale };

    ImTransform2 transform;
    transform.scale = { scale, scale };
    transform.offset = {
        ImGui::GetCursorScreenPos().x + transform.scale.x * padding,
        ImGui::GetCursorScreenPos().y + transform.scale.y * (maxAperture + padding)
    };

    for (int i = 1; i < lens.surfaces.size() - 1; i++) {
        const auto &surface = lens.surfaces[i];
        if (i == lens.stopIndex) {
            drawStop(surface.aperture, transform);
        }
        if (!surface.glass.isAir()) {
            drawElement(surface, lens.surfaces[i + 1], transform);
        }
        transform.offset.x += transform.scale.x * surface.thickness;
    }
    ImGui::Dummy(actualSize);
}

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
        drawLens(lens);
    }
    ImGui::End();

    if (lensChanged) {
        spotDiagram.compute(lens);
        rayInterceptCurve.compute(lens);
        paraxialConstants.compute(lens);

        lensChanged = false;
    }

    spotDiagram.draw();
    rayInterceptCurve.draw();
    paraxialConstants.draw();
}
