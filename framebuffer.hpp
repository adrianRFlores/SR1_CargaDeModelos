#pragma once
#include <glm/glm.hpp>
#include <omp.h>
#include "color.hpp"

//Framebuffer
const int FRAMEBUFFER_WIDTH = 1000, FRAMEBUFFER_HEIGHT = 800;
const int FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

Color framebuffer[FRAMEBUFFER_SIZE];

void clear() {
    #pragma omp parallel for
    for (int i = 0; i < FRAMEBUFFER_SIZE; i++) {
        framebuffer[i] = clearColor;
    }
}

//Pone un punto con el color actual en x, y de un vec4
//c es el alfa del color a dibujar (0 = negro a 1 = 100%)
inline void point(const glm::vec4& vert, const float c) {
    if (vert.x+500 >= 0 && vert.x+500 < FRAMEBUFFER_WIDTH && vert.y+400 >= 0 && vert.y+400 < FRAMEBUFFER_HEIGHT) {
        framebuffer[(int)(vert.y+400) * FRAMEBUFFER_WIDTH + (int)vert.x+500] = Color(currentColor.r * c, currentColor.g * c, currentColor.b * c);
    }
}

//Se utiliza solo para el algoritmo de anti-aliasing
inline void point(const int x, const int y, const float c) {
    if (x+500 >= 0 && x+500 < FRAMEBUFFER_WIDTH && y+400 >= 0 && y+400 < FRAMEBUFFER_HEIGHT) {
        framebuffer[(y+400) * FRAMEBUFFER_WIDTH + x + 500] = Color(currentColor.r * c, currentColor.g * c, currentColor.b * c);
    }
}

//Se utiliza solo para el algoritmo de anti-aliasing
inline void point(const int x, const int y, const float c, const float gradient) {
    if (x+500 >= 0 && x+500 < FRAMEBUFFER_WIDTH && y+400 >= 0 && y+400 < FRAMEBUFFER_HEIGHT) {
        framebuffer[(y+400) * FRAMEBUFFER_WIDTH + x + 500] = Color(currentColor.r * c, currentColor.g * c - (gradient*100), currentColor.b * c);
    }
}