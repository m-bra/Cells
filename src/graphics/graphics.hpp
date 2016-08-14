#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "GLL/GLL.hpp"
#include <functional>
#include <vector>
#include <glm/glm.hpp>

struct ProgramVars
{
    gll::Uniform mvp;
    gll::Attribute vertXYZ;
    gll::Attribute vertUV;
    gll::Uniform tex;
    gll::Uniform tex_off_y;
    gll::Uniform overlay_color;
};

struct Model
{
    GLuint vbo, vao;
};

struct Graphics
{
    static constexpr GLuint cell_texture_unit = 0;
    static constexpr GLuint attachment_texture_unit = 1;
    static constexpr GLuint background_texture_unit = 2;
    ProgramVars program_vars;
    Model cell_model, attachment_model, background_model;
    GLuint cell_tex, attachment_tex, background_tex;
    int cell_tex_rows;
};

void init_graphics(Graphics *graphics, struct PhysicsWorld *physics);
void render(Graphics *graphics, struct PhysicsWorld *physics, struct LogicWorld *logic,
	    glm::mat4 const &view);

#endif
