// CrosshairRenderer.h
#pragma once

#include "Shader.h"

class CrosshairRenderer {
public:
    CrosshairRenderer();
    ~CrosshairRenderer();

    void Draw(Shader& shader);

private:
    unsigned int VAO = 0, VBO = 0;
};