/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>
#include "GLES2/gl2.h"

// Dummy implementation for glActiveTexture
void glActiveTexture(GLenum texture) {
    printf("Dummy implementation of glActiveTexture\n");
}
// Dummy implementation for glAttachShader
void glAttachShader(GLuint program, GLuint shader) {
    printf("Dummy implementation of glAttachShader\n");
}
// Dummy implementation for glBindAttribLocation
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) {
    printf("Dummy implementation of glBindAttribLocation\n");
}
// Dummy implementation for glBindBuffer
void glBindBuffer(GLenum target, GLuint buffer) {
    printf("Dummy implementation of glBindBuffer\n");
}
// Dummy implementation for glBindFramebuffer
void glBindFramebuffer(GLenum target, GLuint framebuffer) {
    printf("Dummy implementation of glBindFramebuffer\n");
}
// Dummy implementation for glBindRenderbuffer
void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    printf("Dummy implementation of glBindRenderbuffer\n");
}
// Dummy implementation for glBindTexture
void glBindTexture(GLenum target, GLuint texture) {
    printf("Dummy implementation of glBindTexture\n");
}
// Dummy implementation for glBlendColor
void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    printf("Dummy implementation of glBlendColor\n");
}
// Dummy implementation for glBlendEquation
void glBlendEquation(GLenum mode) {
    printf("Dummy implementation of glBlendEquation\n");
}
// Dummy implementation for glBlendEquationSeparate
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
    printf("Dummy implementation of glBlendEquationSeparate\n");
}
// Dummy implementation for glBlendFunc
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    printf("Dummy implementation of glBlendFunc\n");
}
// Dummy implementation for glBlendFuncSeparate
void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    printf("Dummy implementation of glBlendFuncSeparate\n");
}
// Dummy implementation for glBufferData
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) {
    printf("Dummy implementation of glBufferData\n");
}
// Dummy implementation for glBufferSubData
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) {
    printf("Dummy implementation of glBufferSubData\n");
}
// Dummy implementation for glCheckFramebufferStatus
GLenum glCheckFramebufferStatus(GLenum target) {
    printf("Dummy implementation of glCheckFramebufferStatus\n");
    return GL_FRAMEBUFFER_COMPLETE;
}
// Dummy implementation for glClear
void glClear(GLbitfield mask) {
    printf("Dummy implementation of glClear\n");
}
// Dummy implementation for glClearColor
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    printf("Dummy implementation of glClearColor\n");
}
// Dummy implementation for glClearDepthf
void glClearDepthf(GLfloat depth) {
    printf("Dummy implementation of glClearDepthf\n");
}
// Dummy implementation for glClearStencil
void glClearStencil(GLint s) {
    printf("Dummy implementation of glClearStencil\n");
}
// Dummy implementation for glColorMask
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    printf("Dummy implementation of glColorMask\n");
}
// Dummy implementation for glCompileShader
void glCompileShader(GLuint shader) {
    printf("Dummy implementation of glCompileShader\n");
}
// Dummy implementation for glCompressedTexImage2D
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {
    printf("Dummy implementation of glCompressedTexImage2D\n");
}
// Dummy implementation for glCompressedTexSubImage2D
void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {
    printf("Dummy implementation of glCompressedTexSubImage2D\n");
}
// Dummy implementation for glCopyTexImage2D
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    printf("Dummy implementation of glCopyTexImage2D\n");
}
// Dummy implementation for glCopyTexSubImage2D
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    printf("Dummy implementation of glCopyTexSubImage2D\n");
}
// Dummy implementation for glCreateProgram
GLuint glCreateProgram(void) {
    printf("Dummy implementation of glCreateProgram\n");
    return 1; // Return a dummy program ID
}
// Dummy implementation for glCreateShader
GLuint glCreateShader(GLenum type) {
    printf("Dummy implementation of glCreateShader\n");
    return 1; // Return a dummy shader ID
}
// Dummy implementation for glCullFace
void glCullFace(GLenum mode) {
    printf("Dummy implementation of glCullFace\n");
}
// Dummy implementation for glDeleteBuffers
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
    printf("Dummy implementation of glDeleteBuffers\n");
}
// Dummy implementation for glDeleteFramebuffers
void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
    printf("Dummy implementation of glDeleteFramebuffers\n");
}
// Dummy implementation for glDeleteProgram
void glDeleteProgram(GLuint program) {
    printf("Dummy implementation of glDeleteProgram\n");
}
// Dummy implementation for glDeleteRenderbuffers
void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
    printf("Dummy implementation of glDeleteRenderbuffers\n");
}
// Dummy implementation for glDeleteShader
void glDeleteShader(GLuint shader) {
    printf("Dummy implementation of glDeleteShader\n");
}
// Dummy implementation for glDeleteTextures
void glDeleteTextures(GLsizei n, const GLuint *textures) {
    printf("Dummy implementation of glDeleteTextures\n");
}
// Dummy implementation for glDepthFunc
void glDepthFunc(GLenum func) {
    printf("Dummy implementation of glDepthFunc\n");
}
// Dummy implementation for glDepthMask
void glDepthMask(GLboolean flag) {
    printf("Dummy implementation of glDepthMask\n");
}
// Dummy implementation for glDepthRangef
void glDepthRangef(GLfloat n, GLfloat f) {
    printf("Dummy implementation of glDepthRangef\n");
}
// Dummy implementation for glDetachShader
void glDetachShader(GLuint program, GLuint shader) {
    printf("Dummy implementation of glDetachShader\n");
}
// Dummy implementation for glDisable
void glDisable(GLenum cap) {
    printf("Dummy implementation of glDisable\n");
}
// Dummy implementation for glDisableVertexAttribArray
void glDisableVertexAttribArray(GLuint index) {
    printf("Dummy implementation of glDisableVertexAttribArray\n");
}
// Dummy implementation for glDrawArrays
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    printf("Dummy implementation of glDrawArrays\n");
}
// Dummy implementation for glDrawElements
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    printf("Dummy implementation of glDrawElements\n");
}
// Dummy implementation for glEnable
void glEnable(GLenum cap) {
    printf("Dummy implementation of glEnable\n");
}
// Dummy implementation for glEnableVertexAttribArray
void glEnableVertexAttribArray(GLuint index) {
    printf("Dummy implementation of glEnableVertexAttribArray\n");
}
// Dummy implementation for glFinish
void glFinish(void) {
    printf("Dummy implementation of glFinish\n");
}
// Dummy implementation for glFlush
void glFlush(void) {
    printf("Dummy implementation of glFlush\n");
}
// Dummy implementation for glFramebufferRenderbuffer
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    printf("Dummy implementation of glFramebufferRenderbuffer\n");
}
// Dummy implementation for glFramebufferTexture2D
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    printf("Dummy implementation of glFramebufferTexture2D\n");
}
// Dummy implementation for glFrontFace
void glFrontFace(GLenum mode) {
    printf("Dummy implementation of glFrontFace\n");
}
// Dummy implementation for glGenBuffers
void glGenBuffers(GLsizei n, GLuint *buffers) {
    printf("Dummy implementation of glGenBuffers\n");
}
// Dummy implementation for glGenerateMipmap
void glGenerateMipmap(GLenum target) {
    printf("Dummy implementation of glGenerateMipmap\n");
}
// Dummy implementation for glGenFramebuffers
void glGenFramebuffers(GLsizei n, GLuint *framebuffers) {
    printf("Dummy implementation of glGenFramebuffers\n");
}
// Dummy implementation for glGenRenderbuffers
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
    printf("Dummy implementation of glGenRenderbuffers\n");
}
// Dummy implementation for glGenTextures
void glGenTextures(GLsizei n, GLuint *textures) {
    printf("Dummy implementation of glGenTextures\n");
}
// Dummy implementation for glGetActiveAttrib
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
    printf("Dummy implementation of glGetActiveAttrib\n");
}
// Dummy implementation for glGetActiveUniform
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
    printf("Dummy implementation of glGetActiveUniform\n");
}
// Dummy implementation for glGetAttachedShaders
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
    printf("Dummy implementation of glGetAttachedShaders\n");
}
// Dummy implementation for glGetAttribLocation
GLint glGetAttribLocation(GLuint program, const GLchar *name) {
    printf("Dummy implementation of glGetAttribLocation\n");
    return 0;
}
// Dummy implementation for glGetBooleanv
void glGetBooleanv(GLenum pname, GLboolean *params) {
    printf("Dummy implementation of glGetBooleanv\n");
}
// Dummy implementation for glGetBufferParameteriv
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetBufferParameteriv\n");
}
// Dummy implementation for glGetError
GLenum glGetError(void) {
    printf("Dummy implementation of glGetError\n");
    return GL_NO_ERROR;
}
// Dummy implementation for glGetFloatv
void glGetFloatv(GLenum pname, GLfloat *params) {
    printf("Dummy implementation of glGetFloatv\n");
}
// Dummy implementation for glGetFramebufferAttachmentParameteriv
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetFramebufferAttachmentParameteriv\n");
}
// Dummy implementation for glGetIntegerv
void glGetIntegerv(GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetIntegerv\n");
}
// Dummy implementation for glGetProgramiv
void glGetProgramiv(GLuint program, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetProgramiv\n");
}
// Dummy implementation for glGetProgramInfoLog
void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei *length, GLchar *infolog) {
    printf("Dummy implementation of glGetProgramInfoLog\n");
}
// Dummy implementation for glGetRenderbufferParameteriv
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetRenderbufferParameteriv\n");
}
// Dummy implementation for glGetShaderiv
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetShaderiv\n");
}
// Dummy implementation for glGetShaderInfoLog
void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *infolog) {
    printf("Dummy implementation of glGetShaderInfoLog\n");
}
// Dummy implementation for glGetShaderPrecisionFormat
void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {
    printf("Dummy implementation of glGetShaderPrecisionFormat\n");
}
// Dummy implementation for glGetShaderSource
void glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source) {
    printf("Dummy implementation of glGetShaderSource\n");
}
// Dummy implementation for glGetString
const GLubyte* glGetString(GLenum name) {
    printf("Dummy implementation of glGetString\n");
    return (const GLubyte*)"Dummy String";
}
// Dummy implementation for glGetTexParameterfv
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) {
    printf("Dummy implementation of glGetTexParameterfv\n");
}
// Dummy implementation for glGetTexParameteriv
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetTexParameteriv\n");
}
// Dummy implementation for glGetUniformfv
void glGetUniformfv(GLuint program, GLint location, GLfloat *params) {
    printf("Dummy implementation of glGetUniformfv\n");
}
// Dummy implementation for glGetUniformiv
void glGetUniformiv(GLuint program, GLint location, GLint *params) {
    printf("Dummy implementation of glGetUniformiv\n");
}
// Dummy implementation for glGetUniformLocation
GLint glGetUniformLocation(GLuint program, const GLchar *name) {
    printf("Dummy implementation of glGetUniformLocation\n");
    return 0;
}
// Dummy implementation for glGetVertexAttribfv
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params) {
    printf("Dummy implementation of glGetVertexAttribfv\n");
}
// Dummy implementation for glGetVertexAttribiv
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params) {
    printf("Dummy implementation of glGetVertexAttribiv\n");
}
// Dummy implementation for glGetVertexAttribPointerv
void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer) {
    printf("Dummy implementation of glGetVertexAttribPointerv\n");
}
// Dummy implementation for glHint
void glHint(GLenum target, GLenum mode) {
    printf("Dummy implementation of glHint\n");
}
// Dummy implementation for glIsBuffer
GLboolean glIsBuffer(GLuint buffer) {
    printf("Dummy implementation of glIsBuffer\n");
    return GL_FALSE;
}
// Dummy implementation for glIsEnabled
GLboolean glIsEnabled(GLenum cap) {
    printf("Dummy implementation of glIsEnabled\n");
    return GL_FALSE;
}
// Dummy implementation for glIsFramebuffer
GLboolean glIsFramebuffer(GLuint framebuffer) {
    printf("Dummy implementation of glIsFramebuffer\n");
    return GL_FALSE;
}
// Dummy implementation for glIsProgram
GLboolean glIsProgram(GLuint program) {
    printf("Dummy implementation of glIsProgram\n");
    return GL_FALSE;
}
// Dummy implementation for glIsRenderbuffer
GLboolean glIsRenderbuffer(GLuint renderbuffer) {
    printf("Dummy implementation of glIsRenderbuffer\n");
    return GL_FALSE;
}
// Dummy implementation for glIsShader
GLboolean glIsShader(GLuint shader) {
    printf("Dummy implementation of glIsShader\n");
    return GL_FALSE;
}
// Dummy implementation for glIsTexture
GLboolean glIsTexture(GLuint texture) {
    printf("Dummy implementation of glIsTexture\n");
    return GL_FALSE;
}
// Dummy implementation for glLineWidth
void glLineWidth(GLfloat width) {
    printf("Dummy implementation of glLineWidth\n");
}
// Dummy implementation for glLinkProgram
void glLinkProgram(GLuint program) {
    printf("Dummy implementation of glLinkProgram\n");
}
// Dummy implementation for glPixelStorei
void glPixelStorei(GLenum pname, GLint param) {
    printf("Dummy implementation of glPixelStorei\n");
}
// Dummy implementation for glPolygonOffset
void glPolygonOffset(GLfloat factor, GLfloat units) {
    printf("Dummy implementation of glPolygonOffset\n");
}
// Dummy implementation for glReadPixels
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) {
    printf("Dummy implementation of glReadPixels\n");
}
// Dummy implementation for glReleaseShaderCompiler
void glReleaseShaderCompiler(void) {
    printf("Dummy implementation of glReleaseShaderCompiler\n");
}
// Dummy implementation for glRenderbufferStorage
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    printf("Dummy implementation of glRenderbufferStorage\n");
}
// Dummy implementation for glSampleCoverage
void glSampleCoverage(GLfloat value, GLboolean invert) {
    printf("Dummy implementation of glSampleCoverage\n");
}
// Dummy implementation for glScissor
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    printf("Dummy implementation of glScissor\n");
}
// Dummy implementation for glShaderBinary
void glShaderBinary(GLsizei n, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length) {
    printf("Dummy implementation of glShaderBinary\n");
}
// Dummy implementation for glShaderSource
void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length){
    printf("Dummy implementation of glShaderSource\n");
}
// Dummy implementation for glStencilFunc
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    printf("Dummy implementation of glStencilFunc\n");
}
// Dummy implementation for glStencilFuncSeparate
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    printf("Dummy implementation of glStencilFuncSeparate\n");
}
// Dummy implementation for glStencilMask
void glStencilMask(GLuint mask) {
    printf("Dummy implementation of glStencilMask\n");
}
// Dummy implementation for glStencilMaskSeparate
void glStencilMaskSeparate(GLenum face, GLuint mask) {
    printf("Dummy implementation of glStencilMaskSeparate\n");
}
// Dummy implementation for glStencilOp
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    printf("Dummy implementation of glStencilOp\n");
}
// Dummy implementation for glStencilOpSeparate
void glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
    printf("Dummy implementation of glStencilOpSeparate\n");
}
// Dummy implementation for glTexImage2D
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    printf("Dummy implementation of glTexImage2D\n");
}
// Dummy implementation for glTexParameterf
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
    printf("Dummy implementation of glTexParameterf\n");
}
// Dummy implementation for glTexParameterfv
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) {
    printf("Dummy implementation of glTexParameterfv\n");
}
// Dummy implementation for glTexParameteri
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    printf("Dummy implementation of glTexParameteri\n");
}
// Dummy implementation for glTexParameteriv
void glTexParameteriv(GLenum target, GLenum pname, const GLint *params) {
    printf("Dummy implementation of glTexParameteriv\n");
}
// Dummy implementation for glTexSubImage2D
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    printf("Dummy implementation of glTexSubImage2D\n");
}
// Dummy implementation for glUniform1f
void glUniform1f(GLint location, GLfloat v0) {
    printf("Dummy implementation of glUniform1f\n");
}
// Dummy implementation for glUniform1fv
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {
    printf("Dummy implementation of glUniform1fv\n");
}
// Dummy implementation for glUniform1i
void glUniform1i(GLint location, GLint v0) {
    printf("Dummy implementation of glUniform1i\n");
}
// Dummy implementation for glUniform1iv
void glUniform1iv(GLint location, GLsizei count, const GLint *value) {
    printf("Dummy implementation of glUniform1iv\n");
}
// Dummy implementation for glUniform2f
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
    printf("Dummy implementation of glUniform2f\n");
}
// Dummy implementation for glUniform2fv
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {
    printf("Dummy implementation of glUniform2fv\n");
}
// Dummy implementation for glUniform2i
void glUniform2i(GLint location, GLint v0, GLint v1) {
    printf("Dummy implementation of glUniform2i\n");
}
// Dummy implementation for glUniform2iv
void glUniform2iv(GLint location, GLsizei count, const GLint *value) {
    printf("Dummy implementation of glUniform2iv\n");
}
// Dummy implementation for glUniform3f
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    printf("Dummy implementation of glUniform3f\n");
}
// Dummy implementation for glUniform3fv
void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {
    printf("Dummy implementation of glUniform3fv\n");
}
// Dummy implementation for glUniform3i
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
    printf("Dummy implementation of glUniform3i\n");
}
// Dummy implementation for glUniform3iv
void glUniform3iv(GLint location, GLsizei count, const GLint *value) {
    printf("Dummy implementation of glUniform3iv\n");
}
// Dummy implementation for glUniform4f
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    printf("Dummy implementation of glUniform4f\n");
}
// Dummy implementation for glUniform4fv
void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {
    printf("Dummy implementation of glUniform4fv\n");
}
// Dummy implementation for glUniform4i
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    printf("Dummy implementation of glUniform4i\n");
}
// Dummy implementation for glUniform4iv
void glUniform4iv(GLint location, GLsizei count, const GLint *value) {
    printf("Dummy implementation of glUniform4iv\n");
}
// Dummy implementation for glUniformMatrix2fv
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    printf("Dummy implementation of glUniformMatrix2fv\n");
}
// Dummy implementation for glUniformMatrix3fv
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    printf("Dummy implementation of glUniformMatrix3fv\n");
}
// Dummy implementation for glUniformMatrix4fv
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    printf("Dummy implementation of glUniformMatrix4fv\n");
}
// Dummy implementation for glUseProgram
void glUseProgram(GLuint program) {
    printf("Dummy implementation of glUseProgram\n");
}
// Dummy implementation for glValidateProgram
void glValidateProgram(GLuint program) {
    printf("Dummy implementation of glValidateProgram\n");
}
// Dummy implementation for glVertexAttrib1f
void glVertexAttrib1f(GLuint index, GLfloat x) {
    printf("Dummy implementation of glVertexAttrib1f\n");
}
// Dummy implementation for glVertexAttrib1fv
void glVertexAttrib1fv(GLuint index, const GLfloat *v) {
    printf("Dummy implementation of glVertexAttrib1fv\n");
}
// Dummy implementation for glVertexAttrib2f
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
    printf("Dummy implementation of glVertexAttrib2f\n");
}
// Dummy implementation for glVertexAttrib2fv
void glVertexAttrib2fv(GLuint index, const GLfloat *v) {
    printf("Dummy implementation of glVertexAttrib2fv\n");
}
// Dummy implementation for glVertexAttrib3f
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    printf("Dummy implementation of glVertexAttrib3f\n");
}
// Dummy implementation for glVertexAttrib3fv
void glVertexAttrib3fv(GLuint index, const GLfloat *v) {
    printf("Dummy implementation of glVertexAttrib3fv\n");
}
// Dummy implementation for glVertexAttrib4f
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    printf("Dummy implementation of glVertexAttrib4f\n");
}
// Dummy implementation for glVertexAttrib4fv
void glVertexAttrib4fv(GLuint index, const GLfloat *v) {
    printf("Dummy implementation of glVertexAttrib4fv\n");
}
// Dummy implementation for glVertexAttribPointer
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {
    printf("Dummy implementation of glVertexAttribPointer\n");
}
// Dummy implementation for glViewport
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    printf("Dummy implementation of glViewport\n");
}
