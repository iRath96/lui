#include <cassert>

#include <lui/RayAimer.hpp>
#include <lui/utils/linspace.hpp>

#include <lore/rt/GeometricalIntersector.h>
#include <lore/rt/SequentialTrace.h>
#include <lore/optim/FADFloat.h>

#include <imgui.h>
#include <implot.h>

using namespace lore;
using namespace lore::rt;

void RayAimer::compute(const LensSchema<float> &lensSchema) {
    using Float = lore::optim::FADFloat<double, 2>;

    points.clear();

    const auto lens = lensSchema.lens<Float>();

    const GeometricalIntersector<Float> intersector;
    const SequentialTrace trace{
        lens,
        intersector,
        Float(lensSchema.primaryWavelength()),
        1,
        lensSchema.stopIndex
    };

    for (const auto &point: linspace(-1, +1, 200)) {
        auto pupil = lore::Vector2<Float>{0, lensSchema.entranceBeamRadius * point};
        pupil.x().dVd(0) = 1;
        pupil.y().dVd(1) = 1;

        auto ray = lore::rt::Ray<Float>(
            lore::Vector3<Float>{pupil.x(), pupil.y(), -10},
            lore::Vector3<Float>{0, 0, 1}.normalized()
        );

        if (trace(ray)) {
            points.push_back({
                                 float(detach(pupil.y())),
                                 float(detach(ray.origin.y())),
                                 float(ray.origin.y().dVd(1))
                             });
        }
    }
}

void RayAimer::draw() {
    if (!ImGui::Begin("Ray Aimer")) {
        ImGui::End();
        return;
    }

    if (ImPlot::BeginPlot("Tangential 0 deg")) {
        ImPlot::SetupAxes("X-Axis 1", "Y-Axis 1");
        ImPlot::SetupAxis(ImAxis_Y2, "Y-Axis 2",ImPlotAxisFlags_AuxDefault);

        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
        ImPlot::PlotLine("Aperture position",
                         (float *) &points[0].x,
                         (float *) &points[0].y,
                         int(points.size()),
                         ImPlotLineFlags_None,
                         0,
                         sizeof(points[0])
        );

        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
        ImPlot::PlotLine("Differential",
                         (float *) &points[0].x,
                         (float *) &points[0].dy,
                         int(points.size()),
                         ImPlotLineFlags_None,
                         0,
                         sizeof(points[0])
        );
        ImPlot::EndPlot();
    }

    ImGui::End();
}
