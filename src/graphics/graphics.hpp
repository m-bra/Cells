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
    gll::Uniform tex_off_x;
};

struct Model
{
    GLuint vbo, vao;
};

struct Graphics
{
    ProgramVars program_vars;
    Model cell_model, attachment_model;
    GLuint cell_tex, attachment_tex;
};

void init_graphics(Graphics *graphics);
void render(Graphics *graphics, struct PhysicsWorld *world);

#endif
