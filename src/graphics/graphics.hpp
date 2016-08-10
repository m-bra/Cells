#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "GLL/GLL.hpp"
#include <functional>
#include <vector>

struct ProgramVars
{
    gll::Uniform mvp;
    gll::Attribute vertXYZ;
    gll::Attribute vertUV;
    gll::Uniform tex;
};

struct Model
{
    GLuint vbo, vao;
};

struct Graphics
{
    ProgramVars program_vars;
    std::vector<Model> models;
    GLuint tex;
};

void init_graphics(Graphics *graphics);
void render(Graphics *graphics, struct Body *bodies, int body_count);

#endif
