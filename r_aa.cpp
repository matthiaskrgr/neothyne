#include "r_aa.h"

#include "m_mat.h"

namespace r {

///! aaMethod
bool aaMethod::init(const u::initializer_list<const char *> &defines) {
    if (!method::init("anti-aliasing"))
        return false;

    for (const auto &it : defines)
        method::define(it);

    if (gl::has(gl::ARB_texture_rectangle))
        method::define("HAS_TEXTURE_RECTANGLE");

    if (!addShader(GL_VERTEX_SHADER, "shaders/fxaa.vs"))
        return false;
    if (!addShader(GL_FRAGMENT_SHADER, "shaders/fxaa.fs"))
        return false;
    if (!finalize({ "position" }))
        return false;

    m_WVP = getUniform("gWVP", uniform::kMat4);
    m_colorMap = getUniform("gColorMap", uniform::kSampler);
    m_screenSize = getUniform("gScreenSize", uniform::kVec2);

    post();
    return true;
}

void aaMethod::setWVP(const m::mat4 &wvp) {
    m_WVP->set(wvp);
}

void aaMethod::setColorTextureUnit(int unit) {
    m_colorMap->set(unit);
}

void aaMethod::setPerspective(const m::perspective &p) {
    m_screenSize->set(m::vec2(p.width, p.height));
}

///! aa
aa::aa()
    : m_fbo(0)
    , m_texture(0)
    , m_width(0)
    , m_height(0)
{
}

aa::~aa() {
    destroy();
}

void aa::destroy() {
    if (m_fbo)
        gl::DeleteFramebuffers(1, &m_fbo);
    if (m_texture)
        gl::DeleteTextures(1, &m_texture);
}

void aa::update(const m::perspective &p) {
    const size_t width = p.width;
    const size_t height = p.height;

    if (m_width == width && m_height == height)
        return;

    GLenum format = gl::has(gl::ARB_texture_rectangle)
        ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;
    m_width = width;
    m_height = height;

    gl::BindTexture(format, m_texture);
    gl::TexImage2D(format, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
}

bool aa::init(const m::perspective &p) {
    m_width = p.width;
    m_height = p.height;

    gl::GenFramebuffers(1, &m_fbo);
    gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    gl::GenTextures(1, &m_texture);

    GLenum format = gl::has(gl::ARB_texture_rectangle)
        ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

    gl::BindTexture(format, m_texture);
    gl::TexImage2D(format, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl::TexParameteri(format, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl::TexParameteri(format, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl::TexParameteri(format, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::TexParameteri(format, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, format, m_texture, 0);

    static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

    gl::DrawBuffers(sizeof drawBuffers / sizeof *drawBuffers, drawBuffers);

    const GLenum status = gl::CheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        return false;

    gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    return true;
}

void aa::bindWriting() {
    gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

GLuint aa::texture() const {
    return m_texture;
}

}
