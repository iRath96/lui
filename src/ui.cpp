#include <lui/ui.hpp>
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

    std::ifstream file("../ext/lore/data/lenses/simple.len");
    lore::io::LensReader reader;
    auto result = reader.read(file);
    lens = result.front();

    spotDiagram.compute(lens);
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

    lensChanged |= ImGui::DragFloat(
        "BFL",
        &lens.surfaces[lens.surfaces.size()-2].thickness,
        0.001f,
        -1000,
        +1000,
        "BFL: %.2fmm"
    );

    ImGui::Text("TTL: %f", trackLength);

    const ImVec2 targetSize = { 600, 400 };
    const float scale = std::min(targetSize.x / trackLength, targetSize.y / (2 * maxAperture));
    const ImVec2 actualSize = { trackLength * scale, 2 * maxAperture * scale };

    ImTransform2 transform;
    transform.scale = { scale, scale };
    transform.offset = {
        ImGui::GetCursorScreenPos().x,
        ImGui::GetCursorScreenPos().y + transform.scale.y * maxAperture
    };

    float z = 0;
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

    ImGui::Text("z = %f", z);
}

void UI::draw() {
    lensChanged = false;

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
        drawLens(lens);
        ImGui::End();
    }

    if (lensChanged) {
        spotDiagram.compute(lens);
    }

    spotDiagram.draw();
}

/*class CircleGrid : public std::iterator<
    std::input_iterator_tag,
    lore::Vector2<float>
> {
    void advance() {
        while (!done) {
            value.x() = float(x) / width;
            value.y() = float(y) / height;

            if (value.lengthSquared() < 1) {
                break;
            }

            if (x++ >= width) {
                x = 0;
                if (y++ >= height) {
                    done = true;
                    y = 0;
                }
            }
        }
    }

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = lore::Vector2<float>;
    using reference = value_type const &;
    using pointer = value_type const *;
    using difference_type = ptrdiff_t;

    CircleGrid(int width, int height)
    : width(width), height(height), x(0), y(0), done(false) {
        advance();
    }

    explicit operator bool() const { return !done; }

    reference operator*() const { return value; }
    pointer operator->() const { return &value; }

    CircleGrid &operator++() {
        advance();
        return *this;
    }

private:
    bool done;
    int width, height;
    int x, y;

    lore::Vector2<float> value;
};*/

struct CircleGrid {
    struct iterator {
        iterator(const CircleGrid &cg, bool) : done(true), cg(cg) {}
        iterator(const CircleGrid &cg)
        : done(false), x(-1), y(0), cg(cg) {
            advance();
        }

        const lore::Vector2<float> &operator*() const { return value; }
        iterator &operator++() {
            advance();
            return *this;
        }

        bool operator!=(const iterator &other) const {
            return done != other.done;
        }

    private:
        lore::Vector2<float> value;

        bool done;
        int x, y;
        const CircleGrid &cg;

        void advance() {
            while (!done) {
                if (++x >= cg.width) {
                    x = 0;
                    if (++y >= cg.height) {
                        done = true;
                        y = 0;
                    }
                }

                value.x() = 2 * float(x) / (cg.width - 1) - 1;
                value.y() = 2 * float(y) / (cg.height - 1) - 1;

                if (value.lengthSquared() <= 1) {
                    break;
                }
            }
        }
    };

    CircleGrid(int width, int height)
    : width(width), height(height) {}

    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(*this, true); }

private:
    int width, height;
};

void SpotDiagram::compute(const lore::LensSchema<float> &lensSchema) {
    using Float = float;

    points.clear();

    const lore::rt::SequentialTrace<Float> trace { lensSchema.primaryWavelength() };
    const lore::rt::GeometricalIntersector<Float> intersector;

    const auto lens = lensSchema.lens<Float>();

    const int Ntheta = 32;
    const int Nradius = 32;

    lore::Vector2<double> firstMoment { 0, 0 };
    lore::Vector2<double> secondMoment { 0, 0 };

    for (const auto &point : CircleGrid(16, 16)) {
        const auto pupil = lensSchema.entranceBeamRadius * point;
        auto ray = lore::rt::Ray<Float>(
            lore::Vector3<Float> { pupil.x(), pupil.y(), -10 },
            lore::Vector3<Float> { 0, 0, 1 }
        );

        if (trace(ray, lens, intersector)) {
            points.push_back({
                float(ray.origin.x()),
                float(ray.origin.y())
            });

            const auto image = lore::Vector2<double> {
                ray.origin.x(),
                ray.origin.y()
            };
            firstMoment += image;
            secondMoment += lore::sqr(image);
        }
    }

    double countNorm = 1 / double(points.size());
    auto meanSquare = (secondMoment * countNorm - lore::sqr(firstMoment * countNorm)).sum();
    spotRMS = float(lore::sqrt(meanSquare));
}

void SpotDiagram::draw() {
    if (!ImGui::Begin("Spot diagram")) {
        ImGui::End();
        return;
    }

    ImGui::Text("%ld Points", points.size());
    ImGui::Text("RMS: %f", spotRMS);

    ImPlot::SetNextAxesLimits(-2, +2, -2, +2);
    if (ImPlot::BeginPlot("Field 0 deg", ImVec2(-1, 0), ImPlotFlags_Equal)) {
        ImPlot::SetNextMarkerStyle(
            ImPlotMarker_Square,
            1,
            ImVec4(1, 1, 1, 1),
            IMPLOT_AUTO,
            ImVec4(1, 1, 1, 0)
        );
        ImPlot::PlotScatter("Primary",
            (float *)points.data(),
            (float *)points.data() + 1,
            int(points.size()),
            ImPlotScatterFlags_None,
            0,
            2 * sizeof(float)
        );
        ImPlot::EndPlot();
    }

    ImGui::End();
}
