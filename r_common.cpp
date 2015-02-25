// File automatically generated by ./tools/glgen.py
#include <stdarg.h>
#define R_COMMON_NO_DEFINES
#include "r_common.h"

#include "u_string.h"
#include "u_set.h"
#include "u_misc.h"

#include "engine.h"

#ifndef APIENTRY
#   ifdef _WIN32
#       define APIENTRY __stdcall
#   else
#       define APIENTRY
#   endif
#endif

#ifndef APIENTRYP
#   define APIENTRYP APIENTRY *
#endif

#ifdef DEBUG_GL
#   define GL_CHECK(SPEC, ...) debugCheck((SPEC), __func__, file, line, __VA_ARGS__)
#else
#   define GL_CHECK(...)
#endif

typedef GLuint (APIENTRYP MYPFNGLCREATESHADERPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void (APIENTRYP MYPFNGLCOMPILESHADERPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef GLuint (APIENTRYP MYPFNGLCREATEPROGRAMPROC)();
typedef void (APIENTRYP MYPFNGLLINKPROGRAMPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLUSEPROGRAMPROC)(GLuint);
typedef GLint (APIENTRYP MYPFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void (APIENTRYP MYPFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (APIENTRYP MYPFNGLBINDBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP MYPFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
typedef void (APIENTRYP MYPFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const GLvoid*, GLenum);
typedef void (APIENTRYP MYPFNGLVALIDATEPROGRAMPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLGENVERTEXARRAYSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP MYPFNGLBINDVERTEXARRAYPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLDELETEPROGRAMPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP MYPFNGLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP MYPFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void (APIENTRYP MYPFNGLUNIFORM1FPROC)(GLint, GLfloat);
typedef void (APIENTRYP MYPFNGLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
typedef void (APIENTRYP MYPFNGLUNIFORM3FVPROC)(GLint, GLsizei, const GLfloat*);
typedef void (APIENTRYP MYPFNGLGENERATEMIPMAPPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLDELETESHADERPROC)(GLuint);
typedef void (APIENTRYP MYPFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP MYPFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void (APIENTRYP MYPFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRYP MYPFNGLACTIVETEXTUREPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLGENFRAMEBUFFERSPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP MYPFNGLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRYP MYPFNGLDRAWBUFFERSPROC)(GLsizei, const GLenum*);
typedef GLenum (APIENTRYP MYPFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLDELETEFRAMEBUFFERSPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP MYPFNGLCLEARPROC)(GLbitfield);
typedef void (APIENTRYP MYPFNGLCLEARCOLORPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRYP MYPFNGLFRONTFACEPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLCULLFACEPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLENABLEPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLDISABLEPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLDRAWELEMENTSPROC)(GLenum, GLsizei, GLenum, const GLvoid*);
typedef void (APIENTRYP MYPFNGLDEPTHMASKPROC)(GLboolean);
typedef void (APIENTRYP MYPFNGLBINDTEXTUREPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLTEXIMAGE2DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*);
typedef void (APIENTRYP MYPFNGLDELETETEXTURESPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP MYPFNGLGENTEXTURESPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP MYPFNGLTEXPARAMETERFPROC)(GLenum, GLenum, GLfloat);
typedef void (APIENTRYP MYPFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
typedef void (APIENTRYP MYPFNGLDRAWARRAYSPROC)(GLenum, GLint, GLsizei);
typedef void (APIENTRYP MYPFNGLBLENDEQUATIONPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLBLENDFUNCPROC)(GLenum, GLenum);
typedef void (APIENTRYP MYPFNGLDEPTHFUNCPROC)(GLenum);
typedef void (APIENTRYP MYPFNGLCOLORMASKPROC)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (APIENTRYP MYPFNGLREADPIXELSPROC)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
typedef void (APIENTRYP MYPFNGLVIEWPORTPROC)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRYP MYPFNGLGETINTEGERVPROC)(GLenum, GLint*);
typedef const GLubyte* (APIENTRYP MYPFNGLGETSTRINGPROC)(GLenum);
typedef const GLubyte* (APIENTRYP MYPFNGLGETSTRINGIPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLGETFLOATVPROC)(GLenum, GLfloat*);
typedef GLenum (APIENTRYP MYPFNGLGETERRORPROC)();
typedef void (APIENTRYP MYPFNGLGETTEXLEVELPARAMETERIVPROC)(GLenum, GLint, GLenum, GLint*);
typedef void (APIENTRYP MYPFNGLGETCOMPRESSEDTEXIMAGEPROC)(GLenum, GLint, GLvoid*);
typedef void (APIENTRYP MYPFNGLCOMPRESSEDTEXIMAGE2DPROC)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*);
typedef void (APIENTRYP MYPFNGLPIXELSTOREIPROC)(GLenum, GLint);
typedef void (APIENTRYP MYPFNGLSCISSORPROC)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRYP MYPFNGLPOLYGONMODEPROC)(GLenum, GLenum);
typedef void (APIENTRYP MYPFNGLHINTPROC)(GLenum, GLenum);
typedef void (APIENTRYP MYPFNGLGENQUERIESPROC)(GLsizei, GLuint*);
typedef void (APIENTRYP MYPFNGLBEGINQUERYPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLENDQUERYPROC)(GLenum, GLuint);
typedef void (APIENTRYP MYPFNGLDELETEQUERIESPROC)(GLsizei, const GLuint*);
typedef void (APIENTRYP MYPFNGLGETQUERYOBJECTUIVPROC)(GLuint, GLenum, GLuint*);
typedef void (APIENTRYP MYPFNGLFLUSHPROC)();
typedef void (APIENTRYP MYPFNGLSTENCILFUNCPROC)(GLenum, GLint, GLuint);
typedef void (APIENTRYP MYPFNGLSTENCILOPPROC)(GLenum, GLenum, GLenum);

static MYPFNGLCREATESHADERPROC              glCreateShader_             = nullptr;
static MYPFNGLSHADERSOURCEPROC              glShaderSource_             = nullptr;
static MYPFNGLCOMPILESHADERPROC             glCompileShader_            = nullptr;
static MYPFNGLATTACHSHADERPROC              glAttachShader_             = nullptr;
static MYPFNGLCREATEPROGRAMPROC             glCreateProgram_            = nullptr;
static MYPFNGLLINKPROGRAMPROC               glLinkProgram_              = nullptr;
static MYPFNGLUSEPROGRAMPROC                glUseProgram_               = nullptr;
static MYPFNGLGETUNIFORMLOCATIONPROC        glGetUniformLocation_       = nullptr;
static MYPFNGLENABLEVERTEXATTRIBARRAYPROC   glEnableVertexAttribArray_  = nullptr;
static MYPFNGLDISABLEVERTEXATTRIBARRAYPROC  glDisableVertexAttribArray_ = nullptr;
static MYPFNGLUNIFORMMATRIX4FVPROC          glUniformMatrix4fv_         = nullptr;
static MYPFNGLBINDBUFFERPROC                glBindBuffer_               = nullptr;
static MYPFNGLGENBUFFERSPROC                glGenBuffers_               = nullptr;
static MYPFNGLVERTEXATTRIBPOINTERPROC       glVertexAttribPointer_      = nullptr;
static MYPFNGLBUFFERDATAPROC                glBufferData_               = nullptr;
static MYPFNGLVALIDATEPROGRAMPROC           glValidateProgram_          = nullptr;
static MYPFNGLGENVERTEXARRAYSPROC           glGenVertexArrays_          = nullptr;
static MYPFNGLBINDVERTEXARRAYPROC           glBindVertexArray_          = nullptr;
static MYPFNGLDELETEPROGRAMPROC             glDeleteProgram_            = nullptr;
static MYPFNGLDELETEBUFFERSPROC             glDeleteBuffers_            = nullptr;
static MYPFNGLDELETEVERTEXARRAYSPROC        glDeleteVertexArrays_       = nullptr;
static MYPFNGLUNIFORM1IPROC                 glUniform1i_                = nullptr;
static MYPFNGLUNIFORM1FPROC                 glUniform1f_                = nullptr;
static MYPFNGLUNIFORM2FPROC                 glUniform2f_                = nullptr;
static MYPFNGLUNIFORM3FVPROC                glUniform3fv_               = nullptr;
static MYPFNGLGENERATEMIPMAPPROC            glGenerateMipmap_           = nullptr;
static MYPFNGLDELETESHADERPROC              glDeleteShader_             = nullptr;
static MYPFNGLGETSHADERIVPROC               glGetShaderiv_              = nullptr;
static MYPFNGLGETPROGRAMIVPROC              glGetProgramiv_             = nullptr;
static MYPFNGLGETSHADERINFOLOGPROC          glGetShaderInfoLog_         = nullptr;
static MYPFNGLACTIVETEXTUREPROC             glActiveTexture_            = nullptr;
static MYPFNGLGENFRAMEBUFFERSPROC           glGenFramebuffers_          = nullptr;
static MYPFNGLBINDFRAMEBUFFERPROC           glBindFramebuffer_          = nullptr;
static MYPFNGLFRAMEBUFFERTEXTURE2DPROC      glFramebufferTexture2D_     = nullptr;
static MYPFNGLDRAWBUFFERSPROC               glDrawBuffers_              = nullptr;
static MYPFNGLCHECKFRAMEBUFFERSTATUSPROC    glCheckFramebufferStatus_   = nullptr;
static MYPFNGLDELETEFRAMEBUFFERSPROC        glDeleteFramebuffers_       = nullptr;
static MYPFNGLCLEARPROC                     glClear_                    = nullptr;
static MYPFNGLCLEARCOLORPROC                glClearColor_               = nullptr;
static MYPFNGLFRONTFACEPROC                 glFrontFace_                = nullptr;
static MYPFNGLCULLFACEPROC                  glCullFace_                 = nullptr;
static MYPFNGLENABLEPROC                    glEnable_                   = nullptr;
static MYPFNGLDISABLEPROC                   glDisable_                  = nullptr;
static MYPFNGLDRAWELEMENTSPROC              glDrawElements_             = nullptr;
static MYPFNGLDEPTHMASKPROC                 glDepthMask_                = nullptr;
static MYPFNGLBINDTEXTUREPROC               glBindTexture_              = nullptr;
static MYPFNGLTEXIMAGE2DPROC                glTexImage2D_               = nullptr;
static MYPFNGLDELETETEXTURESPROC            glDeleteTextures_           = nullptr;
static MYPFNGLGENTEXTURESPROC               glGenTextures_              = nullptr;
static MYPFNGLTEXPARAMETERFPROC             glTexParameterf_            = nullptr;
static MYPFNGLTEXPARAMETERIPROC             glTexParameteri_            = nullptr;
static MYPFNGLDRAWARRAYSPROC                glDrawArrays_               = nullptr;
static MYPFNGLBLENDEQUATIONPROC             glBlendEquation_            = nullptr;
static MYPFNGLBLENDFUNCPROC                 glBlendFunc_                = nullptr;
static MYPFNGLDEPTHFUNCPROC                 glDepthFunc_                = nullptr;
static MYPFNGLCOLORMASKPROC                 glColorMask_                = nullptr;
static MYPFNGLREADPIXELSPROC                glReadPixels_               = nullptr;
static MYPFNGLVIEWPORTPROC                  glViewport_                 = nullptr;
static MYPFNGLGETINTEGERVPROC               glGetIntegerv_              = nullptr;
static MYPFNGLGETSTRINGPROC                 glGetString_                = nullptr;
static MYPFNGLGETSTRINGIPROC                glGetStringi_               = nullptr;
static MYPFNGLGETFLOATVPROC                 glGetFloatv_                = nullptr;
static MYPFNGLGETERRORPROC                  glGetError_                 = nullptr;
static MYPFNGLGETTEXLEVELPARAMETERIVPROC    glGetTexLevelParameteriv_   = nullptr;
static MYPFNGLGETCOMPRESSEDTEXIMAGEPROC     glGetCompressedTexImage_    = nullptr;
static MYPFNGLCOMPRESSEDTEXIMAGE2DPROC      glCompressedTexImage2D_     = nullptr;
static MYPFNGLPIXELSTOREIPROC               glPixelStorei_              = nullptr;
static MYPFNGLSCISSORPROC                   glScissor_                  = nullptr;
static MYPFNGLPOLYGONMODEPROC               glPolygonMode_              = nullptr;
static MYPFNGLHINTPROC                      glHint_                     = nullptr;
static MYPFNGLGENQUERIESPROC                glGenQueries_               = nullptr;
static MYPFNGLBEGINQUERYPROC                glBeginQuery_               = nullptr;
static MYPFNGLENDQUERYPROC                  glEndQuery_                 = nullptr;
static MYPFNGLDELETEQUERIESPROC             glDeleteQueries_            = nullptr;
static MYPFNGLGETQUERYOBJECTUIVPROC         glGetQueryObjectuiv_        = nullptr;
static MYPFNGLFLUSHPROC                     glFlush_                    = nullptr;
static MYPFNGLSTENCILFUNCPROC               glStencilFunc_              = nullptr;
static MYPFNGLSTENCILOPPROC                 glStencilOp_                = nullptr;

#ifdef DEBUG_GL
template <char C, typename T>
u::string stringize(T, char base='?');

template<>
u::string stringize<'1', GLchar>(GLchar value, char) {
    return u::format("GLchar=%c", value);
}
template<>
u::string stringize<'2', GLenum>(GLenum value, char) {
    return u::format("GLenum=0x%X", value);
}
template<>
u::string stringize<'3', GLboolean>(GLboolean value, char) {
    return u::format("GLboolean=%c", value);
}
template<>
u::string stringize<'4', GLbitfield>(GLbitfield value, char) {
    return u::format("GLbitfield=%u", value);
}
template<>
u::string stringize<'5', GLbyte>(GLbyte value, char) {
    return u::format("GLbyte=%x", value);
}
template<>
u::string stringize<'6', GLshort>(GLshort value, char) {
    return u::format("GLshort=%d", value);
}
template<>
u::string stringize<'7', GLint>(GLint value, char) {
    return u::format("GLint=%d", value);
}
template<>
u::string stringize<'8', GLsizei>(GLsizei value, char) {
    return u::format("GLsizei=%d", value);
}
template<>
u::string stringize<'9', GLubyte>(GLubyte value, char) {
    return u::format("GLubyte=%X", value);
}
template<>
u::string stringize<'a', GLushort>(GLushort value, char) {
    return u::format("GLushort=%u", value);
}
template<>
u::string stringize<'b', GLuint>(GLuint value, char) {
    return u::format("GLuint=%u", value);
}
template<>
u::string stringize<'c', GLfloat>(GLfloat value, char) {
    return u::format("GLfloat=%.2f", value);
}
template<>
u::string stringize<'d', GLclampf>(GLclampf value, char) {
    return u::format("GLclampf=%f", value);
}
template<>
u::string stringize<'e', GLintptr>(GLintptr value, char) {
    return u::format("GLintptr=%p", value);
}
template<>
u::string stringize<'f', GLsizeiptr>(GLsizeiptr value, char) {
    return u::format("GLsizeiptr=%p", value);
}
template <>
u::string stringize<'*', void *>(void *value, char base) {
    switch (base) {
        case '?': return "unknown";
        case '0': return u::format("GLvoid*=%p", value);
        case '1': return u::format("GLchar*=%p", value);
        case '2': return u::format("GLenum*=%p", value);
        case '3': return u::format("GLboolean*=%p", value);
        case '4': return u::format("GLbitfield*=%p", value);
        case '5': return u::format("GLbyte*=%p", value);
        case '6': return u::format("GLshort*=%p", value);
        case '7': return u::format("GLint*=%p", value);
        case '8': return u::format("GLsizei*=%p", value);
        case '9': return u::format("GLubyte*=%p", value);
        case 'a': return u::format("GLushort*=%p", value);
        case 'b': return u::format("GLuint*=%p", value);
        case 'c': return u::format("GLfloat*=%p", value);
        case 'd': return u::format("GLclampf*=%p", value);
        case 'e': return u::format("GLintptr*=%p", value);
        case 'f': return u::format("GLsizeiptr*=%p", value);
    }

    return u::format("GLchar*=\"%s\"", (const char *)value);
}

static const char *debugErrorString(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
    }
    return "unknown";
}

static void debugCheck(const char *spec, const char *function, const char *file, size_t line, ...) {
    GLenum error = glGetError_();
    if (error == GL_NO_ERROR)
        return;

    va_list va;
    va_start(va, line);
    u::string contents;

    for (const char *s = spec; *s; s++) {
        switch (*s) {
            case '1':
                contents += stringize<'1'>((GLchar)va_arg(va, int));
                break;
            case '2':
                contents += stringize<'2'>((GLenum)va_arg(va, unsigned int));
                break;
            case '3':
                contents += stringize<'3'>((GLboolean)va_arg(va, int));
                break;
            case '4':
                contents += stringize<'4'>((GLbitfield)va_arg(va, unsigned int));
                break;
            case '5':
                contents += stringize<'5'>((GLbyte)va_arg(va, int));
                break;
            case '6':
                contents += stringize<'6'>((GLshort)va_arg(va, int));
                break;
            case '7':
                contents += stringize<'7'>((GLint)va_arg(va, int));
                break;
            case '8':
                contents += stringize<'8'>((GLsizei)va_arg(va, int));
                break;
            case '9':
                contents += stringize<'9'>((GLubyte)va_arg(va, unsigned int));
                break;
            case 'a':
                contents += stringize<'a'>((GLushort)va_arg(va, unsigned int));
                break;
            case 'b':
                contents += stringize<'b'>((GLuint)va_arg(va, unsigned int));
                break;
            case 'c':
                contents += stringize<'c'>((GLfloat)va_arg(va, double));
                break;
            case 'd':
                contents += stringize<'d'>((GLclampf)va_arg(va, double));
                break;
            case 'e':
                contents += stringize<'e'>((GLintptr)va_arg(va, intptr_t));
                break;
            case 'f':
                contents += stringize<'f'>((GLsizeiptr)va_arg(va, intptr_t));
                break;
            case '*':
                contents += stringize<'*'>(va_arg(va, void *), s[1]);
                s++; // skip basetype spec
                break;
        }
        if (s[1])
            contents += ", ";
    }
    va_end(va);
    u::fprint(stderr, "error %s(%s) (%s:%zu) %s\n", function, contents,
        file, line, debugErrorString(error));
}
#endif

namespace gl {

static u::set<size_t> gExtensions;
static const char *kExtensions[] = {
    "GL_EXT_texture_compression_s3tc",
    "GL_EXT_texture_compression_rgtc",
    "GL_EXT_texture_filter_anisotropic",
    "GL_ARB_texture_compression_bptc",
    "GL_ARB_texture_rectangle"
};

const char *extensionString(size_t what) {
    return kExtensions[what];
}

const u::set<size_t> &extensions() {
    return gExtensions;
}

void init() {
    glCreateShader_             = (MYPFNGLCREATESHADERPROC)neoGetProcAddress("glCreateShader");
    glShaderSource_             = (MYPFNGLSHADERSOURCEPROC)neoGetProcAddress("glShaderSource");
    glCompileShader_            = (MYPFNGLCOMPILESHADERPROC)neoGetProcAddress("glCompileShader");
    glAttachShader_             = (MYPFNGLATTACHSHADERPROC)neoGetProcAddress("glAttachShader");
    glCreateProgram_            = (MYPFNGLCREATEPROGRAMPROC)neoGetProcAddress("glCreateProgram");
    glLinkProgram_              = (MYPFNGLLINKPROGRAMPROC)neoGetProcAddress("glLinkProgram");
    glUseProgram_               = (MYPFNGLUSEPROGRAMPROC)neoGetProcAddress("glUseProgram");
    glGetUniformLocation_       = (MYPFNGLGETUNIFORMLOCATIONPROC)neoGetProcAddress("glGetUniformLocation");
    glEnableVertexAttribArray_  = (MYPFNGLENABLEVERTEXATTRIBARRAYPROC)neoGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray_ = (MYPFNGLDISABLEVERTEXATTRIBARRAYPROC)neoGetProcAddress("glDisableVertexAttribArray");
    glUniformMatrix4fv_         = (MYPFNGLUNIFORMMATRIX4FVPROC)neoGetProcAddress("glUniformMatrix4fv");
    glBindBuffer_               = (MYPFNGLBINDBUFFERPROC)neoGetProcAddress("glBindBuffer");
    glGenBuffers_               = (MYPFNGLGENBUFFERSPROC)neoGetProcAddress("glGenBuffers");
    glVertexAttribPointer_      = (MYPFNGLVERTEXATTRIBPOINTERPROC)neoGetProcAddress("glVertexAttribPointer");
    glBufferData_               = (MYPFNGLBUFFERDATAPROC)neoGetProcAddress("glBufferData");
    glValidateProgram_          = (MYPFNGLVALIDATEPROGRAMPROC)neoGetProcAddress("glValidateProgram");
    glGenVertexArrays_          = (MYPFNGLGENVERTEXARRAYSPROC)neoGetProcAddress("glGenVertexArrays");
    glBindVertexArray_          = (MYPFNGLBINDVERTEXARRAYPROC)neoGetProcAddress("glBindVertexArray");
    glDeleteProgram_            = (MYPFNGLDELETEPROGRAMPROC)neoGetProcAddress("glDeleteProgram");
    glDeleteBuffers_            = (MYPFNGLDELETEBUFFERSPROC)neoGetProcAddress("glDeleteBuffers");
    glDeleteVertexArrays_       = (MYPFNGLDELETEVERTEXARRAYSPROC)neoGetProcAddress("glDeleteVertexArrays");
    glUniform1i_                = (MYPFNGLUNIFORM1IPROC)neoGetProcAddress("glUniform1i");
    glUniform1f_                = (MYPFNGLUNIFORM1FPROC)neoGetProcAddress("glUniform1f");
    glUniform2f_                = (MYPFNGLUNIFORM2FPROC)neoGetProcAddress("glUniform2f");
    glUniform3fv_               = (MYPFNGLUNIFORM3FVPROC)neoGetProcAddress("glUniform3fv");
    glGenerateMipmap_           = (MYPFNGLGENERATEMIPMAPPROC)neoGetProcAddress("glGenerateMipmap");
    glDeleteShader_             = (MYPFNGLDELETESHADERPROC)neoGetProcAddress("glDeleteShader");
    glGetShaderiv_              = (MYPFNGLGETSHADERIVPROC)neoGetProcAddress("glGetShaderiv");
    glGetProgramiv_             = (MYPFNGLGETPROGRAMIVPROC)neoGetProcAddress("glGetProgramiv");
    glGetShaderInfoLog_         = (MYPFNGLGETSHADERINFOLOGPROC)neoGetProcAddress("glGetShaderInfoLog");
    glActiveTexture_            = (MYPFNGLACTIVETEXTUREPROC)neoGetProcAddress("glActiveTexture");
    glGenFramebuffers_          = (MYPFNGLGENFRAMEBUFFERSPROC)neoGetProcAddress("glGenFramebuffers");
    glBindFramebuffer_          = (MYPFNGLBINDFRAMEBUFFERPROC)neoGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D_     = (MYPFNGLFRAMEBUFFERTEXTURE2DPROC)neoGetProcAddress("glFramebufferTexture2D");
    glDrawBuffers_              = (MYPFNGLDRAWBUFFERSPROC)neoGetProcAddress("glDrawBuffers");
    glCheckFramebufferStatus_   = (MYPFNGLCHECKFRAMEBUFFERSTATUSPROC)neoGetProcAddress("glCheckFramebufferStatus");
    glDeleteFramebuffers_       = (MYPFNGLDELETEFRAMEBUFFERSPROC)neoGetProcAddress("glDeleteFramebuffers");
    glClear_                    = (MYPFNGLCLEARPROC)neoGetProcAddress("glClear");
    glClearColor_               = (MYPFNGLCLEARCOLORPROC)neoGetProcAddress("glClearColor");
    glFrontFace_                = (MYPFNGLFRONTFACEPROC)neoGetProcAddress("glFrontFace");
    glCullFace_                 = (MYPFNGLCULLFACEPROC)neoGetProcAddress("glCullFace");
    glEnable_                   = (MYPFNGLENABLEPROC)neoGetProcAddress("glEnable");
    glDisable_                  = (MYPFNGLDISABLEPROC)neoGetProcAddress("glDisable");
    glDrawElements_             = (MYPFNGLDRAWELEMENTSPROC)neoGetProcAddress("glDrawElements");
    glDepthMask_                = (MYPFNGLDEPTHMASKPROC)neoGetProcAddress("glDepthMask");
    glBindTexture_              = (MYPFNGLBINDTEXTUREPROC)neoGetProcAddress("glBindTexture");
    glTexImage2D_               = (MYPFNGLTEXIMAGE2DPROC)neoGetProcAddress("glTexImage2D");
    glDeleteTextures_           = (MYPFNGLDELETETEXTURESPROC)neoGetProcAddress("glDeleteTextures");
    glGenTextures_              = (MYPFNGLGENTEXTURESPROC)neoGetProcAddress("glGenTextures");
    glTexParameterf_            = (MYPFNGLTEXPARAMETERFPROC)neoGetProcAddress("glTexParameterf");
    glTexParameteri_            = (MYPFNGLTEXPARAMETERIPROC)neoGetProcAddress("glTexParameteri");
    glDrawArrays_               = (MYPFNGLDRAWARRAYSPROC)neoGetProcAddress("glDrawArrays");
    glBlendEquation_            = (MYPFNGLBLENDEQUATIONPROC)neoGetProcAddress("glBlendEquation");
    glBlendFunc_                = (MYPFNGLBLENDFUNCPROC)neoGetProcAddress("glBlendFunc");
    glDepthFunc_                = (MYPFNGLDEPTHFUNCPROC)neoGetProcAddress("glDepthFunc");
    glColorMask_                = (MYPFNGLCOLORMASKPROC)neoGetProcAddress("glColorMask");
    glReadPixels_               = (MYPFNGLREADPIXELSPROC)neoGetProcAddress("glReadPixels");
    glViewport_                 = (MYPFNGLVIEWPORTPROC)neoGetProcAddress("glViewport");
    glGetIntegerv_              = (MYPFNGLGETINTEGERVPROC)neoGetProcAddress("glGetIntegerv");
    glGetString_                = (MYPFNGLGETSTRINGPROC)neoGetProcAddress("glGetString");
    glGetStringi_               = (MYPFNGLGETSTRINGIPROC)neoGetProcAddress("glGetStringi");
    glGetFloatv_                = (MYPFNGLGETFLOATVPROC)neoGetProcAddress("glGetFloatv");
    glGetError_                 = (MYPFNGLGETERRORPROC)neoGetProcAddress("glGetError");
    glGetTexLevelParameteriv_   = (MYPFNGLGETTEXLEVELPARAMETERIVPROC)neoGetProcAddress("glGetTexLevelParameteriv");
    glGetCompressedTexImage_    = (MYPFNGLGETCOMPRESSEDTEXIMAGEPROC)neoGetProcAddress("glGetCompressedTexImage");
    glCompressedTexImage2D_     = (MYPFNGLCOMPRESSEDTEXIMAGE2DPROC)neoGetProcAddress("glCompressedTexImage2D");
    glPixelStorei_              = (MYPFNGLPIXELSTOREIPROC)neoGetProcAddress("glPixelStorei");
    glScissor_                  = (MYPFNGLSCISSORPROC)neoGetProcAddress("glScissor");
    glPolygonMode_              = (MYPFNGLPOLYGONMODEPROC)neoGetProcAddress("glPolygonMode");
    glHint_                     = (MYPFNGLHINTPROC)neoGetProcAddress("glHint");
    glGenQueries_               = (MYPFNGLGENQUERIESPROC)neoGetProcAddress("glGenQueries");
    glBeginQuery_               = (MYPFNGLBEGINQUERYPROC)neoGetProcAddress("glBeginQuery");
    glEndQuery_                 = (MYPFNGLENDQUERYPROC)neoGetProcAddress("glEndQuery");
    glDeleteQueries_            = (MYPFNGLDELETEQUERIESPROC)neoGetProcAddress("glDeleteQueries");
    glGetQueryObjectuiv_        = (MYPFNGLGETQUERYOBJECTUIVPROC)neoGetProcAddress("glGetQueryObjectuiv");
    glFlush_                    = (MYPFNGLFLUSHPROC)neoGetProcAddress("glFlush");
    glStencilFunc_              = (MYPFNGLSTENCILFUNCPROC)neoGetProcAddress("glStencilFunc");
    glStencilOp_                = (MYPFNGLSTENCILOPPROC)neoGetProcAddress("glStencilOp");

    if (!glGetIntegerv_ || !glGetStringi_)
        neoFatal("Failed to initialize OpenGL\n");

    GLint count = 0;
    glGetIntegerv_(GL_NUM_EXTENSIONS, &count);
    for (GLint i = 0; i < count; i++)
        for (size_t j = 0; j < sizeof(kExtensions)/sizeof(*kExtensions); j++)
            if (!strcmp(kExtensions[j], (const char *)glGetStringi_(GL_EXTENSIONS, i)))
                gExtensions.insert(j);
}

bool has(size_t ext) {
    return gExtensions.find(ext) != gExtensions.end();
}


GLuint CreateShader(GLenum shaderType GL_INFOP) {
    GLuint result = glCreateShader_(shaderType);
    GL_CHECK("2", shaderType);
    return result;
}

void ShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length GL_INFOP) {
    glShaderSource_(shader, count, string, length);
    GL_CHECK("b8*0*7", shader, count, string, length);
}

void CompileShader(GLuint shader GL_INFOP) {
    glCompileShader_(shader);
    GL_CHECK("b", shader);
}

void AttachShader(GLuint program, GLuint shader GL_INFOP) {
    glAttachShader_(program, shader);
    GL_CHECK("bb", program, shader);
}

GLuint CreateProgram(GL_INFO) {
    GLuint result = glCreateProgram_();
    GL_CHECK("",0);
    return result;
}

void LinkProgram(GLuint program GL_INFOP) {
    glLinkProgram_(program);
    GL_CHECK("b", program);
}

void UseProgram(GLuint program GL_INFOP) {
    glUseProgram_(program);
    GL_CHECK("b", program);
}

GLint GetUniformLocation(GLuint program, const GLchar* name GL_INFOP) {
    GLint result = glGetUniformLocation_(program, name);
    GL_CHECK("b*1", program, name);
    return result;
}

void EnableVertexAttribArray(GLuint index GL_INFOP) {
    glEnableVertexAttribArray_(index);
    GL_CHECK("b", index);
}

void DisableVertexAttribArray(GLuint index GL_INFOP) {
    glDisableVertexAttribArray_(index);
    GL_CHECK("b", index);
}

void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value GL_INFOP) {
    glUniformMatrix4fv_(location, count, transpose, value);
    GL_CHECK("783*c", location, count, transpose, value);
}

void BindBuffer(GLenum target, GLuint buffer GL_INFOP) {
    glBindBuffer_(target, buffer);
    GL_CHECK("2b", target, buffer);
}

void GenBuffers(GLsizei n, GLuint* buffers GL_INFOP) {
    glGenBuffers_(n, buffers);
    GL_CHECK("8*b", n, buffers);
}

void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer GL_INFOP) {
    glVertexAttribPointer_(index, size, type, normalized, stride, pointer);
    GL_CHECK("b7238*0", index, size, type, normalized, stride, pointer);
}

void BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage GL_INFOP) {
    glBufferData_(target, size, data, usage);
    GL_CHECK("2f*02", target, size, data, usage);
}

void ValidateProgram(GLuint program GL_INFOP) {
    glValidateProgram_(program);
    GL_CHECK("b", program);
}

void GenVertexArrays(GLsizei n, GLuint* arrays GL_INFOP) {
    glGenVertexArrays_(n, arrays);
    GL_CHECK("8*b", n, arrays);
}

void BindVertexArray(GLuint array GL_INFOP) {
    glBindVertexArray_(array);
    GL_CHECK("b", array);
}

void DeleteProgram(GLuint program GL_INFOP) {
    glDeleteProgram_(program);
    GL_CHECK("b", program);
}

void DeleteBuffers(GLsizei n, const GLuint* buffers GL_INFOP) {
    glDeleteBuffers_(n, buffers);
    GL_CHECK("8*b", n, buffers);
}

void DeleteVertexArrays(GLsizei n, const GLuint* arrays GL_INFOP) {
    glDeleteVertexArrays_(n, arrays);
    GL_CHECK("8*b", n, arrays);
}

void Uniform1i(GLint location, GLint v0 GL_INFOP) {
    glUniform1i_(location, v0);
    GL_CHECK("77", location, v0);
}

void Uniform1f(GLint location, GLfloat v0 GL_INFOP) {
    glUniform1f_(location, v0);
    GL_CHECK("7c", location, v0);
}

void Uniform2f(GLint location, GLfloat v0, GLfloat v1 GL_INFOP) {
    glUniform2f_(location, v0, v1);
    GL_CHECK("7cc", location, v0, v1);
}

void Uniform3fv(GLint location, GLsizei count, const GLfloat* value GL_INFOP) {
    glUniform3fv_(location, count, value);
    GL_CHECK("78*c", location, count, value);
}

void GenerateMipmap(GLenum target GL_INFOP) {
    glGenerateMipmap_(target);
    GL_CHECK("2", target);
}

void DeleteShader(GLuint shader GL_INFOP) {
    glDeleteShader_(shader);
    GL_CHECK("b", shader);
}

void GetShaderiv(GLuint shader, GLenum pname, GLint* params GL_INFOP) {
    glGetShaderiv_(shader, pname, params);
    GL_CHECK("b2*7", shader, pname, params);
}

void GetProgramiv(GLuint program, GLenum pname, GLint* params GL_INFOP) {
    glGetProgramiv_(program, pname, params);
    GL_CHECK("b2*7", program, pname, params);
}

void GetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog GL_INFOP) {
    glGetShaderInfoLog_(shader, maxLength, length, infoLog);
    GL_CHECK("b8*8*1", shader, maxLength, length, infoLog);
}

void ActiveTexture(GLenum texture GL_INFOP) {
    glActiveTexture_(texture);
    GL_CHECK("2", texture);
}

void GenFramebuffers(GLsizei n, GLuint* ids GL_INFOP) {
    glGenFramebuffers_(n, ids);
    GL_CHECK("8*b", n, ids);
}

void BindFramebuffer(GLenum target, GLuint framebuffer GL_INFOP) {
    glBindFramebuffer_(target, framebuffer);
    GL_CHECK("2b", target, framebuffer);
}

void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level GL_INFOP) {
    glFramebufferTexture2D_(target, attachment, textarget, texture, level);
    GL_CHECK("222b7", target, attachment, textarget, texture, level);
}

void DrawBuffers(GLsizei n, const GLenum* bufs GL_INFOP) {
    glDrawBuffers_(n, bufs);
    GL_CHECK("8*2", n, bufs);
}

GLenum CheckFramebufferStatus(GLenum target GL_INFOP) {
    GLenum result = glCheckFramebufferStatus_(target);
    GL_CHECK("2", target);
    return result;
}

void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers GL_INFOP) {
    glDeleteFramebuffers_(n, framebuffers);
    GL_CHECK("8*b", n, framebuffers);
}

void Clear(GLbitfield mask GL_INFOP) {
    glClear_(mask);
    GL_CHECK("4", mask);
}

void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha GL_INFOP) {
    glClearColor_(red, green, blue, alpha);
    GL_CHECK("cccc", red, green, blue, alpha);
}

void FrontFace(GLenum mode GL_INFOP) {
    glFrontFace_(mode);
    GL_CHECK("2", mode);
}

void CullFace(GLenum mode GL_INFOP) {
    glCullFace_(mode);
    GL_CHECK("2", mode);
}

void Enable(GLenum cap GL_INFOP) {
    glEnable_(cap);
    GL_CHECK("2", cap);
}

void Disable(GLenum cap GL_INFOP) {
    glDisable_(cap);
    GL_CHECK("2", cap);
}

void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices GL_INFOP) {
    glDrawElements_(mode, count, type, indices);
    GL_CHECK("282*0", mode, count, type, indices);
}

void DepthMask(GLboolean flag GL_INFOP) {
    glDepthMask_(flag);
    GL_CHECK("3", flag);
}

void BindTexture(GLenum target, GLuint texture GL_INFOP) {
    glBindTexture_(target, texture);
    GL_CHECK("2b", target, texture);
}

void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data GL_INFOP) {
    glTexImage2D_(target, level, internalFormat, width, height, border, format, type, data);
    GL_CHECK("27788722*0", target, level, internalFormat, width, height, border, format, type, data);
}

void DeleteTextures(GLsizei n, const GLuint* textures GL_INFOP) {
    glDeleteTextures_(n, textures);
    GL_CHECK("8*b", n, textures);
}

void GenTextures(GLsizei n, GLuint* textures GL_INFOP) {
    glGenTextures_(n, textures);
    GL_CHECK("8*b", n, textures);
}

void TexParameterf(GLenum target, GLenum pname, GLfloat param GL_INFOP) {
    glTexParameterf_(target, pname, param);
    GL_CHECK("22c", target, pname, param);
}

void TexParameteri(GLenum target, GLenum pname, GLint param GL_INFOP) {
    glTexParameteri_(target, pname, param);
    GL_CHECK("227", target, pname, param);
}

void DrawArrays(GLenum mode, GLint first, GLsizei count GL_INFOP) {
    glDrawArrays_(mode, first, count);
    GL_CHECK("278", mode, first, count);
}

void BlendEquation(GLenum mode GL_INFOP) {
    glBlendEquation_(mode);
    GL_CHECK("2", mode);
}

void BlendFunc(GLenum sfactor, GLenum dfactor GL_INFOP) {
    glBlendFunc_(sfactor, dfactor);
    GL_CHECK("22", sfactor, dfactor);
}

void DepthFunc(GLenum func GL_INFOP) {
    glDepthFunc_(func);
    GL_CHECK("2", func);
}

void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha GL_INFOP) {
    glColorMask_(red, green, blue, alpha);
    GL_CHECK("3333", red, green, blue, alpha);
}

void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data GL_INFOP) {
    glReadPixels_(x, y, width, height, format, type, data);
    GL_CHECK("778822*0", x, y, width, height, format, type, data);
}

void Viewport(GLint x, GLint y, GLsizei width, GLsizei height GL_INFOP) {
    glViewport_(x, y, width, height);
    GL_CHECK("7788", x, y, width, height);
}

void GetIntegerv(GLenum pname, GLint* data GL_INFOP) {
    glGetIntegerv_(pname, data);
    GL_CHECK("2*7", pname, data);
}

const GLubyte* GetString(GLenum name GL_INFOP) {
    const GLubyte* result = glGetString_(name);
    GL_CHECK("2", name);
    return result;
}

const GLubyte* GetStringi(GLenum name, GLuint index GL_INFOP) {
    const GLubyte* result = glGetStringi_(name, index);
    GL_CHECK("2b", name, index);
    return result;
}

void GetFloatv(GLenum pname, GLfloat* params GL_INFOP) {
    glGetFloatv_(pname, params);
    GL_CHECK("2*c", pname, params);
}

GLenum GetError(GL_INFO) {
    GLenum result = glGetError_();
    GL_CHECK("",0);
    return result;
}

void GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params GL_INFOP) {
    glGetTexLevelParameteriv_(target, level, pname, params);
    GL_CHECK("272*7", target, level, pname, params);
}

void GetCompressedTexImage(GLenum target, GLint lod, GLvoid* img GL_INFOP) {
    glGetCompressedTexImage_(target, lod, img);
    GL_CHECK("27*0", target, lod, img);
}

void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data GL_INFOP) {
    glCompressedTexImage2D_(target, level, internalformat, width, height, border, imageSize, data);
    GL_CHECK("2728878*0", target, level, internalformat, width, height, border, imageSize, data);
}

void PixelStorei(GLenum pname, GLint param GL_INFOP) {
    glPixelStorei_(pname, param);
    GL_CHECK("27", pname, param);
}

void Scissor(GLint x, GLint y, GLsizei width, GLsizei height GL_INFOP) {
    glScissor_(x, y, width, height);
    GL_CHECK("7788", x, y, width, height);
}

void PolygonMode(GLenum face, GLenum mode GL_INFOP) {
    glPolygonMode_(face, mode);
    GL_CHECK("22", face, mode);
}

void Hint(GLenum target, GLenum mode GL_INFOP) {
    glHint_(target, mode);
    GL_CHECK("22", target, mode);
}

void GenQueries(GLsizei n, GLuint* ids GL_INFOP) {
    glGenQueries_(n, ids);
    GL_CHECK("8*b", n, ids);
}

void BeginQuery(GLenum target, GLuint id GL_INFOP) {
    glBeginQuery_(target, id);
    GL_CHECK("2b", target, id);
}

void EndQuery(GLenum target, GLuint id GL_INFOP) {
    glEndQuery_(target, id);
    GL_CHECK("2b", target, id);
}

void DeleteQueries(GLsizei n, const GLuint* ids GL_INFOP) {
    glDeleteQueries_(n, ids);
    GL_CHECK("8*b", n, ids);
}

void GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params GL_INFOP) {
    glGetQueryObjectuiv_(id, pname, params);
    GL_CHECK("b2*b", id, pname, params);
}

void Flush(GL_INFO) {
    glFlush_();
    GL_CHECK("",0);
}

void StencilFunc(GLenum func, GLint ref, GLuint mask GL_INFOP) {
    glStencilFunc_(func, ref, mask);
    GL_CHECK("27b", func, ref, mask);
}

void StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass GL_INFOP) {
    glStencilOp_(sfail, dpfail, dppass);
    GL_CHECK("222", sfail, dpfail, dppass);
}

}
