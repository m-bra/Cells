#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "GLL/GLL.hpp"

struct ProgramVars
{
    gll::Uniform mvp;
    gll::Attribute vertXYZ;
    gll::Attribute vertUV;
    gll::Uniform tex;
};

struct Graphics
{
    ProgramVars program_vars;
    GLuint vbo, vao;
};

void init_graphics(Graphics *graphics);
void render(Graphics *graphics);

#endif
