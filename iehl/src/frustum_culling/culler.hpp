#pragma once

#include "shader/all.hpp"

#include <agl/engine/all.hpp>

#include <array>

struct FrustumCuller {
    eng::Program compute_shader;

    // Transformations.

    agl::Mat4 clip_to_world;
    agl::Mat4 world_to_clip;

    // CPU > Frustum.

    agl::common::Interval<agl::Vec3> frustum_clip_bounds;
    agl::common::Interval<agl::Vec3> frustum_world_bounds;

    // CPU > Objects.

    std::vector<agl::common::Interval<agl::Vec3>> objects_world_bounds;

    // Objects GPU.

    agl::Buffer object_ssbo;

    // Inputs CPU.

    std::vector<DrawElementsParameters> input_draw_parameters;

    // Inputs GPU.

    agl::Buffer input_draw_parameters_ssbo;
    
    // Outputs CPU.

    GLuint output_count;
    std::vector<DrawElementsParameters> output_draw_parameters;

    // Outputs GPU.
    
    agl::Buffer output_count_ssbo;
    agl::Buffer output_draw_parameters_ssbo;
};

inline
auto frustum_culler(agl::common::Interval<agl::Vec3> frustum) {
    auto fc = FrustumCuller();

    return fc;
}

inline
bool aabb_intersecting(
    agl::common::Interval<agl::Vec3> i0,
    agl::Mat4 m0to1,
    agl::common::Interval<agl::Vec3> i1,
    agl::Mat4 m1to0)
{
    auto lb0 = lower_bound(i0);
    auto ub0 = upper_bound(i0);
    auto corners0 = std::array{
        agl::vec3(lb0[0], lb0[1], lb0[2]),
        agl::vec3(lb0[0], lb0[1], ub0[2]),
        agl::vec3(lb0[0], ub0[1], lb0[2]),
        agl::vec3(lb0[0], ub0[1], ub0[2]),
        agl::vec3(ub0[0], lb0[1], lb0[2]),
        agl::vec3(ub0[0], lb0[1], ub0[2]),
        agl::vec3(ub0[0], ub0[1], lb0[2]),
        agl::vec3(ub0[0], ub0[1], ub0[2]),
    };

    auto i0_in_1 = agl::common::Interval<agl::Vec3>();
    {
        {
            auto c0_in_1 = m0to1 * vec4(corners0[0], 1.f);
            i0_in_1 = agl::common::Interval<agl::Vec3>(
                c0_in_1.xyz() / c0_in_1[3]);
        }
        for(std::size_t i = 1; i < size(corners0); ++i) {
            auto c0_in_1 = m0to1 * vec4(corners0[i], 1.f);
            extend(i0_in_1, c0_in_1.xyz() / c0_in_1[3]);
        }
    }

    auto lb1 = lower_bound(i1);
    auto ub1 = upper_bound(i1);
    auto corners1 = std::array{
        agl::vec3(lb1[0], lb1[1], lb1[2]),
        agl::vec3(lb1[0], lb1[1], ub1[2]),
        agl::vec3(lb1[0], ub1[1], lb1[2]),
        agl::vec3(lb1[0], ub1[1], ub1[2]),
        agl::vec3(ub1[0], lb1[1], lb1[2]),
        agl::vec3(ub1[0], lb1[1], ub1[2]),
        agl::vec3(ub1[0], ub1[1], lb1[2]),
        agl::vec3(ub1[0], ub1[1], ub1[2]),
    };

    auto i1_in_0 = agl::common::Interval<agl::Vec3>();
    {
        {
            auto c1_in_0 = m1to0 * vec4(corners1[0], 1.f);
            i1_in_0 = agl::common::Interval<agl::Vec3>(
                c1_in_0.xyz() / c1_in_0[3]);
        }
        for(std::size_t i = 1; i < size(corners1); ++i) {
            auto c1_in_0 = m1to0 * vec4(corners1[i], 1.f);
            extend(i1_in_0, c1_in_0.xyz() / c1_in_0[3]);
        }
    }

    return not(false
    or lower_bound(i0_in_1)[0] > upper_bound(i1)[0]
    or lower_bound(i0_in_1)[1] > upper_bound(i1)[1]
    or lower_bound(i0_in_1)[2] > upper_bound(i1)[2]

    or upper_bound(i0_in_1)[0] < lower_bound(i1)[0]
    or upper_bound(i0_in_1)[1] < lower_bound(i1)[1]
    or upper_bound(i0_in_1)[2] < lower_bound(i1)[2]

    or lower_bound(i1_in_0)[0] > upper_bound(i0)[0]
    or lower_bound(i1_in_0)[1] > upper_bound(i0)[1]
    or lower_bound(i1_in_0)[2] > upper_bound(i0)[2]

    or upper_bound(i1_in_0)[0] < lower_bound(i0)[0]
    or upper_bound(i1_in_0)[1] < lower_bound(i0)[1]
    or upper_bound(i1_in_0)[2] < lower_bound(i0)[2]);
}