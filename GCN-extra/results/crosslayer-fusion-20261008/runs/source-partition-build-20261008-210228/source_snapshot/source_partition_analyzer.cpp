#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

struct Csr {
    int n = 0;
    uint32_t e = 0;
    std::vector<uint32_t> row, col;
};

static Csr load_csr(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (!f) throw std::runtime_error("cannot open CSR");
    uint8_t hdr[36];
    if (std::fread(hdr, 1, 36, f) != 36) throw std::runtime_error("short CSR header");
    uint64_t nr = 0, nz = 0;
    std::memcpy(&nr, hdr + 12, 8); std::memcpy(&nz, hdr + 28, 8);
    Csr g; g.n = int(nr); g.e = uint32_t(nz);
    g.row.resize(size_t(g.n) + 1); g.col.resize(g.e);
    if (std::fread(g.row.data(), sizeof(uint32_t), g.row.size(), f) != g.row.size()) throw std::runtime_error("short row");
    if (std::fread(g.col.data(), sizeof(uint32_t), g.col.size(), f) != g.col.size()) throw std::runtime_error("short col");
    std::fclose(f); return g;
}

struct RowKey { uint32_t min_src=0, max_src=0, avg_src=0, degree=0; };

static std::vector<int> order_by(const Csr& g, const std::vector<RowKey>& key, const char* kind) {
    std::vector<int> p(g.n); std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(), [&](int a, int b) {
        if (std::string(kind) == "identity") return a < b;
        if (std::string(kind) == "degree") {
            if (key[a].degree != key[b].degree) return key[a].degree < key[b].degree;
            return a < b;
        }
        if (std::string(kind) == "degree_desc") {
            if (key[a].degree != key[b].degree) return key[a].degree > key[b].degree;
            return a < b;
        }
        if (std::string(kind) == "min_source") {
            if (key[a].min_src != key[b].min_src) return key[a].min_src < key[b].min_src;
            return key[a].max_src < key[b].max_src;
        }
        if (std::string(kind) == "avg_source") {
            if (key[a].avg_src != key[b].avg_src) return key[a].avg_src < key[b].avg_src;
            return key[a].min_src < key[b].min_src;
        }
        if (key[a].max_src != key[b].max_src) return key[a].max_src < key[b].max_src;
        return a < b;
    });
    return p;
}

static void evaluate(const Csr& g, const std::vector<int>& p, const char* kind, int parts) {
    std::vector<uint32_t> seen(g.n, 0);
    uint32_t stamp = 0; uint64_t total = 0; uint64_t all_edges = 0; uint64_t max_unique = 0; uint64_t min_unique = UINT64_MAX;
    uint64_t max_edges = 0, min_edges = UINT64_MAX;
    for (int part = 0; part < parts; ++part) {
        ++stamp; uint64_t unique = 0, edges = 0;
        int begin = (g.n * part) / parts, end = (g.n * (part + 1)) / parts;
        for (int pi = begin; pi < end; ++pi) {
            uint32_t row = uint32_t(p[pi]);
            edges += g.row[row + 1] - g.row[row];
            for (uint32_t e = g.row[row]; e < g.row[row + 1]; ++e) {
                uint32_t src = g.col[e];
                if (seen[src] != stamp) { seen[src] = stamp; ++unique; }
            }
        }
        total += unique; all_edges += edges;
        max_unique = std::max(max_unique, unique); min_unique = std::min(min_unique, unique);
        max_edges = std::max(max_edges, edges); min_edges = std::min(min_edges, edges);
    }
    ++stamp; uint64_t global = 0;
    for (uint32_t src : g.col) if (seen[src] != stamp) { seen[src] = stamp; ++global; }
    std::printf("PARTITION order=%s parts=%d n=%d e=%u global_sources=%llu total_unique_sources=%llu duplication=%.9f min_unique=%llu max_unique=%llu min_edges=%llu max_edges=%llu edge_check=%llu\n",
        kind, parts, g.n, g.e, (unsigned long long)global, (unsigned long long)total,
        double(total) / double(std::max<uint64_t>(global, 1)), (unsigned long long)min_unique,
        (unsigned long long)max_unique, (unsigned long long)min_edges, (unsigned long long)max_edges,
        (unsigned long long)all_edges);
}

int main(int argc, char** argv) {
    if (argc != 3) { std::fprintf(stderr, "usage: %s CSR PARTS\n", argv[0]); return 2; }
    Csr g = load_csr(argv[1]); int parts = std::atoi(argv[2]);
    std::vector<RowKey> key(g.n);
    for (int i = 0; i < g.n; ++i) {
        uint32_t a = g.row[i], b = g.row[i + 1]; key[i].degree = b - a;
        if (a == b) continue;
        key[i].min_src = key[i].max_src = g.col[a]; uint64_t sum = 0;
        for (uint32_t e = a; e < b; ++e) { key[i].min_src = std::min(key[i].min_src, g.col[e]); key[i].max_src = std::max(key[i].max_src, g.col[e]); sum += g.col[e]; }
        key[i].avg_src = uint32_t(sum / (b - a));
    }
    for (const char* kind : {"identity", "degree", "degree_desc", "min_source", "avg_source", "max_source"})
        evaluate(g, order_by(g, key, kind), kind, parts);
    return 0;
}
