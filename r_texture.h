#ifndef R_TEXTURE_HDR
#define R_TEXTURE_HDR
#include "texture.h"
#include "r_common.h"

namespace r {

enum {
    kFilterBilinear  = 1 << 0,
    kFilterTrilinear = 1 << 1,
    kFilterAniso     = 1 << 2,
    kFilterDefault   = kFilterBilinear | kFilterTrilinear | kFilterAniso
};

struct texture2D {
    texture2D(bool mipmaps = true, int filter = kFilterDefault);
    ~texture2D();

    texture2D(texture &tex, bool mipmaps = true, int filter = kFilterDefault);

    bool load(const u::string &file);
    bool upload();
    bool cache(GLuint internal);
    void bind(GLenum unit);
    void resize(size_t width, size_t height);
    textureFormat format() const;

private:
    bool useCache();
    void applyFilter();
    bool m_uploaded;
    GLuint m_textureHandle;
    texture m_texture;
    bool m_mipmaps;
    int m_filter;
};

struct texture3D {
    texture3D();
    ~texture3D();

    bool load(const u::string &ft, const u::string &bk, const u::string &up,
              const u::string &dn, const u::string &rt, const u::string &lf);
    bool upload();
    void bind(GLenum unit);
    void resize(size_t width, size_t height);

private:
    void applyFilter();
    bool m_uploaded;
    GLuint m_textureHandle;
    texture m_textures[6];
};

}

#endif
