#include "engine.h"
#include "gui.h"

#include "c_console.h"

#include "r_stats.h"
#include "r_world.h"

#include "u_log.h"

// Debug visualizations
enum {
    kDebugDepth = 1,
    kDebugNormal,
    kDebugPosition,
    kDebugSSAO
};

VAR(int, r_fxaa, "fast approximate anti-aliasing", 0, 1, 1);
VAR(int, r_parallax, "parallax mapping", 0, 1, 1);
VAR(int, r_ssao, "screen space ambient occlusion", 0, 1, 1);
VAR(int, r_spec, "specularity mapping", 0, 1, 1);
VAR(int, r_fog, "fog", 0, 1, 1);
VAR(int, r_sm_size, "shadow map size", 16, 4096, 256);
VAR(int, r_sm_border, "shadow map border", 0, 8, 3);
VAR(int, r_vignette, "vignette", 0, 1, 1);
VAR(float, r_vignette_radius, "vignette radius", 0.25f, 1.0f, 0.90f);
VAR(float, r_vignette_softness, "vignette softness", 0.0f, 1.0f, 0.45f);
VAR(float, r_vignette_opacity, "vignette opacity", 0.0f, 1.0f, 0.5f);
VAR(float, r_sm_bias, "shadow map bias", -10.0f, 10.0f, -0.1f);
VAR(float, r_sm_poly_factor, "shadow map polygon offset factor", -1000.0f, 1000.0f, 1.0f);
VAR(float, r_sm_poly_offset, "shadow map polygon offset units", -1000.0f, 1000.0f, 0.0f);
NVAR(int, r_debug, "debug visualizations", 0, 4, 0);
NVAR(int, r_reload, "reload shaders", 0, 1, 0);

namespace r {

struct lightPermutation {
    int permute;    // flags of the permutations
};

// All the light shader permutations
enum {
    kLightPermSSAO          = 1 << 0,
    kLightPermFog           = 1 << 1,
    kLightPermDebugDepth    = 1 << 2,
    kLightPermDebugNormal   = 1 << 3,
    kLightPermDebugPosition = 1 << 4,
    kLightPermDebugSSAO     = 1 << 5
};

// The prelude defines to compile that light shader permutation
// These must be in the same order as the enums
static const char *lightPermutationNames[] = {
    "USE_SSAO",
    "USE_FOG",
    "USE_DEBUG_DEPTH",
    "USE_DEBUG_NORMAL",
    "USE_DEBUG_POSITION",
    "USE_DEBUG_SSAO"
};

// All the possible light permutations
static const lightPermutation lightPermutations[] = {
    { 0 },
    { kLightPermSSAO },
    { kLightPermSSAO | kLightPermFog },
    { kLightPermFog },
    // Debug visualizations
    { kLightPermDebugDepth },
    { kLightPermDebugNormal },
    { kLightPermDebugPosition },
    { kLightPermDebugSSAO },
};

// Generate the list of permutation names for the shader
template <typename T, size_t N, typename U>
static u::vector<const char *> generatePermutation(const T(&list)[N], const U &p) {
    u::vector<const char *> permutes;
    for (size_t i = 0; i < N; i++)
        if (p.permute & (1 << i))
            permutes.push_back(list[i]);
    return permutes;
}

// Calculate the correct permutation to use for the light buffer
static size_t lightCalculatePermutation(bool stencil) {
    for (auto &it : lightPermutations) {
        switch (r_debug) {
        case kDebugDepth:
            if (it.permute == kLightPermDebugDepth)
                return &it - lightPermutations;
            break;
        case kDebugNormal:
            if (it.permute == kLightPermDebugNormal)
                return &it - lightPermutations;
            break;
        case kDebugPosition:
            if (it.permute == kLightPermDebugPosition)
                return &it - lightPermutations;
            break;
        case kDebugSSAO:
            if (it.permute == kLightPermDebugSSAO && !stencil)
                return &it - lightPermutations;
            break;
        }
    }
    int permute = 0;
    if (r_fog)
        permute |= kLightPermFog;
    if (r_ssao && !stencil)
        permute |= kLightPermSSAO;
    for (auto &it : lightPermutations)
        if (it.permute == permute)
            return &it - lightPermutations;
    return 0;
}

static uint8_t calcTriangleSideMask(const m::vec3 &p1,
                                    const m::vec3 &p2,
                                    const m::vec3 &p3,
                                    float bias)
{
    // p1, p2, p3 are in the cubemap's local coordinate system
    // bias = border/(size - border)
    uint8_t mask = 0x3F;
    float dp1 = p1.x + p1.y, dn1 = p1.x - p1.y, ap1 = m::abs(dp1), an1 = m::abs(dn1),
          dp2 = p2.x + p2.y, dn2 = p2.x - p2.y, ap2 = m::abs(dp2), an2 = m::abs(dn2),
          dp3 = p3.x + p3.y, dn3 = p3.x - p3.y, ap3 = m::abs(dp3), an3 = m::abs(dn3);
    if (ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<4)
            | (dp1 < 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2))
            | (dp2 < 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2))
            | (dp3 < 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2));
    if (an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<4)
            | (dn1 < 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2))
            | (dn2 < 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2))
            | (dn3 < 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2));

    dp1 = p1.y + p1.z, dn1 = p1.y - p1.z, ap1 = m::abs(dp1), an1 = m::abs(dn1),
    dp2 = p2.y + p2.z, dn2 = p2.y - p2.z, ap2 = m::abs(dp2), an2 = m::abs(dn2),
    dp3 = p3.y + p3.z, dn3 = p3.y - p3.z, ap3 = m::abs(dp3), an3 = m::abs(dn3);
    if (ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<0)
            | (dp1 < 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4))
            | (dp2 < 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4))
            | (dp3 < 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4));
    if (an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<0)
            | (dn1 < 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4))
            | (dn2 < 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4))
            | (dn3 < 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4));

    dp1 = p1.z + p1.x, dn1 = p1.z - p1.x, ap1 = m::abs(dp1), an1 = m::abs(dn1),
    dp2 = p2.z + p2.x, dn2 = p2.z - p2.x, ap2 = m::abs(dp2), an2 = m::abs(dn2),
    dp3 = p3.z + p3.x, dn3 = p3.z - p3.x, ap3 = m::abs(dp3), an3 = m::abs(dn3);
    if (ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<2)
            | (dp1 < 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0))
            | (dp2 < 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0))
            | (dp3 < 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0));
    if (an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<2)
            | (dn1 < 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0))
            | (dn2 < 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0))
            | (dn3 < 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0));

    return mask;
}

///! lightChunk
World::LightChunk::LightChunk()
    : hash(0)
    , count(0)
    , memory(0)
    , collect(false)
    , visible(false)
    , ebo(0)
    , stats(nullptr)
{
}

World::LightChunk::~LightChunk() {
    if (ebo)
        gl::DeleteBuffers(1, &ebo);
    // adjust statistics accordingly
    if (stats) {
        stats->decInstances();
        stats->decIBOMemory(memory);
    }
}

bool World::LightChunk::init(const char *name, const char *description) {
    stats = r::stat::add(name, description);
    gl::GenBuffers(1, &ebo);
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    return true;
}

///! spotLightChunk
World::SpotLightChunk::SpotLightChunk()
    : light(nullptr)
{
}

World::SpotLightChunk::SpotLightChunk(const spotLight *light)
    : light(light)
{
}

bool World::SpotLightChunk::buildMesh(kdMap *map) {
    if (hash == 0 && !init("slshadow", "Spot Light Shadows"))
        return false;
    if (memory)
        stats->decIBOMemory(memory);
    u::vector<size_t> triangleIndices;
    u::vector<GLuint> indices;
    map->inSphere(triangleIndices, light->position, light->radius);
    indices.reserve(triangleIndices.size() * 3 / 2);
    for (const auto &it : triangleIndices) {
        const auto &triangle = map->triangles[it];
        const m::vec3 p1 = map->vertices[triangle.v[0]].vertex - light->position;
        const m::vec3 p2 = map->vertices[triangle.v[1]].vertex - light->position;
        const m::vec3 p3 = map->vertices[triangle.v[2]].vertex - light->position;
        if (p1 * (p2 - p1).cross(p3 - p1) > 0)
            continue;
        for (const auto &it : triangle.v)
            indices.push_back(it);
    }
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof indices[0],
        &indices[0], GL_STATIC_DRAW);
    memory = indices.size() * sizeof indices[0];
    count = indices.size();
    stats->incIBOMemory(memory);
    return true;
}

///! pointLightChunk
World::PointLightChunk::PointLightChunk()
    : light(nullptr)
{
    memset(sideCounts, 0, sizeof sideCounts);
}

World::PointLightChunk::PointLightChunk(const pointLight *light)
    : light(light)
{
    memset(sideCounts, 0, sizeof sideCounts);
}

bool World::PointLightChunk::buildMesh(kdMap *map) {
    if (hash == 0 && !init("plshadow", "Point Light Shadows"))
        return false;
    // rebuilding mesh: throw away old memory statistics
    if (memory)
        stats->decIBOMemory(memory);

    u::vector<size_t> triangleIndices;
    u::vector<GLuint> indices[6];

    map->inSphere(triangleIndices, light->position, light->radius);

    for (size_t side = 0; side < 6; ++side)
        indices[side].reserve(triangleIndices.size() * 3 / 6);
    for (const auto &it : triangleIndices) {
        const auto &triangle = map->triangles[it];
        const m::vec3 p1 = map->vertices[triangle.v[0]].vertex - light->position;
        const m::vec3 p2 = map->vertices[triangle.v[1]].vertex - light->position;
        const m::vec3 p3 = map->vertices[triangle.v[2]].vertex - light->position;
        if (p1 * (p2 - p1).cross(p3 - p1) > 0)
            continue;
        const uint8_t mask = calcTriangleSideMask(p1, p2, p3, r_sm_border / float(r_sm_size - r_sm_border));
        for (size_t side = 0; side < 6; ++side) {
            if (mask & (1 << side)) {
                for (const auto &it : triangle.v)
                    indices[side].push_back(it);
            }
        }
    }
    count = 0;
    memory = 0;
    for (size_t side = 0; side < 6; ++side) {
        sideCounts[side] = indices[side].size();
        count += sideCounts[side];
    }
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof indices[0][0], nullptr, GL_STATIC_DRAW);
    size_t offset = 0;
    for (size_t side = 0; side < 6; ++side) {
        if (sideCounts[side] > 0) {
            gl::BufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof indices[0][0],
                sideCounts[side] * sizeof indices[0][0], &indices[side][0]);
            memory += sideCounts[side] * sizeof indices[0][0];
        }
        offset += sideCounts[side];
    }
    stats->incIBOMemory(memory);
    return true;
}

///! modelChunk
World::ModelChunk::ModelChunk()
    : frame(0.0f)
    , collect(false)
    , highlight(false)
    , visible(false)
    , model(nullptr)
{
}

World::ModelChunk::ModelChunk(const r::model *model,
                              bool highlight,
                              const m::vec3 &position,
                              const m::vec3 &scale,
                              const m::vec3 &rotate)
    : position(position)
    , scale(scale)
    , rotate(rotate)
    , frame(0.0f)
    , collect(false)
    , highlight(highlight)
    , visible(false)
    , model(model)
{
}

/// NOTE: Testing only
struct dustSystem final : particleSystem {
    dustSystem(const m::vec3 &ownerPosition);
protected:
    void initParticle(particle &p, const m::vec3 &ownerPosition);
    virtual float gravity() final { return 98.0f; }
private:
    m::vec3 m_direction;
};

dustSystem::dustSystem(const m::vec3 &ownerPosition)
{
#if 0
    static constexpr size_t kParticles = 128;
    m_particles.reserve(kParticles);
    for (size_t i = 0; i < kParticles-1; i++) {
        particle p;
        initParticle(p, ownerPosition);
        addParticle(u::move(p));
    }
#endif
}

void dustSystem::initParticle(particle &p, const m::vec3 &ownerPosition) {
    p.startAlpha = 1.0f;
    p.alpha = 1.0f;
    p.color = { 1.0f, 1.0f, 1.0f };
    p.totalLifeTime = 0.3f + u::randf() * 0.9f;
    p.lifeTime = p.totalLifeTime;
    p.origin = m::vec3::rand(0.05f, 0.0f, 0.05f) + ownerPosition;
    p.origin.y = ownerPosition.y;
    p.size = 10.0f;
    p.startSize = 10.0f;
    p.velocity = (m_direction + m::vec3::rand(1.5f, 0.0f, 1.5f)).normalized() * 10.0f;
    p.velocity.y = 0.1f;
    p.respawn = true;
    p.visible = true;
}

///! world
static constexpr float kLightRadiusTweak = 1.11f;

// light entities
void World::addPointLight(r::pointLight *light) {
    // ignore adding it again
    auto find = m_culledPointLights.find(light);
    if (find != m_culledPointLights.end()) {
        find->second->collect = false;
        return;
    }
    m_culledPointLights.insert( { light, new PointLightChunk(light) } );
}

void World::addSpotLight(r::spotLight *light) {
    // ignore adding it again
    auto find = m_culledSpotLights.find(light);
    if (find != m_culledSpotLights.end()) {
        find->second->collect = false;
        return;
    }
    m_culledSpotLights.insert( { light, new SpotLightChunk(light) } );
}

void World::addBillboard(r::billboard *billboard) {
    auto find = m_billboards.find(billboard);
    if (find != m_billboards.end()) {
        find->second.first = false;
        return;
    }
    // upload and insert the billboard
    billboard->upload();
    m_billboards.insert({ billboard, { false, billboard } });
}

void World::addModel(r::model *model,
                     bool highlight,
                     const m::vec3 &position,
                     const m::vec3 &scale,
                     const m::vec3 &rotate)
{
    auto find = m_models.find(model);
    if (find != m_models.end()) {
        find->second->collect = false;
        // update properties of existing
        find->second->highlight = highlight;
        find->second->position = position;
        find->second->rotate = rotate;
        return;
    }
    // upload and insert the model
    model->upload();
    m_models.insert({ model, new ModelChunk(model, highlight, position, scale, rotate) });
}

// global entities
fog &World::getFog() {
    return m_fog;
}

directionalLight *World::getDirectionalLight() {
    return &m_directionalLight;
}

ColorGrader *World::getColorGrader() {
    return &m_grader;
}

///! world
World::World()
    : m_geomMethods(&geomMethods::instance())
    , m_gun(nullptr)
    , m_kdWorld(nullptr)
    , m_uploaded(false)
    , m_stats(nullptr)
{
}

World::~World() {
    unload(false);
}

bool World::load(kdMap *map) {
    // load skybox
    if (!m_skybox.load("textures/sky01"))
        return false;

    // make rendering batches for triangles which share the same texture
    for (size_t i = 0; i < map->textures.size(); i++) {
        renderTextureBatch batch;
        batch.start = m_indices.size();
        batch.index = i;
        for (size_t j = 0; j < map->triangles.size(); j++) {
            if (map->triangles[j].texture == i)
                for (size_t k = 0; k < 3; k++)
                    m_indices.push_back(map->triangles[j].v[k]);
        }
        batch.count = m_indices.size() - batch.start;
        m_textureBatches.push_back(batch);
    }

    m_kdWorld = map;

    // load materials
    for (auto &it : m_textureBatches) {
        if (!it.mat.load(m_textures2D, map->textures[it.index].name, "textures/"))
            neoFatal("failed to load material\n");
        it.mat.calculatePermutation();
    }

    // HACK: Testing only
    m_gun = new model;
    if (!m_gun->load(m_textures2D, "models/lg"))
        neoFatal("failed to load gun");

    m_stats = r::stat::add("world", "Map");
    u::Log::out("[world] => loaded\n");
    return true;
}

bool World::upload(const m::perspective &p) {
    if (m_uploaded)
        return true;

    // hack
    m_particleSystems.push_back(new dustSystem(m::vec3::origin));

    // particles for entities
    for (auto *it : m_particleSystems)
        it->load("textures/particle"); // TODO: texture cache for particles

    // upload skybox
    if (!m_skybox.upload())
        neoFatal("failed to upload skybox");
    if (!m_quad.upload())
        neoFatal("failed to upload quad");
    if (!m_sphere.upload())
        neoFatal("failed to upload sphere");
    if (!m_bbox.upload())
        neoFatal("failed to upload bbox");
    if (!m_cone.upload())
        neoFatal("failed to upload cone");

    // HACK: Testing only
    if (!m_gun->upload())
        neoFatal("failed to upload gun");

    // upload particle systems
    for (auto *it : m_particleSystems)
        it->upload();

    // upload materials
    for (auto &it : m_textureBatches)
        if (!it.mat.upload())
            neoFatal("failed to upload world materials");

    // count textures and memory utilization for map
    for (const auto &it : m_textures2D) {
        m_stats->incTextureCount();
        m_stats->incTextureMemory(it.second->memory());
    }

    geom::upload();

    const auto &vertices = m_kdWorld->vertices;
    gl::BindVertexArray(vao);
    gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
    gl::BufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(kdBinVertex), &vertices[0], GL_STATIC_DRAW);
    gl::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(kdBinVertex), u::offset_of(&kdBinVertex::vertex));  // vertex
    gl::VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(kdBinVertex), u::offset_of(&kdBinVertex::normal));  // normals
    gl::VertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(kdBinVertex), u::offset_of(&kdBinVertex::coordinate));  // texCoord
    gl::VertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(kdBinVertex), u::offset_of(&kdBinVertex::tangent));  // tangent
    gl::EnableVertexAttribArray(0);
    gl::EnableVertexAttribArray(1);
    gl::EnableVertexAttribArray(2);
    gl::EnableVertexAttribArray(3);

    m_stats->incVBOMemory(vertices.size() * sizeof(kdBinVertex));

    // upload data
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof m_indices[0], &m_indices[0], GL_STATIC_DRAW);

    m_stats->incIBOMemory(m_indices.size() * sizeof m_indices[0]);

    // composite shader
    if (!m_compositeMethod.init())
        neoFatal("failed to initialize composite rendering method");
    m_compositeMethod.enable();
    m_compositeMethod.setColorTextureUnit(0);
    m_compositeMethod.setColorGradingTextureUnit(1);
    m_compositeMethod.setWVP(m::mat4::kIdentity);

    // aa shader
    if (!m_aaMethod.init())
        neoFatal("failed to initialize aa rendering method");
    m_aaMethod.enable();
    m_aaMethod.setColorTextureUnit(0);
    m_aaMethod.setWVP(m::mat4::kIdentity);

    if (!m_geomMethods->init())
        neoFatal("failed to initialize geometry rendering method");

    // directional light shader permutations
    static const size_t lightCount = sizeof lightPermutations / sizeof *lightPermutations;
    m_directionalLightMethods.resize(lightCount);
    for (size_t i = 0; i < lightCount; i++) {
        const auto &p = lightPermutations[i];
        if (!m_directionalLightMethods[i].init(generatePermutation(lightPermutationNames, p)))
            neoFatal("failed to initialize light rendering method");
        m_directionalLightMethods[i].enable();
        m_directionalLightMethods[i].setWVP(m::mat4::kIdentity);
        m_directionalLightMethods[i].setColorTextureUnit(lightMethod::kColor);
        m_directionalLightMethods[i].setNormalTextureUnit(lightMethod::kNormal);
        m_directionalLightMethods[i].setDepthTextureUnit(lightMethod::kDepth);
        m_directionalLightMethods[i].setOcclusionTextureUnit(lightMethod::kOcclusion);
    }

    // point light method
    if (!m_pointLightMethods[0].init())
        neoFatal("failed to initialize point-light rendering method");
    m_pointLightMethods[0].enable();
    m_pointLightMethods[0].setColorTextureUnit(lightMethod::kColor);
    m_pointLightMethods[0].setNormalTextureUnit(lightMethod::kNormal);
    m_pointLightMethods[0].setDepthTextureUnit(lightMethod::kDepth);

    if (!m_pointLightMethods[1].init({"USE_SHADOWMAP"}))
        neoFatal("failed to initialize point-light rendering method");
    m_pointLightMethods[1].enable();
    m_pointLightMethods[1].setColorTextureUnit(lightMethod::kColor);
    m_pointLightMethods[1].setNormalTextureUnit(lightMethod::kNormal);
    m_pointLightMethods[1].setDepthTextureUnit(lightMethod::kDepth);
    m_pointLightMethods[1].setShadowMapTextureUnit(lightMethod::kShadowMap);

    // spot light method
    if (!m_spotLightMethods[0].init())
        neoFatal("failed to initialize spotlight rendering method");
    m_spotLightMethods[0].enable();
    m_spotLightMethods[0].setColorTextureUnit(lightMethod::kColor);
    m_spotLightMethods[0].setNormalTextureUnit(lightMethod::kNormal);
    m_spotLightMethods[0].setDepthTextureUnit(lightMethod::kDepth);

    if (!m_spotLightMethods[1].init({"USE_SHADOWMAP"}))
        neoFatal("failed to initialize spotlight rendering method");
    m_spotLightMethods[1].enable();
    m_spotLightMethods[1].setColorTextureUnit(lightMethod::kColor);
    m_spotLightMethods[1].setNormalTextureUnit(lightMethod::kNormal);
    m_spotLightMethods[1].setDepthTextureUnit(lightMethod::kDepth);
    m_spotLightMethods[1].setShadowMapTextureUnit(lightMethod::kShadowMap);

    // bbox method
    if (!m_bboxMethod.init())
        neoFatal("failed to initialize bounding box rendering method");
    m_bboxMethod.enable();
    m_bboxMethod.setColor({1.0f, 1.0f, 1.0f}); // White by default

    // default method
    if (!m_defaultMethod.init())
        neoFatal("failed to initialize default rendering method");
    m_defaultMethod.enable();
    m_defaultMethod.setColorTextureUnit(0);

    // ssao method
    if (!m_ssaoMethod.init())
        neoFatal("failed to initialize ambient-occlusion rendering method");

    // Setup default uniforms for ssao
    m_ssaoMethod.enable();
    m_ssaoMethod.setWVP(m::mat4::kIdentity);
    m_ssaoMethod.setOccluderBias(0.05f);
    m_ssaoMethod.setSamplingRadius(15.0f);
    m_ssaoMethod.setAttenuation(1.5f, 0.0000005f);
    m_ssaoMethod.setNormalTextureUnit(ssaoMethod::kNormal);
    m_ssaoMethod.setDepthTextureUnit(ssaoMethod::kDepth);
    m_ssaoMethod.setRandomTextureUnit(ssaoMethod::kRandom);

    // vignette method
    if (!m_vignetteMethod.init())
        neoFatal("failed to initialize vignette rendering method");

    // Setup default uniforms for vignette
    m_vignetteMethod.enable();
    m_vignetteMethod.setWVP(m::mat4::kIdentity);
    m_vignetteMethod.setColorTextureUnit(0);

    // render buffers
    if (!m_gBuffer.init(p))
        neoFatal("failed to initialize world render buffer");
    if (!m_ssao.init(p))
        neoFatal("failed to initialize ambient-occlusion render buffer");
    if (!m_final.init(p, m_gBuffer.texture(gBuffer::kDepth)))
        neoFatal("failed to initialize final composite render buffer");
    if (!m_aa.init(p))
        neoFatal("failed to initialize anti-aliasing render buffer");
    if (!m_colorGrader.init(p, m_grader.data()))
        neoFatal("failed to initialize color grading render buffer");
    if (!m_vignette.init(p))
        neoFatal("failed to initialize vignette render buffer");

    if (!m_shadowMap.init(r_sm_size*3, r_sm_size*2))
        neoFatal("failed to initialize shadow map");
    if (!m_shadowMapMethod.init())
        neoFatal("failed to initialize shadow map method");
    m_shadowMapMethod.enable();
    m_shadowMapMethod.setWVP(m::mat4::kIdentity);

    u::Log::out("[world] => uploaded\n");
    return m_uploaded = true;
}

void World::unload(bool destroy) {
    // Note: must be before deleting the textures below
    delete m_gun;

    for (auto &it : m_culledPointLights) delete it.second;
    for (auto &it : m_culledSpotLights)  delete it.second;
    for (auto &it : m_models)            delete it.second;

    for (auto &it : m_textures2D) {
        m_stats->decTextureCount();
        m_stats->decTextureMemory(it.second->memory());
        delete it.second;
    }

    for (auto *it : m_particleSystems)
        delete it;

    if (destroy) {
        m_culledPointLights.clear();
        m_culledSpotLights.clear();
        m_models.clear();
        m_billboards.clear();
        m_indices.destroy();
        m_textureBatches.destroy();
        m_textures2D.clear();
    }

    m_stats->decIBOMemory(m_indices.size() * sizeof m_indices[0]);
    m_stats->decVBOMemory(m_kdWorld->vertices.size() * sizeof(kdBinVertex));

    m_uploaded = false;
    u::Log::out("[world] => unloaded\n");
}

void World::render(const pipeline &pl) {
    m_frustum.update(pl.projection() * pl.view() * pl.world());

    // the job of the game code is to call reset before the start
    // of every world frame and add all entities to the world
    // before calling render.
    //
    // the world in a sense manages internal entities and doesn't actually
    // remove entities during reset but rather marks them to be collected
    // here.
    //
    // the idea is the game code will add the entities back which will
    // unmark the collect flag preventing them from being removed
    u::vector<SpotLightChunk*> removeSpotLights;
    u::vector<PointLightChunk*> removePointLights;
    u::vector<r::model*> removeModels;
    u::vector<r::billboard*> removeBillboards;
    for (auto it = m_models.begin(); it != m_models.end(); ++it)
        if (it->second->collect)
            removeModels.push_back(it->first);
    for (auto it = m_culledSpotLights.begin(); it != m_culledSpotLights.end(); ++it)
        if (it->second->collect)
            removeSpotLights.push_back(it->second);
    for (auto it = m_culledPointLights.begin(); it != m_culledPointLights.end(); ++it)
        if (it->second->collect)
            removePointLights.push_back(it->second);
    for (auto it = m_billboards.begin(); it != m_billboards.end(); ++it)
        if (it->second.first)
            removeBillboards.push_back(it->first);
    for (auto *it : removeSpotLights) {
        m_culledSpotLights.erase(m_culledSpotLights.find((r::spotLight*)it->light));
        delete it;
    }
    for (auto *it : removePointLights) {
        m_culledPointLights.erase(m_culledPointLights.find((r::pointLight*)it->light));
        delete it;
    }
    for (const auto &it : removeModels) {
        m_models.erase(m_models.find(it));
        delete it;
    }
    for (const auto &it : removeBillboards) {
        // note: not deleted like others since this references the game's
        // billboard state directly
        m_billboards.erase(m_billboards.find(it));
    }

    // TODO: commands (u::function needs to be implemented)
    if (r_reload) {
        m_geomMethods->reload();
        for (auto &it : m_directionalLightMethods)
            it.reload();
        m_compositeMethod.reload();
        for (auto &it : m_pointLightMethods)
            it.reload();
        for (auto &it : m_spotLightMethods)
            it.reload();
        m_ssaoMethod.reload();
        m_bboxMethod.reload();
        m_aaMethod.reload();
        m_defaultMethod.reload();
        r_reload.set(0);
    }

    cullPass(pl);
    geometryPass(pl);
    lightingPass(pl);
    forwardPass(pl);
    compositePass(pl);
}

void World::cullPass(const pipeline &pl) {
    const float widthOffset = 0.5f * m_shadowMap.widthScale(r_sm_size);
    const float heightOffset = 0.5f * m_shadowMap.heightScale(r_sm_size);
    const float widthScale = 0.5f * m_shadowMap.widthScale(r_sm_size - r_sm_border);
    const float heightScale = 0.5f * m_shadowMap.heightScale(r_sm_size - r_sm_border);

    // cull spot lights (and calculate theit transform)
    for (auto &pair : m_culledSpotLights) {
        auto &it = *pair.second;
        const auto &light = pair.first;
        const float scale = light->radius * kLightRadiusTweak;
        it.visible = m_frustum.testSphere(light->position, scale);
        const auto hash = light->hash();
        if (it.visible && light->castShadows && it.hash != hash) {
            it.buildMesh(m_kdWorld);
            it.hash = hash;
            it.transform = m::mat4::translate({widthOffset, heightOffset, 0.5f}) *
                           m::mat4::scale({widthScale, heightScale, 0.5f}) *
                           m::mat4::project(light->cutOff, 1.0f / light->radius, m::sqrt(3.0f), r_sm_bias / light->radius) *
                           m::mat4::lookat(light->direction, m::vec3::yAxis) *
                           m::mat4::scale(1.0f / light->radius) *
                           m::mat4::translate(-light->position);
        }
    }

    // cull point lights (and calculate their transform)
    for (auto &pair : m_culledPointLights) {
        auto &it = *pair.second;
        const auto &light = pair.first;
        const float scale = light->radius * kLightRadiusTweak;
        it.visible = m_frustum.testSphere(light->position, scale);
        const auto hash = light->hash();
        if (it.visible && light->castShadows && it.hash != hash) {
            it.buildMesh(m_kdWorld);
            it.hash = hash;
            it.transform = m::mat4::translate({widthOffset, heightOffset, 0.5f}) *
                           m::mat4::scale({widthScale, heightScale, 0.5f}) *
                           m::mat4::project(90.0f, 1.0f / light->radius, m::sqrt(3.0f), r_sm_bias / light->radius) *
                           m::mat4::scale(1.0f / light->radius);
        }
    }

    // cull models (and calculate their pipeline)
    for (auto &pair : m_models) {
        auto &it = *pair.second;
        auto &mdl = *pair.first;

        it.pipeline = pl;
        it.pipeline.setWorld(it.position);
        it.pipeline.setScale(it.scale + mdl.scale);

        const m::vec3 rot = it.rotate + mdl.rotate;
        const m::quat rx(m::toRadian(rot.x), m::vec3::xAxis);
        const m::quat ry(m::toRadian(rot.y), m::vec3::yAxis);
        const m::quat rz(m::toRadian(rot.z), m::vec3::zAxis);
        m::mat4 rotate = (rz * ry * rx).getMatrix();
        it.pipeline.setRotate(rotate);

        it.visible = m_frustum.testBox(mdl.bounds().transform(it.pipeline.world()));
    }
}

void World::geometryPass(const pipeline &pl) {
    // The scene pass will be writing into the gbuffer
    m_gBuffer.update(pl.perspective());
    m_gBuffer.bindWriting();

    // Clear the depth and color buffers. This is a new scene pass.
    // We need depth testing as the scene pass will write into the depth
    // buffer. Blending isn't needed.
    gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    gl::Enable(GL_DEPTH_TEST);
    gl::Disable(GL_BLEND);

    // Render the map
    gl::BindVertexArray(vao);
    for (auto &it : m_textureBatches) {
        it.mat.bind(pl, pl.world());
        gl::DrawElements(GL_TRIANGLES, it.count, GL_UNSIGNED_INT,
            (const GLvoid*)(it.start * sizeof m_indices[0]));
    }

#if 0
    // Render map models
    for (const auto &pair : m_models) {
        auto &it = *pair.second;
        auto &mdl = *pair.first;

        it.pipeline = pl;
        it.pipeline.setWorld(it.position);
        it.pipeline.setScale(it.scale + mdl.scale);

        const m::vec3 rot = it.rotate + mdl.rotate;
        const m::quat rx(m::toRadian(rot.x), m::vec3::xAxis);
        const m::quat ry(m::toRadian(rot.y), m::vec3::yAxis);
        const m::quat rz(m::toRadian(rot.z), m::vec3::zAxis);
        m::mat4 rotate = (rz * ry * rx).getMatrix();
        it.pipeline.setRotate(rotate);

        //it.visible = m_frustum.testBox(mdl.bounds().transform(it.pipeline.world()));

        mdl.render(it.pipeline, pl.world());
    }
#endif

    // Only the scene pass needs to write to the depth buffer
    gl::Disable(GL_DEPTH_TEST);

    // Screen space ambient occlusion pass
    if (r_ssao) {
        // Read from the gbuffer, write to the ssao pass
        m_ssao.update(pl.perspective());
        m_ssao.bindWriting();

        // Write to SSAO
        GLenum format = gl::has(gl::ARB_texture_rectangle)
            ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

        // Bind normal/depth/random
        gl::ActiveTexture(GL_TEXTURE0 + ssaoMethod::kNormal);
        gl::BindTexture(format, m_gBuffer.texture(gBuffer::kNormal));
        gl::ActiveTexture(GL_TEXTURE0 + ssaoMethod::kDepth);
        gl::BindTexture(format, m_gBuffer.texture(gBuffer::kDepth));
        gl::ActiveTexture(GL_TEXTURE0 + ssaoMethod::kRandom);
        gl::BindTexture(format, m_ssao.texture(ssao::kRandom));

        m_ssaoMethod.enable();
        m_ssaoMethod.setPerspective(pl.perspective());
        m_ssaoMethod.setInverse((pl.projection() * pl.view()).inverse());

        m_quad.render();

        // TODO: X, Y + Blur
    }

    // Draw HUD elements to g-buffer
    if (r_debug != kDebugSSAO) {
        m_gBuffer.bindWriting();

        // Stencil test
        gl::Enable(GL_STENCIL_TEST);
        gl::Enable(GL_DEPTH_TEST);

        // Write 1s to stencil
        gl::StencilFunc(GL_ALWAYS, 1, 0xFF); // Stencil to 1
        gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
#if 1
        // NOTE: Testing Only
        {
            pipeline p = pl;
            p.setRotation(m::quat());
            const m::vec3 rot = m::vec3(0, 180, 0);
            const m::quat rx(m::toRadian(rot.x), m::vec3::xAxis);
            const m::quat ry(m::toRadian(rot.y), m::vec3::yAxis);
            const m::quat rz(m::toRadian(rot.z), m::vec3::zAxis);
            m::mat4 rotate = (rz * ry * rx).getMatrix();
            p.setRotate(rotate);
            p.setScale({0.1, 0.1, 0.1});
            p.setPosition({-0.15, 0.2, -0.35});
            p.setWorld({0, 0, 0});
            m_gun->render(p, pl.world());
        }
#endif

        gl::Disable(GL_STENCIL_TEST);
        gl::Disable(GL_DEPTH_TEST);
    }
}

void World::lightingPass(const pipeline &pl) {
    // Write to the final composite
    m_final.update(pl.perspective());
    m_final.bindWriting();

    // Lighting will require blending
    gl::Enable(GL_BLEND);
    gl::BlendEquation(GL_FUNC_ADD);
    gl::BlendFunc(GL_ONE, GL_ONE);

    // Clear the final composite
    gl::Clear(GL_COLOR_BUFFER_BIT);

    // Need to read from the gbuffer and ssao buffer to do lighting
    GLenum format = gl::has(gl::ARB_texture_rectangle)
        ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

    gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kColor);
    gl::BindTexture(format, m_gBuffer.texture(gBuffer::kColor));
    gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kNormal);
    gl::BindTexture(format, m_gBuffer.texture(gBuffer::kNormal));
    gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kDepth);
    gl::BindTexture(format, m_gBuffer.texture(gBuffer::kDepth));

    if (!r_debug) {
        gl::Enable(GL_DEPTH_TEST);

        pointLightPass(pl);
        spotLightPass(pl);

        gl::Disable(GL_DEPTH_TEST);
    }

    // Change the blending function such that point and spot lights get fogged
    // as well.
    if (r_fog)
        gl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Two lighting passes for directional light (stencil and non stencil)
    gl::Enable(GL_STENCIL_TEST);
    gl::StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    directionalLightPass(pl, true);
    directionalLightPass(pl, false);

    gl::Disable(GL_STENCIL_TEST);
}

void World::forwardPass(const pipeline &pl) {
    gl::Enable(GL_DEPTH_TEST);

    // Skybox:
    //  Forwarded rendered and the last thing rendered to prevent overdraw.
    m_skybox.render(pl, m_fog);

    // Editing aids
    static constexpr m::vec3 kHighlighted = { 1.0f, 0.0f, 0.0f };
    static constexpr m::vec3 kOutline = { 0.0f, 0.0f, 1.0f };

    gl::DepthMask(GL_FALSE);
    if (c::Console::value<int>("cl_edit").get()) {
        // World billboards:
        //  All billboards have pre-multiplied alpha to prevent strange blending
        //  issues around the edges.
        //
        //  Billboards are represented as one face so we have to disable
        //  culling too
        gl::Disable(GL_CULL_FACE);
        gl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        for (auto &board : m_billboards)
            board.first->render(pl, 5.0f);
        gl::Enable(GL_CULL_FACE);

#if 0
        // Map models
        for (auto &mdl : models) {
            const auto &mdl = m_models[it->name];

            pipeline p = pl;
            p.setWorld(it->position);
            p.setScale(it->scale + mdl->scale);

            const m::vec3 rot = mdl->rotate + it->rotate;
            const m::quat rx(m::toRadian(rot.x), m::vec3::xAxis);
            const m::quat ry(m::toRadian(rot.y), m::vec3::yAxis);
            const m::quat rz(m::toRadian(rot.z), m::vec3::zAxis);
            m::mat4 rotate = (rz * ry * rx).getMatrix();
            p.setRotate(rotate);

            pipeline bp;
            bp.setWorld(mdl->bounds().center());
            bp.setScale(mdl->bounds().size());
            m_bboxMethod.enable();
            m_bboxMethod.setColor(it->highlight ? kHighlighted : kOutline);
            m_bboxMethod.setWVP((p.projection() * p.view() * p.world()) * bp.world());
            m_bbox.render();
        }
#endif

        // Lights
        gl::Disable(GL_CULL_FACE);
        gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        m_bboxMethod.enable();
        m_bboxMethod.setColor(kHighlighted);
        for (const auto &pair : m_culledPointLights) {
            const auto &chunk = *pair.second;
            const auto &it = pair.first;
            if (!chunk.visible || !it->highlight)
                continue;
            const float scale = it->radius * kLightRadiusTweak;
            pipeline p = pl;
            p.setWorld(it->position);
            p.setScale({scale, scale, scale});
            m_bboxMethod.setWVP(p.projection() * p.view() * p.world());
            m_sphere.render();
        }

        for (const auto &pair : m_culledSpotLights) {
            const auto &chunk = *pair.second;
            const auto &it = pair.first;
            if (!chunk.visible || it->highlight)
                continue;
            const float scale = it->radius * kLightRadiusTweak;
            pipeline p = pl;
            p.setWorld(it->position);
            p.setScale({it->cutOff, scale, it->cutOff});

            const m::vec3 rot = it->direction;
            const m::quat rx(m::toRadian(rot.x), m::vec3::xAxis);
            const m::quat ry(m::toRadian(rot.y), m::vec3::yAxis);
            const m::quat rz(m::toRadian(rot.z), m::vec3::zAxis);
            m::mat4 rotate = (rz * ry * rx).getMatrix();
            p.setRotate(rotate);

            m_bboxMethod.setWVP(p.projection() * p.view() * p.world());
            m_cone.render(false);
        }

        gl::Enable(GL_CULL_FACE);
        gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Particles
    // Bind the depth buffer for soft particles
    gl::ActiveTexture(GL_TEXTURE1);
    gl::BindTexture(GL_TEXTURE_RECTANGLE, m_gBuffer.texture(gBuffer::kDepth));
    for (auto *const it : m_particleSystems) {
        it->update(pl);
        it->render(pl);
    }
    // Don't need depth testing or blending anymore
    gl::DepthMask(GL_TRUE);
    gl::Disable(GL_DEPTH_TEST);
    gl::Disable(GL_BLEND);

    // Render text showing what we're debugging
    const size_t x = neoWidth() / 2;
    const size_t y = neoHeight() - 20;
    switch (r_debug) {
    case kDebugDepth:
        gui::drawText(x, y, gui::kAlignCenter, "Depth", gui::RGBA(255, 0, 0));
        break;
    case kDebugNormal:
        gui::drawText(x, y, gui::kAlignCenter, "Normals", gui::RGBA(255, 0, 0));
        break;
    case kDebugPosition:
        gui::drawText(x, y, gui::kAlignCenter, "Position", gui::RGBA(255, 0, 0));
        break;
    case kDebugSSAO:
        gui::drawText(x, y, gui::kAlignCenter, "Ambient Occlusion", gui::RGBA(255, 0, 0));
        break;
    default:
        break;
    }

    r::stat::render(20);
}

void World::compositePass(const pipeline &pl) {
    auto &colorGrading = m_grader;
    if (colorGrading.updated()) {
        colorGrading.grade();
        m_colorGrader.update(pl.perspective(), colorGrading.data());
        colorGrading.update();
    } else {
        m_colorGrader.update(pl.perspective(), nullptr);
    }

    // Writing to color grader
    m_colorGrader.bindWriting();

    const GLenum format = gl::has(gl::ARB_texture_rectangle) ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

    // take final composite on unit 0
    gl::ActiveTexture(GL_TEXTURE0);
    gl::BindTexture(format, m_final.texture());

    // take color grading lut on unit 1
    gl::ActiveTexture(GL_TEXTURE1);
    gl::BindTexture(GL_TEXTURE_3D, m_colorGrader.texture(grader::kColorGrading));

    // render to color grading buffer
    m_compositeMethod.enable();
    m_compositeMethod.setPerspective(pl.perspective());
    m_quad.render();

    // apply vignette now
    if (r_vignette) {
        m_vignette.bindWriting();

        // take color grading result on unit 0
        gl::ActiveTexture(GL_TEXTURE0);
        gl::BindTexture(format, m_colorGrader.texture(grader::kOutput));

        // render vignette
        m_vignetteMethod.enable();
        m_vignetteMethod.setPerspective(pl.perspective());
        m_vignetteMethod.setProperties(r_vignette_radius,
                                       r_vignette_softness,
                                       r_vignette_opacity);
        m_quad.render();
    }

    if (r_fxaa) {
        // write to aa buffer
        m_aa.bindWriting();

        gl::ActiveTexture(GL_TEXTURE0);
        if (r_vignette) {
            // take vignette result on unit 0
            gl::BindTexture(format, m_vignette.texture());
        } else {
            // take color grading result on unit 0
            gl::BindTexture(format, m_colorGrader.texture(grader::kOutput));
        }

        // render aa buffer
        m_aaMethod.enable();
        m_aaMethod.setPerspective(pl.perspective());
        m_quad.render();

        // write to window buffer
        gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // take the aa result on unit 0
        gl::ActiveTexture(GL_TEXTURE0);
        gl::BindTexture(format, m_aa.texture());

        // render window buffer
        m_defaultMethod.enable();
        m_defaultMethod.setPerspective(pl.perspective());
        m_quad.render();

    } else {
        // write to window buffer
        gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        gl::ActiveTexture(GL_TEXTURE0);
        if (r_vignette) {
            // take vignette result on unit 0
            gl::BindTexture(format, m_vignette.texture());
        } else {
            // take color grading result on unit 0
            gl::BindTexture(format, m_colorGrader.texture(grader::kOutput));
        }

        // render window buffer
        m_defaultMethod.enable();
        m_defaultMethod.setPerspective(pl.perspective());
        m_quad.render();
    }
}

void World::pointLightPass(const pipeline &pl) {
    gl::DepthMask(GL_FALSE);

    for (const auto &pair : m_culledPointLights) {
        const auto &plc = *pair.second;
        if (!plc.visible)
            continue;
        const auto &it = plc.light;
        float scale = it->radius * kLightRadiusTweak;

        pointLightMethod *method = &m_pointLightMethods[0];

        // Only bother if these are casting shadows
        if (it->castShadows) {
            pointLightShadowPass(&plc);

            gl::DepthMask(GL_FALSE);

            method = &m_pointLightMethods[1];

            gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kShadowMap);
            gl::BindTexture(GL_TEXTURE_2D, m_shadowMap.texture());

            method->enable();
            method->setLightWVP(plc.transform);
        } else {
            method->enable();
        }

        method->setPerspective(pl.perspective());
        method->setEyeWorldPos(pl.position());
        method->setInverse((pl.projection() * pl.view()).inverse());

        pipeline p = pl;
        p.setWorld(it->position);
        p.setScale({scale, scale, scale});

        const m::mat4 wvp = p.projection() * p.view() * p.world();
        method->setLight(*it);
        method->setWVP(wvp);

        const m::vec3 dist = it->position - p.position();
        scale += pl.perspective().nearp + 1.0f;
        if (dist*dist >= scale*scale) {
            gl::DepthFunc(GL_LESS);
            gl::CullFace(GL_BACK);
        } else {
            gl::DepthFunc(GL_GEQUAL);
            gl::CullFace(GL_FRONT);
        }
        m_sphere.render();
    }

    gl::DepthMask(GL_TRUE);
    gl::DepthFunc(GL_LESS);
    gl::CullFace(GL_BACK);
}

void World::pointLightShadowPass(const PointLightChunk *const plc) {
    const pointLight *const pl = plc->light;
    gl::DepthMask(GL_TRUE);
    gl::DepthFunc(GL_LEQUAL);

    m_shadowMap.update(r_sm_size*3, r_sm_size*2);
    m_shadowMap.bindWriting();
    gl::Clear(GL_DEPTH_BUFFER_BIT);

    if (r_sm_poly_factor || r_sm_poly_offset) {
        gl::PolygonOffset(r_sm_poly_factor, r_sm_poly_offset);
        gl::Enable(GL_POLYGON_OFFSET_FILL);
    }

    gl::Enable(GL_SCISSOR_TEST);

    m_shadowMapMethod.enable();

    // v = identity
    // for i=0 to 6:
    //  if i < 2: swap(v.a, v.c)
    //  else if i < 4: swap(v.b, v.c)
    //  if even(i): v.c = -v.c
    //  c = (odd(i) ^ (i >= 4)) ? GL_FRONT : GL_BACK
    static const struct {
        m::mat4 view;
        GLenum cullFace;
    } kSideViews[] = {
        { { {  0.0f,  0.0f,  1.0f,  0.0f }, {  0.0f,  1.0f,  0.0f,  0.0f },
            { -1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_BACK },
        { { {  0.0f,  0.0f,  1.0f,  0.0f }, {  0.0f,  1.0f,  0.0f,  0.0f },
            {  1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_FRONT },
        { { {  1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f,  0.0f },
            {  0.0f, -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_BACK },
        { { {  1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f,  0.0f },
            {  0.0f,  1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_FRONT },
        { { {  1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  1.0f,  0.0f,  0.0f },
            {  0.0f,  0.0f, -1.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_FRONT },
        { { {  1.0f,  0.0f,  0.0f,  0.0f }, {  0.0f,  1.0f,  0.0f,  0.0f },
            {  0.0f,  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  0.0f,  1.0f } }, GL_BACK }
    };

    gl::BindVertexArray(vao);
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, plc->ebo);
    const float borderScale = float(r_sm_size - r_sm_border) / r_sm_size;
    size_t offset = 0;
    for (size_t side = 0; side < 6; ++side) {
        if (plc->sideCounts[side] <= 0)
            continue;

        const auto &view = kSideViews[side];
        m_shadowMapMethod.setWVP(m::mat4::scale({borderScale, borderScale, 1.0f}) *
                                 m::mat4::project(90.0f, 1.0f / pl->radius, m::sqrt(3.0f)) *
                                 view.view *
                                 m::mat4::scale(1.0f /  pl->radius) *
                                 m::mat4::translate(-pl->position));

        const size_t x = r_sm_size * (side / 2);
        const size_t y = r_sm_size * (side % 2);
        gl::Viewport(x, y, r_sm_size, r_sm_size);
        gl::Scissor(x, y, r_sm_size, r_sm_size);
        gl::CullFace(view.cullFace);
        gl::DrawElements(GL_TRIANGLES, plc->sideCounts[side], GL_UNSIGNED_INT,
            (const GLvoid *)(sizeof(GLuint)*offset));

        offset += plc->sideCounts[side];
    }
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    gl::Viewport(0, 0, neoWidth(), neoHeight());

    gl::Disable(GL_SCISSOR_TEST);

    if (r_sm_poly_factor || r_sm_poly_offset)
        gl::Disable(GL_POLYGON_OFFSET_FILL);

    m_final.bindWriting();
}

void World::spotLightPass(const pipeline &pl) {
    gl::DepthMask(GL_FALSE);

    for (const auto &pair : m_culledSpotLights) {
        const auto &slc = *pair.second;
        if (!slc.visible)
            continue;
        const auto &sl = slc.light;
        float scale = sl->radius * kLightRadiusTweak;

        spotLightMethod *method = &m_spotLightMethods[0];

        // Only bother if these are casting shadows
        if (sl->castShadows) {
            spotLightShadowPass(&slc);

            gl::DepthMask(GL_FALSE);

            method = &m_spotLightMethods[1];

            gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kShadowMap);
            gl::BindTexture(GL_TEXTURE_2D, m_shadowMap.texture());

            method->enable();
            method->setLightWVP(slc.transform);
        } else {
            method->enable();
        }

        method->setPerspective(pl.perspective());
        method->setEyeWorldPos(pl.position());
        method->setInverse((pl.projection() * pl.view()).inverse());

        pipeline p = pl;
        p.setWorld(sl->position);
        p.setScale({scale, scale, scale});

        const m::mat4 wvp = p.projection() * p.view() * p.world();
        method->setLight(*sl);
        method->setWVP(wvp);

        const m::vec3 dist = sl->position - p.position();
        scale += pl.perspective().nearp + 1.0f;
        if (dist*dist >= scale*scale) {
            gl::DepthFunc(GL_LESS);
            gl::CullFace(GL_BACK);
        } else {
            gl::DepthFunc(GL_GEQUAL);
            gl::CullFace(GL_FRONT);
        }
        m_sphere.render();
    }

    gl::DepthMask(GL_TRUE);
    gl::DepthFunc(GL_LESS);
    gl::CullFace(GL_BACK);
}

void World::spotLightShadowPass(const SpotLightChunk *const slc) {
    const spotLight *const sl = slc->light;
    gl::DepthMask(GL_TRUE);
    gl::DepthFunc(GL_LEQUAL);
    gl::CullFace(GL_BACK);

    if (r_sm_poly_factor || r_sm_poly_offset) {
        gl::PolygonOffset(r_sm_poly_factor, r_sm_poly_offset);
        gl::Enable(GL_POLYGON_OFFSET_FILL);
    }

    gl::Enable(GL_SCISSOR_TEST);
    gl::Scissor(0, 0, r_sm_size, r_sm_size);

    m_shadowMap.update(r_sm_size*3, r_sm_size*2);
    m_shadowMap.bindWriting();
    gl::Clear(GL_DEPTH_BUFFER_BIT);

    const float borderScale = float(r_sm_size - r_sm_border) / r_sm_size;
    m_shadowMapMethod.enable();
    m_shadowMapMethod.setWVP(m::mat4::scale({borderScale, borderScale, 1.0f}) *
                             m::mat4::project(sl->cutOff, 1.0f / sl->radius, m::sqrt(3.0f)) *
                             m::mat4::lookat(sl->direction, m::vec3::yAxis) *
                             m::mat4::scale(1.0f / sl->radius) *
                             m::mat4::translate(-sl->position));

    // Draw the scene from the lights perspective into the shadow map
    gl::Viewport(0, 0, r_sm_size, r_sm_size);
    gl::BindVertexArray(vao);
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, slc->ebo);
    gl::DrawElements(GL_TRIANGLES, slc->count, GL_UNSIGNED_INT, 0);
    gl::Viewport(0, 0, neoWidth(), neoHeight());
    gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    gl::Disable(GL_SCISSOR_TEST);

    if (r_sm_poly_factor || r_sm_poly_offset)
        gl::Disable(GL_POLYGON_OFFSET_FILL);

    m_final.bindWriting();
}

void World::directionalLightPass(const pipeline &pl, bool stencil) {
    GLenum format = gl::has(gl::ARB_texture_rectangle)
        ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

    if ((r_ssao || r_debug == kDebugSSAO) && stencil) {
        gl::ActiveTexture(GL_TEXTURE0 + lightMethod::kOcclusion);
        gl::BindTexture(format, m_ssao.texture(ssao::kBuffer));
    }

    gl::StencilFunc(GL_EQUAL, stencil, 0xFF);

    auto &method = m_directionalLightMethods[lightCalculatePermutation(stencil)];
    method.enable();
    method.setLight(m_directionalLight);
    method.setPerspective(pl.perspective());
    method.setEyeWorldPos(pl.position());
    method.setInverse((pl.projection() * pl.view()).inverse());
    if (r_fog)
        method.setFog(m_fog);
    m_quad.render();
}

}
