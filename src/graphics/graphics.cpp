#include "GLL/GLL.hpp"
#include "graphics.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CImg.h"
#include "Logger.hpp"
#include "physics/physics.hpp"

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

unsigned char *load_img(const char *filename, int *width, int *height)
{
    CImg<unsigned char> img(filename);
    *width = img.width();
    *height = img.height();
    unsigned char *pixels = new unsigned char[img.width() * img.height() * 4];
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
	LOG_FATAL("Cannot load ", filename, " 3 or 4 channels required, have ", img.spectrum());
    return pixels;
}

void upload_texture(GLuint *texture, unsigned char *pixels, int width, int height)
{
    // push texture to gpu
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
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
    graphics->program_vars.tex_off_x = program.getUniformLocation("tex_off_x");
    program.bind();

    // cell vbo & vao
    {
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
	
	graphics->cell_model.vbo = vbo;
	graphics->cell_model.vao = vao;	
    }

    // attachment vbo & vao
    {
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	
     	float vertices[6 * VERTEX_STRIDE] = {
	    -0.2, 0, 0,  0, 0,
	    +0.2, 0, 0,  0, 1,
	    +0.2, 1, 0,  1, 1,

	    -0.2, 0, 0,  0, 0,
	    +0.2, 1, 0,  1, 1,
	    -0.2, 1, 0,  1, 0,
	};
       	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glBindVertexArray(vao);
	GLenum floatType = gll::OpenGLType<float>::type;
	glEnableVertexAttribArray(graphics->program_vars.vertXYZ);
	glEnableVertexAttribArray(graphics->program_vars.vertUV);
	int stride = VERTEX_STRIDE * sizeof(float);
	glVertexAttribPointer(graphics->program_vars.vertXYZ, 3, floatType, GL_FALSE,
			      stride, (void *)(0 * sizeof(float)));
	glVertexAttribPointer(graphics->program_vars.vertUV, 2, floatType, GL_FALSE,
			      stride, (void *)(3 * sizeof(float)));
	
	graphics->attachment_model.vbo = vbo;
	graphics->attachment_model.vao = vao;		
    }

    // cell texture
    int width, height;
    unsigned char *pixels = load_img("res/cell.png", &width, &height);
    upload_texture(&graphics->cell_tex, pixels, width, height);
    delete[] pixels;

    // attachment texture
    pixels = load_img("res/attachment.png", &width, &height);
    upload_texture(&graphics->attachment_tex, pixels, width, height);
    delete [] pixels;
}

void render(Graphics *graphics, PhysicsWorld *world)
{
    glClearColor(.1, 1, .1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1i(graphics->program_vars.tex, 0);
    glActiveTexture(GL_TEXTURE0 + 0);

    glm::mat4 view = glm::scale(glm::mat4(), glm::vec3(1 / 10.f, 1 / 10.f, 1 / 10.f));
    view = glm::translate(view, glm::vec3(-5, -5, 0));

    glBindTexture(GL_TEXTURE_2D, graphics->attachment_tex);
    world->attachments.iter().do_each([&](Attachment *attachment)
    {
	Body *body0 = attachment->bodies[0];
	Body *body1 = attachment->bodies[1];
	
	glm::vec2 sub = body1->pos - body0->pos;
	glm::vec3 sub3(sub.x, sub.y, 0);
	glm::vec4 sub4(sub.x, sub.y, 0, 0);
	glm::vec3 orthsub = glm::normalize(glm::cross(sub3, glm::vec3(0, 0, 1)));
	glm::vec4 orthsub4(orthsub.x, orthsub.y, orthsub.z, 0);
	glm::vec4 pos4(body0->pos.x, body0->pos.y, 0, 1);

	glm::mat4 model;
	// stretch & rotate
	model[0] = orthsub4;
	model[1] = sub4;
	// translate
	model[3] = pos4;
	
	glm::mat4 mvp = view * model;
	glUniformMatrix4fv(graphics->program_vars.mvp, 1, false, &mvp[0][0]);
	glBindVertexArray(graphics->attachment_model.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    });
    
    glBindTexture(GL_TEXTURE_2D, graphics->cell_tex);

    world->bodies.iter().do_each([&](Body *body)
    {
	float r = body->radius();
	glm::mat4 model = glm::scale(glm::mat4(), glm::vec3(r, r, r));
	model = glm::translate(model, glm::vec3(body->pos.x, body->pos.y, 0.f));
	model = glm::rotate(model, body->angle, glm::vec3(0, 0, 1));
       	glm::mat4 mvp = view * model;
	glUniformMatrix4fv(graphics->program_vars.mvp, 1, false, &mvp[0][0]);
	glBindVertexArray(graphics->cell_model.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 20 + 1);
    });
}
