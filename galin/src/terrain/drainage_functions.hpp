#pragma once

#include "terrain/all.hpp"

void steepest_compute(Terrain &t) {
    auto nx = resolution(t)[0];
    auto ny = resolution(t)[1];
    std::vector<agl::Vec2> neighbours = {
        agl::vec2(-1, 1), agl::vec2(0, 1), agl::vec2(1, 1), agl::vec2(-1, 0), 
        agl::vec2(1, 0), agl::vec2(-1, -1), agl::vec2(0, -1), agl::vec2(1, -1)};

    auto positions = std::vector<Drainage_data>((nx - 2) * (ny - 2));
    for(size_t i = 1; i < nx - 1; ++i)
    for(size_t j = 1; j < ny - 1; ++j) {
        positions[(ny - 2) * (i - 1) + (j - 1)] = {std::array<size_t, 2>{i, j}, at(t.heights, i, j)};
    }
    std::sort(positions.begin(), positions.end(), compare_height);

    for(std::size_t i = 0; i < positions.size(); ++i) {
        auto x = positions[i].position[0];
        auto y = positions[i].position[1];
        // for(std::size_t c = x; c < x + 1; c+= 2) {
        //     for(std::size_t r = y + 1; r > y - 1; r -=2) {
        //         if(r != x && c != y) {
                    
        //         }
        //     }
        // }

        auto up = at(t.heights, x, y + 1);
        auto down = at(t.heights, x, y - 1);
        auto left = at(t.heights, x - 1, y);
        auto right = at(t.heights, x + 1, y);
        auto current_height = positions[i].height;
        auto max = current_height - up;
        auto x_max = x;
        auto y_max = y + 1;

        if(max < current_height - down) {
            max = current_height - down;
            x_max = x;
            y_max = y - 1;
        }
        if (max < current_height - left) {
            max = current_height - left;
            x_max = x - 1;
            y_max = y;
        }
        if (max < current_height - right) {
            max = current_height - right;
            x_max = x + 1;
            y_max = y;
        }
        at(t.drainage_areas, x_max, y_max) += at(t.drainage_areas, x, y);
    }
}