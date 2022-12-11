#include <lui/RayInterceptCurve.hpp>
#include <lui/utils/linspace.hpp>

#include <lore/rt/GeometricalIntersector.h>
#include <lore/rt/SequentialTrace.h>

#include <imgui.h>
#include <implot.h>

using namespace lore;
using namespace lore::rt;



void RayInterceptCurve::compute(const LensSchema<float> &lensSchema) {
    using Float = double;

    subplots.clear();

    const auto lens = lensSchema.lens<Float>();

    for (const auto &wavelength: lensSchema.wavelengths) {
        subplots.emplace_back();
        SubPlot &subplot = subplots.back();
        subplot.wavelength = wavelength.wavelength;

        const GeometricalIntersector<Float> intersector;
        const SequentialTrace trace{
            lens,
            intersector,
            Float(wavelength.wavelength)
        };

        for (const auto &point: linspace(-1, +1, 200)) {
            const auto pupil = lore::Vector2<float>{ 0, lensSchema.entranceBeamRadius * point };
            auto ray = lore::rt::Ray<Float>(
                lore::Vector3<Float>{0, pupil.y(), -10},
                lore::Vector3<Float>{0, 0, 1}.normalized()
            );

            if (trace(ray)) {
                subplot.points.push_back({
                                             float(point),
                                             float(ray.origin.y())
                                         });
            }
        }
    }
}

void RayInterceptCurve::draw() {
    if (!ImGui::Begin("Ray Intercept Curve")) {
        ImGui::End();
        return;
    }

    if (ImPlot::BeginPlot("Tangential 0 deg")) {
        static ImVec4 colors[] = {
            {0, 1, 0, 1},
            {0, 0, 1, 1},
            {1, 0, 0, 1}
        };
        int index = 0;
        for (const auto &subplot: subplots) {
            ImPlot::SetNextLineStyle(colors[index++ % 3]);
            char name[256];
            snprintf(name, sizeof(name), "%.3fum", subplot.wavelength);
            ImPlot::PlotLine(name,
                             (float *) subplot.points.data(),
                             (float *) subplot.points.data() + 1,
                             int(subplot.points.size()),
                             ImPlotLineFlags_None,
                             0,
                             2 * sizeof(float)
            );
        }
        ImPlot::EndPlot();
    }

    ImGui::End();
}
