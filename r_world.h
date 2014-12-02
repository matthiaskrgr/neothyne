#ifndef R_WORLD_HDR
#define R_WORLD_HDR
#include "r_ssao.h"
#include "r_skybox.h"
#include "r_billboard.h"
#include "r_geom.h"
#include "r_model.h"
#include "r_light.h"

#include "u_map.h"

#include "kdmap.h"

namespace r {

struct geomMethod : method {
    bool init(const u::vector<const char *> &defines = u::vector<const char *>());

    void setWVP(const m::mat4 &wvp);
    void setWorld(const m::mat4 &wvp);
    void setColorTextureUnit(int unit);
    void setNormalTextureUnit(int unit);
    void setSpecTextureUnit(int unit);
    void setDispTextureUnit(int unit);
    void setSpecPower(float power);
    void setSpecIntensity(float intensity);
    void setEyeWorldPos(const m::vec3 &position);
    void setParallax(float scale, float bias);

private:
    GLint m_WVPLocation;
    GLint m_worldLocation;
    GLint m_colorTextureUnitLocation;
    GLint m_normalTextureUnitLocation;
    GLint m_specTextureUnitLocation;
    GLint m_dispTextureUnitLocation;
    GLint m_specPowerLocation;
    GLint m_specIntensityLocation;
    GLint m_eyeWorldPositionLocation;
    GLint m_parallaxLocation;
};

struct finalMethod : method {
    bool init(const u::vector<const char *> &defines = u::vector<const char *>());

    void setWVP(const m::mat4 &wvp);
    void setColorTextureUnit(int unit);
    void setPerspectiveProjection(const m::perspectiveProjection &project);

private:
    GLint m_WVPLocation;
    GLint m_colorMapLocation;
    GLint m_screenSizeLocation;
};

struct finalComposite {
    finalComposite();
    ~finalComposite();

    bool init(const m::perspectiveProjection &project, GLuint depth);
    void update(const m::perspectiveProjection &project, GLuint depth);
    void bindWriting();

    GLuint texture() const;

private:
    enum {
        kBuffer,
        kDepth
    };

    void destroy();

    GLuint m_fbo;
    GLuint m_texture;
    size_t m_width;
    size_t m_height;
};

struct renderTextureBatch {
    int permute;
    size_t start;
    size_t count;
    size_t index;
    material mat; // Rendering material (world and models share this)
};

struct world : geom {
    world();
    ~world();

    bool load(const kdMap &map);
    bool upload(const m::perspectiveProjection &p);

    void addPoint(m::vec3 &thing) {
        m_mapModels.push_back({&m_models[0], {0,0,0}, {0,0,0}, thing});
    }

    void render(const rendererPipeline &p);

private:
    void scenePass(const rendererPipeline &pipeline);
    void lightPass(const rendererPipeline &pipeline);
    void finalPass(const rendererPipeline &pipeline);
    void otherPass(const rendererPipeline &pipeline);

    // world shading methods and permutations
    u::vector<geomMethod> m_geomMethods;
    u::vector<finalMethod> m_finalMethods;
    u::vector<directionalLightMethod> m_directionalLightMethods;
    pointLightMethod m_pointLightMethod;
    spotLightMethod m_spotLightMethod;
    ssaoMethod m_ssaoMethod;

    // Other things in the world to render
    skybox m_skybox;
    quad m_quad;
    sphere m_sphere;
    u::vector<billboard> m_billboards;
    u::vector<model> m_models; // All models

    struct mapModel {
        model *mesh;
        m::vec3 scale;
        m::vec3 rotate;
        m::vec3 origin;
    };

    u::vector<mapModel> m_mapModels;
    size_t m_weapon;

    // The world itself
    u::vector<uint32_t> m_indices;
    u::vector<kdBinVertex> m_vertices;
    u::vector<renderTextureBatch> m_textureBatches;
    u::map<u::string, texture2D*> m_textures2D;

    // World lights
    directionalLight m_directionalLight;
    u::vector<pointLight> m_pointLights;
    u::vector<spotLight> m_spotLights;

    gBuffer m_gBuffer;
    ssao m_ssao;
    finalComposite m_final;

    m::mat4 m_identity;
    m::frustum m_frustum;

    bool m_uploaded;
};

}

#endif
