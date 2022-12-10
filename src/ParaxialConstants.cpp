#include <lui/ParaxialConstants.hpp>

#include <lore/analysis/Paraxial.h>
#include <lore/analysis/Petzval.h>

#include <imgui.h>
#include <implot.h>

using namespace lore;

void ParaxialConstants::compute(const LensSchema<float> &lensSchema) {
    const auto lens = lensSchema.lens<double>();

    const auto transfer = abcd::full(lens, double(lensSchema.primaryWavelength()));
    const double objectDistance = lens.surfaces.front().thickness;
    const double objectHeight = lens.surfaces.front().aperture;
    const double inSlope = lensSchema.entranceBeamRadius / objectDistance;
    const double outSlope = lensSchema.entranceBeamRadius * transfer(1, 0) + inSlope * transfer(1, 1);

    petzvalRadius = 1 / petzvalCurvature(lens, double(lensSchema.primaryWavelength()));
    effectiveFocalLength = -1 / transfer(1, 0);
    gaussianImageHeight = -inSlope / outSlope * objectHeight;
    lagrangeInvariant = -inSlope * objectHeight;
    lateralMagnification = inSlope / outSlope;
    numericalAperture = abs(outSlope) / sqrt(1 + sqr(inSlope));
    workingFNumber = 1 / (2 * numericalAperture);
}

void ParaxialConstants::draw() {
    if (!ImGui::Begin("Paraxial Constants")) {
        ImGui::End();
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::Text(
        "Effective focal length: %12.6lg\n"
        "Numerical aperture:     %12.6lg\n"
        "Working F-number:       %12.6lg\n"
        "Lagrange invariant:     %12.6lg\n"
        "Lateral magnification:  %12.6lg\n"
        "Gaussian image height:  %12.6lg\n"
        "Petzval radius:         %12.6lg",
        effectiveFocalLength,
        numericalAperture,
        workingFNumber,
        lagrangeInvariant,
        lateralMagnification,
        gaussianImageHeight,
        petzvalRadius
    );
    ImGui::PopFont();

    ImGui::End();
}
