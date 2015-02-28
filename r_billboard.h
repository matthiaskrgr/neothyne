#ifndef R_BILLBOARD_HDR
#define R_BILLBOARD_HDR
#include "r_common.h"
#include "r_texture.h"
#include "r_method.h"

#include "u_vector.h"

#include "m_vec3.h"

namespace u {
    struct string;
}

namespace m {
    struct mat4;
}

namespace r {

struct pipeline;

struct billboardMethod : method {
    bool init();

    void setVP(const m::mat4 &vp);
    void setColorTextureUnit(int unit);

private:
    GLuint m_VPLocation;
    GLuint m_colorMapLocation;
};

struct billboard {
    billboard();
    ~billboard();

    bool load(const u::string &billboardTexture);
    bool upload();

    void render(const pipeline &pl, float size);

    // you must add all positions for this billboard before calling `upload'
    void add(const m::vec3 &position);

private:
    struct vertex {
        m::vec3 p;
        float u, v;
    };
    u::vector<m::vec3> m_positions;
    u::vector<vertex> m_vertices;
    GLuint m_vbo;
    GLuint m_vao;
    texture2D m_texture;
    billboardMethod m_method;
};

}

#endif
