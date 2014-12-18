#include <assert.h>

#include "mesh.h"
#include "engine.h"

#include "u_file.h"
#include "u_map.h"

bool mesh::load(const u::string &file) {
    if (!m_obj.load(file))
        return false;

    // Calculate bounding box
    const auto &p = positions();
    for (const auto &it : p)
        bbox.expand(it);
    return true;
}

u::vector<size_t> mesh::indices() const {
    return m_obj.indices();
}

u::vector<m::vec3> mesh::positions() const {
    return m_obj.positions();
}

u::vector<m::vec3> mesh::normals() const {
    return m_obj.normals();
}

u::vector<m::vec3> mesh::coordinates() const {
    return m_obj.coordinates();
}

///! OBJ Model Loading and Rendering
bool obj::load(const u::string &file) {
    u::file fp = fopen(neoGamePath() + file + ".obj", "r");
    if (!fp)
        return false;

    // Processed vertices, normals and coordinates from the OBJ file
    u::vector<m::vec3> vertices;
    u::vector<m::vec3> normals;
    u::vector<m::vec3> coordinates;

    // Unique vertices are stored in a map keyed by face.
    u::map<face, size_t> uniques;

    size_t count = 0;
    size_t group = 0;
    while (auto get = u::getline(fp)) {
        auto &line = *get;
        // Skip whitespace
        while (line.size() && strchr(" \t", line[0]))
            line.pop_front();
        // Skip comments
        if (strchr("#$", line[0]))
            continue;
        // Skip empty lines
        if (line.empty())
            continue;

        // Process the individual lines
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        if (u::sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
            // v float float float
            vertices.push_back({x, y, z * -1.0f});
        } else if (u::sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
            // vn float float float
            normals.push_back({x * -1.0f, y * -1.0f, z});
        } else if (u::sscanf(line, "vt %f %f", &x, &y) == 2) {
            // vt float float
            coordinates.push_back({x, 1.0f - y, 0.0f});
        } else if (line[0] == 'g') {
            group++;
        } else if (line[0] == 'f' && group == 0) { // Only process the first group faces
            u::vector<size_t> v;
            u::vector<size_t> n;
            u::vector<size_t> t;

            // Note: 1 to skip "f"
            auto contents = u::split(line);
            for (size_t i = 1; i < contents.size(); i++) {
                size_t vi = 0;
                size_t ni = 0;
                size_t ti = 0;
                if (u::sscanf(contents[i], "%zu/%zu/%zu", &vi, &ti, &ni) == 3) {
                    v.push_back(vi - 1);
                    t.push_back(ti - 1);
                    n.push_back(ni - 1);
                } else if (u::sscanf(contents[i], "%zu//%zu", &vi, &ni) == 2) {
                    v.push_back(vi - 1);
                    n.push_back(ni - 1);
                } else if (u::sscanf(contents[i], "%zu/%zu", &vi, &ti) == 2) {
                    v.push_back(vi - 1);
                    t.push_back(ti - 1);
                } else if (u::sscanf(contents[i], "%zu", &vi) == 1) {
                    v.push_back(vi - 1);
                }
            }

            // Triangulate the mesh
            for (size_t i = 1; i < v.size() - 1; ++i) {
                auto index = m_indices.size();
                m_indices.resize(index + 3);
                auto triangulate = [&v, &n, &t, &uniques, &count](size_t index, size_t &out) {
                    face triangle;
                    triangle.vertex = v[index];
                    if (n.size()) triangle.normal = n[index];
                    if (t.size()) triangle.coordinate = t[index];
                    // Only insert in the map if it doesn't exist
                    if (uniques.find(triangle) == uniques.end())
                        uniques[triangle] = count++;
                    out = uniques[triangle];
                };
                triangulate(0,     m_indices[index + 0]);
                triangulate(i + 0, m_indices[index + 1]);
                triangulate(i + 1, m_indices[index + 2]);
            }
        }
    }

    // Construct the model, indices are already generated
    m_positions.resize(count);
    m_normals.resize(count);
    m_coordinates.resize(count);
    for (auto &it : uniques) {
        const auto &first = it.first;
        const auto &second = it.second;
        m_positions[second] = vertices[first.vertex];
        if (normals.size())
            m_normals[second] = normals[first.normal];
        if (coordinates.size())
            m_coordinates[second] = coordinates[first.coordinate];
    }

    // Optimize the indices
    vertexCacheOptimizer vco;
    vco.optimize(m_indices);

    // Change winding order
    for (size_t i = 0; i < m_indices.size(); i += 3)
        u::swap(m_indices[i], m_indices[i + 2]);

    return true;
}

u::vector<size_t> obj::indices() const {
    return m_indices;
}

u::vector<m::vec3> obj::positions() const {
    return m_positions;
}

u::vector<m::vec3> obj::normals() const {
    return m_normals;
}

u::vector<m::vec3> obj::coordinates() const {
    return m_coordinates;
}

///! vertexCacheData
size_t vertexCacheData::findTriangle(size_t triangle) {
    for (size_t i = 0; i < indices.size(); i++)
        if (indices[i] == triangle)
            return i;
    return (size_t)-1;
}

void vertexCacheData::moveTriangle(size_t triangle) {
    size_t index = findTriangle(triangle);
    assert(index != (size_t)-1);

    // Erase the index and add it to the end
    indices.erase(indices.begin() + index,
                  indices.begin() + index + 1);
    indices.push_back(triangle);
}

vertexCacheData::vertexCacheData()
    : cachePosition((size_t)-1)
    , currentScore(0.0f)
    , totalValence(0)
    , remainingValence(0)
    , calculated(false)
{
}

///!triangleCacheData
triangleCacheData::triangleCacheData()
    : rendered(false)
    , currentScore(0.0f)
    , calculated(false)
{
    vertices[0] = (size_t)-1;
    vertices[1] = (size_t)-1;
    vertices[2] = (size_t)-1;
}

///! vertexCache
size_t vertexCache::findVertex(size_t vertex) {
    for (size_t i = 0; i < 32; i++)
        if (m_cache[i] == vertex)
            return i;
    return (size_t)-1;
}

void vertexCache::removeVertex(size_t stackIndex) {
    for (size_t i = stackIndex; i < 38; i++)
        m_cache[i] = m_cache[i + 1];
}

void vertexCache::addVertex(size_t vertex) {
    size_t w = findVertex(vertex);
    // remove the vertex for later reinsertion at the top
    if (w != (size_t)-1)
        removeVertex(w);
    else // not found, cache miss!
        m_misses++;
    // shift all vertices down to make room for the new top vertex
    for (size_t i = 39; i != 0; i--)
        m_cache[i] = m_cache[i - 1];
    // add new vertex to cache
    m_cache[0] = vertex;
}

void vertexCache::clear() {
    for (size_t i = 0; i < 40; i++)
        m_cache[i] = (size_t)-1;
    m_misses = 0;
}

size_t vertexCache::getCacheMissCount() const {
    return m_misses;
}

size_t vertexCache::getCacheMissCount(const u::vector<size_t> &indices) {
    clear();
    for (auto &it : indices)
        addVertex(it);
    return m_misses;
}

size_t vertexCache::getCachedVertex(size_t index) const {
    return m_cache[index];
}

vertexCache::vertexCache() {
    clear();
}

///! vertexCacheOptimizer
static constexpr float kCacheDecayPower = 1.5f;
static constexpr float kLastTriScore = 0.75f;
static constexpr float kValenceBoostScale = 2.0f;
static constexpr float kValenceBoostPower = 0.5f;

vertexCacheOptimizer::vertexCacheOptimizer()
    : m_bestTriangle(0)
{
}

vertexCacheOptimizer::result vertexCacheOptimizer::optimize(u::vector<size_t> &indices) {
    size_t find = (size_t)-1;
    for (size_t i = 0; i < indices.size(); i++)
        if (find == (size_t)-1 || (find != (size_t)-1 && indices[i] > find))
            find = indices[i];
    if (find == (size_t)-1)
        return kErrorNoVertices;

    result begin = init(indices, find + 1);
    if (begin != kSuccess)
        return begin;

    // Process
    while (process())
        ;

    // Rewrite the indices
    for (size_t i = 0; i < m_drawList.size(); i++)
        for (size_t j = 0; j < 3; j++)
            indices[3 * i + j] = m_triangles[m_drawList[i]].vertices[j];

    return kSuccess;
}

float vertexCacheOptimizer::calcVertexScore(size_t vertex) {
    vertexCacheData *v = &m_vertices[vertex];
    if (v->remainingValence == (size_t)-1 || v->remainingValence == 0)
        return -1.0f; // No triangle needs this vertex

    float value = 0.0f;
    if (v->cachePosition == (size_t)-1) {
        // Vertex is not in FIFO cache.
    } else {
        if (v->cachePosition < 3) {
            // This vertex was used in the last triangle. It has fixed score
            // in whichever of the tree it's in.
            value = kLastTriScore;
        } else {
            // Points for being heigh in the cache
            const float scale = 1.0f / (32 - 3);
            value = 1.0f - (v->cachePosition - 3) * scale;
            value = powf(value, kCacheDecayPower);
        }
    }

    // Bonus points for having a low number of triangles.
    float valenceBoost = powf(float(v->remainingValence), -kValenceBoostPower);
    value += kValenceBoostScale * valenceBoost;
    return value;
}

size_t vertexCacheOptimizer::fullScoreRecalculation() {
    // Calculate score for all vertices
    for (size_t i = 0; i < m_vertices.size(); i++)
        m_vertices[i].currentScore = calcVertexScore(i);

    // Calculate scores for all active triangles
    float maxScore = 0.0f;
    size_t maxScoreTriangle = (size_t)-1;
    bool firstTime = true;

    for (size_t i = 0; i < m_triangles.size(); i++) {
        auto &it = m_triangles[i];
        if (it.rendered)
            continue;

        // Sum the score of all the triangle's vertices
        float sum = m_vertices[it.vertices[0]].currentScore +
                    m_vertices[it.vertices[1]].currentScore +
                    m_vertices[it.vertices[2]].currentScore;
        it.currentScore = sum;

        if (firstTime || sum > maxScore) {
            firstTime = false;
            maxScore = sum;
            maxScoreTriangle = i;
        }
    }

    return maxScoreTriangle;
}

vertexCacheOptimizer::result vertexCacheOptimizer::initialPass() {
    for (size_t i = 0; i < m_indices.size(); i++) {
        size_t index = m_indices[i];
        if (index == (size_t)-1 || index >= m_vertices.size())
            return kErrorInvalidIndex;
        m_vertices[index].totalValence++;
        m_vertices[index].remainingValence++;
        m_vertices[index].indices.push_back(i / 3);
    }
    m_bestTriangle = fullScoreRecalculation();
    return kSuccess;
}

vertexCacheOptimizer::result vertexCacheOptimizer::init(u::vector<size_t> &indices, size_t maxVertex) {
    const size_t triangleCount = indices.size() / 3;

    // Reset draw list
    m_drawList.clear();
    m_drawList.reserve(maxVertex);

    // Reset and initialize vertices and triangles
    m_vertices.clear();
    m_vertices.reserve(maxVertex);
    for (size_t i = 0; i < maxVertex; i++)
        m_vertices.push_back(vertexCacheData());

    m_triangles.clear();
    m_triangles.reserve(triangleCount);
    for (size_t i = 0; i < triangleCount; i++) {
        triangleCacheData data;
        for (size_t j = 0; j < 3; j++)
            data.vertices[j] = indices[i * 3 + j];
        m_triangles.push_back(data);
    }

    // Copy the indices
    m_indices.clear();
    m_indices.reserve(indices.size());
    for (auto &it : indices)
        m_indices.push_back(it);

    // Run the initial pass
    m_vertexCache.clear();
    m_bestTriangle = (size_t)-1;

    return initialPass();
}

void vertexCacheOptimizer::addTriangle(size_t triangle) {
    // reset all cache positions
    for (size_t i = 0; i < 32; i++) {
        size_t find = m_vertexCache.getCachedVertex(i);
        if (find == (size_t)-1)
            continue;
        m_vertices[find].cachePosition = (size_t)-1;
    }

    triangleCacheData *t = &m_triangles[triangle];
    if (t->rendered)
        return;

    for (size_t i = 0; i < 3; i++) {
        // Add all the triangle's vertices to the cache
        m_vertexCache.addVertex(t->vertices[i]);
        vertexCacheData *v = &m_vertices[t->vertices[i]];

        // Decrease the remaining valence.
        v->remainingValence--;

        // Move the added triangle to the end of the vertex's triangle index
        // list such that the first `remainingValence' triangles in the index
        // list are only the active ones.
        v->moveTriangle(triangle);
    }

    // It's been rendered, mark it
    m_drawList.push_back(triangle);
    t->rendered = true;

    // Update all the vertex cache positions
    for (size_t i = 0; i < 32; i++) {
        size_t index = m_vertexCache.getCachedVertex(i);
        if (index == (size_t)-1)
            continue;
        m_vertices[index].cachePosition = i;
    }
}

// Avoid duplicate calculations during processing. Triangles and vertices have
// a `calculated' flag which must be reset at the beginning of the process for
// all active triangles that have one or more of their vertices currently in
// cache as well all their other vertices.
//
// If there aren't any active triangles in the cache this function returns
// false and a full recalculation of the tree is performed.
bool vertexCacheOptimizer::cleanFlags() {
    bool found = false;
    for (size_t i = 0; i < 32; i++) {
        size_t find = m_vertexCache.getCachedVertex(i);
        if (find == (size_t)-1)
            continue;

        vertexCacheData *v = &m_vertices[find];
        for (size_t j = 0; j < v->remainingValence; j++) {
            triangleCacheData *t = &m_triangles[v->indices[j]];
            found = true;
            // Clear flags
            t->calculated = false;
            for (size_t k = 0; k < 3; k++)
                m_vertices[t->vertices[k]].calculated = false;
        }
    }
    return found;
}

void vertexCacheOptimizer::triangleScoreRecalculation(size_t triangle) {
    triangleCacheData *t = &m_triangles[triangle];

    // Calculate vertex scores
    float sum = 0.0f;
    for (size_t i = 0; i < 3; i++) {
        vertexCacheData *v = &m_vertices[t->vertices[i]];
        float score = v->calculated ? v->currentScore : calcVertexScore(t->vertices[i]);
        v->currentScore = score;
        v->calculated = true;
        sum += score;
    }

    t->currentScore = sum;
    t->calculated = true;
}

size_t vertexCacheOptimizer::partialScoreRecalculation() {
    // Iterate through all the vertices of the cache
    bool firstTime = true;
    float maxScore = 0.0f;
    size_t maxScoreTriangle = (size_t)-1;
    for (size_t i = 0; i < 32; i++) {
        size_t find = m_vertexCache.getCachedVertex(i);
        if (find == (size_t)-1)
            continue;

        vertexCacheData *v = &m_vertices[find];

        // Iterate through all the active triangles of this vertex
        for (size_t j = 0; j < v->remainingValence; j++) {
            size_t triangle = v->indices[j];
            triangleCacheData *t = &m_triangles[triangle];

            // Calculate triangle score if it isn't already calculated
            if (!t->calculated)
                triangleScoreRecalculation(triangle);

            float score = t->currentScore;
            // Found a triangle to process
            if (firstTime || score > maxScore) {
                firstTime = false;
                maxScore = score;
                maxScoreTriangle = triangle;
            }
        }
    }
    return maxScoreTriangle;
}

inline bool vertexCacheOptimizer::process() {
    if (m_drawList.size() == m_triangles.size())
        return false;

    // Add the selected triangle to the draw list
    addTriangle(m_bestTriangle);

    // Recalculate the vertex and triangle scores and select the best triangle
    // for the next iteration.
    m_bestTriangle = cleanFlags() ? partialScoreRecalculation() : fullScoreRecalculation();

    return true;
}

size_t vertexCacheOptimizer::getCacheMissCount() const {
    return m_vertexCache.getCacheMissCount();
}
