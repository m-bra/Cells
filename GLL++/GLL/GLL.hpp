/*
 * OpenGLWrapper.hpp
 *
 *  Created on: Aug 30, 2014
 *      Author: merlin
 */

#ifndef OPENGLWRAPPER_HPP_
#define OPENGLWRAPPER_HPP_

#include <cstdlib>
#include <iostream>
#include <vector>
#include "glbinding/gl/gl.h"
#include "glbinding/Binding.h"

using namespace gl;

namespace gll
{

using Attribute = GLuint;
using Uniform = GLint;

inline void init()
{
    glbinding::Binding::initialize();
}

inline void setDepthTest(bool enabled, GLenum func = GL_LEQUAL)
{
	(enabled ?glEnable :glDisable)(GL_DEPTH_TEST);
	glDepthFunc(func);
}

inline void setCulling(bool enabled, GLenum frontFace = GL_CCW)
{
	(enabled ?glEnable :glDisable)(GL_CULL_FACE);
	glFrontFace(frontFace);
	glCullFace(GL_BACK);
}

template <typename T>
struct OpenGLType
{
};

template <>
struct OpenGLType<float>
{
	static constexpr GLenum type = GL_FLOAT;
};

template <>
struct OpenGLType<double>
{
	static constexpr GLenum type = GL_DOUBLE;
};

template <>
struct OpenGLType<unsigned char>
{
	static constexpr GLenum type = GL_UNSIGNED_BYTE;
};

template <>
struct OpenGLType<char>
{
	static constexpr GLenum type = GL_BYTE;
};

}  // namespace gll

#include "Program.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"

#endif /* OPENGLWRAPPER_HPP_ */
