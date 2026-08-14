#include "Mesh.h"

#include <charconv>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace terrain {

namespace {

struct Vec3f {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

float parseFloat(std::string_view s) {
    float v = 0.0f;
    std::from_chars(s.data(), s.data() + s.size(), v);
    return v;
}

// Parses "v", "v/vt", "v//vn", "v/vt/vn" → {posIdx, normIdx} (1-based,
// negative = relative). normIdx is 0 when absent.
std::pair<long, long> parseFaceVertex(std::string_view tok) {
    long vi = 0;
    long ni = 0;
    const auto first = tok.find('/');
    const std::string_view vPart = tok.substr(0, first);
    std::from_chars(vPart.data(), vPart.data() + vPart.size(), vi);
    if (first != std::string_view::npos) {
        const auto second = tok.find('/', first + 1);
        if (second != std::string_view::npos) {
            const std::string_view nPart = tok.substr(second + 1);
            if (!nPart.empty())
                std::from_chars(nPart.data(), nPart.data() + nPart.size(), ni);
        }
    }
    return {vi, ni};
}

long resolveIndex(long idx, std::size_t count) {
    return idx > 0 ? idx - 1 : static_cast<long>(count) + idx; // OBJ: -1 = last
}

} // namespace

std::expected<Mesh, std::string> loadObj(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        return std::unexpected("loadObj: cannot open " + path);

    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;

    Mesh mesh;
    mesh.name = path.substr(path.find_last_of("/\\") + 1);

    // Dedupe (pos,normal) pairs → final vertex index.
    std::unordered_map<std::uint64_t, std::uint32_t> dedup;

    std::string line;
    std::size_t lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            std::string x, y, z;
            ss >> x >> y >> z;
            positions.push_back({parseFloat(x), parseFloat(y), parseFloat(z)});
        } else if (tag == "vn") {
            std::string x, y, z;
            ss >> x >> y >> z;
            normals.push_back({parseFloat(x), parseFloat(y), parseFloat(z)});
        } else if (tag == "f") {
            std::vector<std::uint32_t> face;
            std::string tok;
            while (ss >> tok) {
                const auto [vi, ni] = parseFaceVertex(tok);
                const long rvi = resolveIndex(vi, positions.size());
                const long rni = ni != 0 ? resolveIndex(ni, normals.size()) : -1;
                if (rvi < 0 || rvi >= static_cast<long>(positions.size()))
                    return std::unexpected("loadObj: bad vertex index at line " +
                                           std::to_string(lineNo) + " in " + path);
                const std::uint64_t key =
                    (static_cast<std::uint64_t>(static_cast<std::uint32_t>(rvi)) << 32) |
                    static_cast<std::uint32_t>(rni);
                auto [it, inserted] =
                    dedup.try_emplace(key, static_cast<std::uint32_t>(mesh.vertices.size()));
                if (inserted) {
                    MeshVertex mv;
                    const Vec3f& p = positions[static_cast<std::size_t>(rvi)];
                    mv.px = p.x;
                    mv.py = p.y;
                    mv.pz = p.z;
                    if (rni >= 0 && rni < static_cast<long>(normals.size())) {
                        const Vec3f& n = normals[static_cast<std::size_t>(rni)];
                        mv.nx = n.x;
                        mv.ny = n.y;
                        mv.nz = n.z;
                    }
                    mesh.vertices.push_back(mv);
                }
                face.push_back(it->second);
            }
            if (face.size() < 3)
                return std::unexpected("loadObj: degenerate face at line " +
                                       std::to_string(lineNo) + " in " + path);
            // Fan triangulation.
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                mesh.indices.push_back(face[0]);
                mesh.indices.push_back(face[i]);
                mesh.indices.push_back(face[i + 1]);
            }
        }
        // o, g, s, mtllib, usemtl, vt: ignored on purpose.
    }

    if (mesh.vertices.empty() || mesh.indices.empty())
        return std::unexpected("loadObj: no geometry in " + path);

    // AABB.
    for (int a = 0; a < 3; ++a) {
        mesh.aabbMin[a] = 1e30f;
        mesh.aabbMax[a] = -1e30f;
    }
    for (const auto& v : mesh.vertices) {
        const float p[3] = {v.px, v.py, v.pz};
        for (int a = 0; a < 3; ++a) {
            mesh.aabbMin[a] = std::fmin(mesh.aabbMin[a], p[a]);
            mesh.aabbMax[a] = std::fmax(mesh.aabbMax[a], p[a]);
        }
    }
    return mesh;
}

} // namespace terrain
