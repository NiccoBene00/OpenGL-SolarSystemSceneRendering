#pragma once

#include <glad/glad.h>

// load texture standard (jpg/png)
unsigned int loadTexture(const char* path, bool gammaCorrection = false);

// load texture HDR (float)
unsigned int loadHDRTexture(const char* path);