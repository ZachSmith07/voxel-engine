#pragma once

// cube
inline const float CubeFaces[6][18] = {
    // -Z (Back)
    { 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f },

    // +Z (Front)
    { 0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f },

    // -X (Left)
    { 0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f },

    // +X (Right)
    { 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f },

    // -Y (Bottom)
    { 0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f },

    // +Y (Top)
    { 0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f },
};

inline const float* CubeFaceVertices(int face) {
    return CubeFaces[face];
}
