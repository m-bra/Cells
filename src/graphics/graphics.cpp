#include "GLL/GLL.hpp"
#include "graphics.hpp"
#include <glm/glm.hpp>

void init_graphics(Graphics *graphics)
{
    gll::init();

    using namespace glbinding;
    setCallbackMaskExcept(CallbackMask::After, { "glGetError" });
    setAfterCallback([](const FunctionCall &)
    {
	const auto error = glGetError();
	if (error != GL_NO_ERROR)
	    std::cout << "error: " << std::hex << error << std::endl;
    });
    
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

    glGenBuffers(1, &graphics->vbo);
    glGenVertexArrays(1, &graphics->vao);

    float vertices[3 * 5] = {
	0, 0, 0,   0, 0,
	1, 0, 0,   1, 0,
	1, 1, 0,   1, 1,
    };
    glBindBuffer(GL_ARRAY_BUFFER, graphics->vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 5 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindVertexArray(graphics->vao);
    GLenum floatType = gll::OpenGLType<float>::type;
    glEnableVertexAttribArray(graphics->program_vars.vertXYZ);
    glEnableVertexAttribArray(graphics->program_vars.vertUV);
    int stride = 3 * 5 * sizeof(float);
    glVertexAttribPointer(graphics->program_vars.vertXYZ, 3, floatType, GL_FALSE,
			  stride, (void *)(0 * sizeof(float)));
    glVertexAttribPointer(graphics->program_vars.vertUV, 2, floatType, GL_FALSE,
			  stride, (void *)(3 * sizeof(float)));
}

void render(Graphics *graphics)
{
    glClearColor(.1, 1, .1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 ident = glm::mat4(1.0);
    glUniformMatrix4fv(graphics->program_vars.mvp, 1, false, &ident[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
