#pragma once

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "../graphics/Shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H

enum class TextAlign
{
    Start,   // Left-aligned
    Center,  // Centered
    End      // Right-aligned
};

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

class TextRenderer {
public:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    Shader TextShader;

    TextRenderer(unsigned int width, unsigned int height, const Shader& shader);
    void Load(const std::string& font, unsigned int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color, TextAlign align = TextAlign::Start);
};

