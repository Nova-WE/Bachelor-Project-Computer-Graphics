//
// Created by eliasw on 08.09.26.
//

#ifndef BACHELORPROJECT_TEXTURE_H
#define BACHELORPROJECT_TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#include <iostream>
#include <ostream>
#include <string>
#include "rtweekend.h"

class Texture {
public:
    Texture() = default;

    explicit Texture(const std::string & path) {
        loadFromFile(path);
    }

    bool loadFromFile(const std::string & path) {
        int w, h, c;
        unsigned char *img = stbi_load(path.c_str(), &w, &h, &c, 4);
        if (!img) return false;
        // Do something with the loaded image data
        width = w; height = h; channels = 4;
        data.assign(img, img + width * height * channels);
        stbi_image_free(img);
        return true;
    }

    [[nodiscard]] color sample(float u, float v) const {
        if (width == 0 || height == 0) return {1.0f, 1.0f, 1.0f, 1.0f}; // Return white if texture is not loaded
        u = u - floor(u);
        v = v - floor(v);

        const float xf = u * (width - 1.0);
        const float yf = (1.0f - v) * (height - 1.0);

        const int x0 = static_cast<int>(floor(xf));
        const int y0 = static_cast<int>(floor(yf));
        const int x1 = std::min(x0 + 1, width - 1);
        const int y1 = std::min(y0 + 1, height - 1);

        const float sx = xf - x0;
        const float sy = yf - y0;

        auto fetch = [&](const int x, const int y) -> color {
            const size_t idx = (static_cast<size_t>(y) * width + x) * channels;
            const float r = data[idx] / 255.0f;
            const float g = data[idx + 1] / 255.0f;
            const float b = data[idx + 2] / 255.0f;
            const float a = data[idx + 3] / 255.0f;
            return {r, g, b, a};
        };

        const color c00 = fetch(x0, y0);
        const color c10 = fetch(x1, y0);
        const color c01 = fetch(x0, y1);
        const color c11 = fetch(x1, y1);

        return lerp(lerp(c00, c10, sx), lerp(c01, c11, sx), sy);
    }

private:
    int width = 0, height = 0, channels = 0;
    std::vector<unsigned char> data;

    static color lerp(const color &c1, const color &c2, float t) {
        return (1.0f - t) * c1 + t * c2;
    }
};

#endif //BACHELORPROJECT_TEXTURE_H
