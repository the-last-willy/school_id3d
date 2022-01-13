#pragma once

#include "lighting/all.hpp"
#include "material/all.hpp"
#include "object/all.hpp"
#include "vertex_attribute/all.hpp"

#include <agl/engine/all.hpp>

#include <array>
#include <vector>

struct Scene {
    LightGroup lights;
    MaterialGroup materials;
    ObjectGroup objects;
};
