#include "GLL/GLL.hpp"
#include "graphics.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include "CImg.h"
#include "Logger.hpp"

using namespace cimg_library;

/// vertex layout: floats: x, y, z, u, v
#define VERTEX_STRIDE 5

/// set position of given vertex
void set_xyz(float *vertex, float x, float y, float z)
{
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
}

/// set texture coordinates of given vertex
void set_uv(float *vertex, float u, float v)
{
    vertex[3] = u;
    vertex[4] = v;
}

/// get the nth vertex
float *get_n(float *vertex, int n)
{
    return &vertex[n * VERTEX_STRIDE];
}

/// construct n+2 circle vertices (n+1 edges), first vertex is center, second is top, then clockwise on the edge, last vertex is top again
/// to be used in a vbo and rendered with GL_TRIANGLE_FAN
/// (n+1) * vertex_size = (n+1) * 5 elements are accessed beyond the vertices pointer
/// top has to be covered twice for correct texturing
void circle(int n, float radius, float edge_z, float center_z, float tex_left, float tex_top, float tex_width, float tex_height, float *vertices)
{
    set_xyz(get_n(vertices, 0), 0, 0, center_z);
    set_uv(get_n(vertices, 0), tex_left + tex_width / 2, tex_top + tex_height / 2);

    for (int i = 0; i < n + 1; ++i)
    {
	float angle = 2 * M_PI / n * i;
	float x = cos(angle), y = sin(angle);
	set_xyz(get_n(vertices, i), x * radius, y * radius, edge_z);
	float u = tex_left + (x + 1) / 2 * tex_width;
	float v = tex_top + (y + 1) / 2 * tex_height;
	set_uv(get_n(vertices, i), u, v);
    }
}

void init_graphics(Graphics *graphics)
{
    gll::init();
    gll::setCulling(false);

    using namespace glbinding;
    setCallbackMaskExcept(CallbackMask::After, { "glGetError" });
    setAfterCallback([](const FunctionCall &)
    {
	const auto error = glGetError();
	if (error != GL_NO_ERROR)
	    std::cout << "error: " << std::hex << error << std::endl;
    });
    
    // shader
    gll::Program program;
    program.create();
    program.addShader(GL_VERTEX_SHADER, "res/shader.vert", true);
    program.addShader(GL_FRAGMENT_SHADER, "res/shader.frag", true);
    graphics->program_vars.vertXYZ = program.getAttribute("vertXYZ");
    graphics->program_vars.vertUV = program.getAttribute("vertUV");
    program.link();
    graphics->program_vars.mvp = program.getUniformLocation("mvp");
    graphics->program_vars.tex = program.getUniformLocation("tex");
    program.bind();

    // vbo & vao
    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    constexpr int circle_resolution = 20;
    float vertices[(circle_resolution+2) * VERTEX_STRIDE];
    circle(circle_resolution, 0.7, 1, 0, 0, 0, 1, 1, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (circle_resolution + 2) * VERTEX_STRIDE * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vao);
    GLenum floatType = gll::OpenGLType<float>::type;
    glEnableVertexAttribArray(graphics->program_vars.vertXYZ);
    glEnableVertexAttribArray(graphics->program_vars.vertUV);
    int stride = VERTEX_STRIDE * sizeof(float);
    glVertexAttribPointer(graphics->program_vars.vertXYZ, 3, floatType, GL_FALSE,
			  stride, (void *)(0 * sizeof(float)));
    glVertexAttribPointer(graphics->program_vars.vertUV, 2, floatType, GL_FALSE,
			  stride, (void *)(3 * sizeof(float)));

    graphics->models.push_back({vbo, vao});

    // texture
    using ImgComponent = unsigned char;
    CImg<ImgComponent> img("res/texture.png");
    ImgComponent *pixels = new ImgComponent[img.width() * img.height() * 4];
    if (img.spectrum() == 4)
	for (int y = 0; y < img.height(); ++y)
	for (int x = 0; x < img.width(); ++x)
	{
	    pixels[y * img.width() * 4 + x * 4 + 0] = img(x, y, 0);
	    pixels[y * img.width() * 4 + x * 4 + 1] = img(x, y, 1);
	    pixels[y * img.width() * 4 + x * 4 + 2] = img(x, y, 2);
	    pixels[y * img.width() * 4 + x * 4 + 3] = img(x, y, 3);
	}
    else if (img.spectrum() == 3)
	for (int y = 0; y < img.height(); ++y)
	for (int x = 0; x < img.width(); ++x)
	{
	    pixels[y * img.width() * 4 + x * 4 + 0] = img(x, y, 0);
	    pixels[y * img.width() * 4 + x * 4 + 1] = img(x, y, 1);
	    pixels[y * img.width() * 4 + x * 4 + 2] = img(x, y, 2);
	    pixels[y * img.width() * 4 + x * 4 + 3] = 255;
	}
    else
	LOG_FATAL("Cannot load chunk.bmp: 3 or 4 channels required, have ", img.spectrum());

    // push texture to gpu
    glGenTextures(1, &graphics->tex);
    glBindTexture(GL_TEXTURE_2D, graphics->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    delete[] pixels;
}

void render(Graphics *graphics)
{
    glClearColor(.1, 1, .1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1i(graphics->program_vars.tex, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, graphics->tex);
    
    glm::mat4 ident = glm::mat4(1.0);
    glUniformMatrix4fv(graphics->program_vars.mvp, 1, false, &ident[0][0]);
    glBindVertexArray(graphics->models[0].vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 20 + 1);
}
