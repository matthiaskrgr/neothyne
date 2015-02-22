#ifndef WORLD_HDR
#define WORLD_HDR
#include "r_world.h"

struct baseLight {
    baseLight();
    m::vec3 color;
    float ambient;
    union {
        float diffuse;
        float intensity;
    };
    bool highlight;
};

inline baseLight::baseLight()
    : ambient(1.0f)
    , highlight(false)
{
}

// a directional light (local ambiance and diffuse)
struct directionalLight : baseLight {
    m::vec3 direction;
};

// a point light
struct pointLight : baseLight {
    pointLight();
    m::vec3 position;
    float radius;
};

inline pointLight::pointLight()
    : radius(0.0f)
{
}

// a spot light
struct spotLight : pointLight {
    spotLight();
    m::vec3 direction;
    float cutOff;
};

inline spotLight::spotLight()
    : cutOff(45.0f)
{
}

// a map model
struct mapModel {
    mapModel();
    m::vec3 position;
    m::vec3 scale;
    m::vec3 rotate;
    u::string name;
    bool highlight;
    r::occlusionQueries::ref occlusionQuery;
};

inline mapModel::mapModel()
    : highlight(false)
    , occlusionQuery(size_t(-1))
{
}

struct playerStart {
    m::vec3 position;
    m::vec3 direction;
    bool highlight;
};

struct teleport {
    m::vec3 position;
    m::vec3 direction;
    bool highlight;
};

struct jumppad {
    m::vec3 position;
    m::vec3 direction;
    m::vec3 velocity;
    bool highlight;
};

enum class entity {
    kMapModel,
    kPlayerStart,
    kDirectionalLight,
    kPointLight,
    kSpotLight,
    kTeleport,
    kJumppad
};

struct world {
    world();
    ~world();

    bool load(const u::string &map);
    bool upload(const m::perspective &p);
    void render(const r::pipeline &pl);

    void unload(bool destroy = true);
    bool isLoaded() const;

    // World entity descriptor
    struct descriptor {
        entity type;
        size_t index;
        size_t where;
    };

    struct trace {
        struct hit {
            m::vec3 position; // Position of what was hit
            m::vec3 normal; // Normal of what was hit
            descriptor *ent; // The entity hit or null if level geometry
            float fraction; // normalized [0, 1] fraction of distance made before hit
        };
        struct query {
            m::vec3 start;
            m::vec3 direction;
            float radius;
        };
    };

    bool trace(const trace::query &q, trace::hit *result, float maxDistance, bool entities = true, descriptor *ignore = nullptr);

    void insert(const directionalLight &it);
    descriptor *insert(const spotLight &it);
    descriptor *insert(const pointLight &it);
    descriptor *insert(const mapModel &it);
    descriptor *insert(const playerStart &it);
    descriptor *insert(const teleport &it);
    descriptor *insert(const jumppad &it);

    void erase(size_t where); // Erase an entity

    directionalLight &getDirectionalLight();
    spotLight &getSpotLight(size_t index);
    pointLight &getPointLight(size_t index);
    mapModel &getMapModel(size_t index);
    playerStart &getPlayerStart(size_t index);
    teleport &getTeleport(size_t index);
    jumppad &getJumppad(size_t index);

    const u::vector<mapModel*> &getMapModels() const;

protected:
    friend struct r::world;

    static constexpr float kMaxTraceDistance = 99999.9f;

    // Load from compressed data
    bool load(const u::vector<unsigned char> &data);

private:
    kdMap m_map; // The map for this world

    r::world m_renderer;

    struct billboard {
        struct board {
            m::vec3 position;
            bool highlight;
        };
        const char *name;
        float size;
        bool bbox;
        u::vector<board> boards;
        void add(const m::vec3 &position, const m::vec3 &adjust, bool highlight) {
            boards.push_back({ position + adjust, highlight });
        }
    };

    u::vector<descriptor> m_entities;
    u::vector<billboard> m_billboards;

    // The following are populated via insert/erase
    directionalLight *m_directionalLight;
    u::vector<spotLight*> m_spotLights;
    u::vector<pointLight*> m_pointLights;
    u::vector<mapModel*> m_mapModels;
    u::vector<playerStart*> m_playerStarts;
    u::vector<teleport*> m_teleports;
    u::vector<jumppad*> m_jumppads;
};

#endif
