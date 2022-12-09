#include <lui/SpotDiagram.hpp>

#include <lore/rt/GeometricalIntersector.h>
#include <lore/rt/SequentialTrace.h>

#include <imgui.h>
#include <implot.h>

using namespace lore;
using namespace lore::rt;

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

                value.x() = 2 * float(x) / float(cg.width - 1) - 1;
                value.y() = 2 * float(y) / float(cg.height - 1) - 1;

                value /= 1.002f; // simulate what OSLO does (???)
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

void SpotDiagram::compute(const LensSchema<float> &lensSchema) {
    using Float = double;

    subplots.clear();

    const auto lens = lensSchema.lens<Float>();

    lore::Vector2<double> firstMoment{0, 0};
    lore::Vector2<double> secondMoment{0, 0};
    double weightSum = 0;

    for (const auto &wavelength : lensSchema.wavelengths) {
        subplots.emplace_back();
        SubPlot &subplot = subplots.back();
        subplot.wavelength = wavelength.wavelength;

        const SequentialTrace<Float> trace{wavelength.wavelength};
        const GeometricalIntersector<Float> intersector;

        for (const auto &point: CircleGrid(18, 18)) {
            const auto pupil = lensSchema.entranceBeamRadius * point;
            auto ray = lore::rt::Ray<Float>(
                lore::Vector3<Float>{pupil.x(), pupil.y(), -10},
                lore::Vector3<Float>{0, 0, 1}
            );

            if (trace(ray, lens, intersector)) {
                subplot.points.push_back({
                     float(ray.origin.x()),
                     float(ray.origin.y())
                });

                const auto image = lore::Vector2<double>{
                    ray.origin.x(),
                    ray.origin.y()
                };

                const double weight = wavelength.weight;
                firstMoment += weight * image;
                secondMoment += weight * lore::sqr(image);
                weightSum += weight;
            }
        }
    }

    double norm = 1 / weightSum;
    auto meanSquare = (secondMoment * norm - lore::sqr(firstMoment * norm)).sum();
    spotRMS = float(lore::sqrt(meanSquare));
}

void SpotDiagram::draw() {
    if (!ImGui::Begin("Spot diagram")) {
        ImGui::End();
        return;
    }

    ImGui::Text("RMS: %f", spotRMS);

    ImPlot::SetNextAxesLimits(-2, +2, -2, +2);
    if (ImPlot::BeginPlot("Field 0 deg", ImVec2(-1, 0), ImPlotFlags_Equal)) {
        static ImVec4 colors[] = {
            { 0, 1, 0, 1 },
            { 0, 0, 1, 1 },
            { 1, 0, 0, 1 }
        };
        int index = 0;
        for (const auto &subplot : subplots) {
            ImPlot::SetNextMarkerStyle(
                ImPlotMarker_Square,
                1,
                colors[(index++) % 3],
                IMPLOT_AUTO,
                ImVec4(1, 1, 1, 0)
            );
            char name[256];
            snprintf(name, sizeof(name), "%.3fum", subplot.wavelength);
            ImPlot::PlotScatter(name,
                                (float *)subplot.points.data(),
                                (float *)subplot.points.data() + 1,
                                int(subplot.points.size()),
                                ImPlotScatterFlags_None,
                                0,
                                2 * sizeof(float)
            );
        }
        ImPlot::EndPlot();
    }

    ImGui::End();
}
