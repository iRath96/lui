#include <cassert>

#include <lui/SpotDiagram.hpp>
#include <lui/utils/CircleGrid.hpp>

#include <lore/rt/GeometricalIntersector.h>
#include <lore/rt/SequentialTrace.h>
#include <lore/optim/FADFloat.h>

#include <imgui.h>
#include <implot.h>

using namespace lore;
using namespace lore::rt;

void SpotDiagram::compute(const LensSchema<float> &lensSchema) {
    using Float = optim::FADFloat<double, 1>;

    subplots.clear();

    auto lens = lensSchema.lens<Float>();
    lens.surfaces[lens.surfaces.size() - 2].thickness.dVd(0) = 1;

    lore::Vector2<Float> firstMoment{0, 0};
    lore::Vector2<Float> secondMoment{0, 0};
    Float weightSum = 0;

    for (const auto &wavelength : lensSchema.wavelengths) {
        subplots.emplace_back();
        SubPlot &subplot = subplots.back();
        subplot.wavelength = wavelength.wavelength;

        const GeometricalIntersector<Float> intersector;
        const SequentialTrace trace{
            lens,
            intersector,
            Float(wavelength.wavelength)
        };

        for (const auto &point: CircleGrid(18, 18)) {
            const auto pupil = lensSchema.entranceBeamRadius * point;
            auto ray = lore::rt::Ray<Float>(
                lore::Vector3<Float>{pupil.x(), pupil.y(), -10},
                lore::Vector3<Float>{0, 0, 1}
            );

            if (trace(ray)) {
                subplot.points.push_back({
                     float(detach(ray.origin.x())),
                     float(detach(ray.origin.y()))
                });

                const auto image = lore::Vector2<Float>{
                    ray.origin.x(),
                    ray.origin.y()
                };

                const Float weight = wavelength.weight;
                firstMoment += weight * image;
                secondMoment += weight * lore::sqr(image);
                weightSum += weight;
            }
        }
    }

    Float norm = Float(1) / weightSum;
    auto meanSquare = (secondMoment * norm - sqr(firstMoment * norm)).sum();
    auto rms = sqrt(meanSquare);

    spotRMS = float(detach(rms));
    autodiff = rms.dVd(0);
}

void SpotDiagram::draw() {
    if (!ImGui::Begin("Spot Diagram")) {
        ImGui::End();
        return;
    }

    ImGui::Text("RMS: %f\ndVd: %f", spotRMS, autodiff);

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
