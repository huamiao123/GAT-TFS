#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "用法: %s <input.mtx> <output.csrbin>\n", argv[0]);
        return 1;
    }
    const char* mtx_path = argv[1];
    const char* out_path = argv[2];

    FILE* fp = fopen(mtx_path, "r");
    if (!fp) { fprintf(stderr, "错误: 无法打开 %s\n", mtx_path); return 1; }

    char line[1024];
    bool is_symmetric = false, is_pattern = false, is_integer = false;

    if (!fgets(line, sizeof(line), fp)) { fprintf(stderr, "错误: 文件为空\n"); return 1; }
    if (strstr(line, "symmetric") || strstr(line, "Symmetric")) is_symmetric = true;
    if (strstr(line, "pattern") || strstr(line, "Pattern")) is_pattern = true;
    if (strstr(line, "integer") || strstr(line, "Integer")) is_integer = true;

    printf("[MTX] symmetric=%d, pattern=%d, integer=%d\n", is_symmetric, is_pattern, is_integer);

    while (fgets(line, sizeof(line), fp)) { if (line[0] != '%') break; }

    int64_t nrow, ncol, nnz_file;
    if (sscanf(line, "%ld %ld %ld", &nrow, &ncol, &nnz_file) != 3) {
        fprintf(stderr, "错误: 无法解析维度行: %s\n", line);
        return 1;
    }
    printf("[MTX] nrow=%ld, ncol=%ld, nnz_in_file=%ld\n", nrow, ncol, nnz_file);

    struct Triple { uint32_t row, col; float val; };
    std::vector<Triple> entries;
    entries.reserve(is_symmetric ? nnz_file * 2 : nnz_file);

    for (int64_t i = 0; i < nnz_file; i++) {
        int r, c; float v = 1.0f;
        if (is_pattern) {
            if (fscanf(fp, "%d %d", &r, &c) != 2) { fprintf(stderr, "错误: 第 %ld 行\n", i); return 1; }
        } else if (is_integer) {
            int iv;
            if (fscanf(fp, "%d %d %d", &r, &c, &iv) != 3) { fprintf(stderr, "错误: 第 %ld 行\n", i); return 1; }
            v = (float)iv;
        } else {
            double dv;
            if (fscanf(fp, "%d %d %lf", &r, &c, &dv) != 3) { fprintf(stderr, "错误: 第 %ld 行\n", i); return 1; }
            v = (float)dv;
        }
        r--; c--;
        entries.push_back({(uint32_t)r, (uint32_t)c, v});
        if (is_symmetric && r != c) entries.push_back({(uint32_t)c, (uint32_t)r, v});
    }
    fclose(fp);

    int64_t total_nnz = (int64_t)entries.size();
    printf("[MTX] 展开后 nnz=%ld\n", total_nnz);

    printf("[Sort] 排序中...\n");
    std::sort(entries.begin(), entries.end(), [](const Triple& a, const Triple& b) {
        return a.row < b.row || (a.row == b.row && a.col < b.col);
    });

    int64_t unique_nnz = 0;
    for (int64_t i = 0; i < total_nnz; i++) {
        if (i > 0 && entries[i].row == entries[i-1].row && entries[i].col == entries[i-1].col) {
            entries[unique_nnz - 1].val += entries[i].val;
        } else {
            entries[unique_nnz++] = entries[i];
        }
    }
    if (unique_nnz < total_nnz) printf("[Dedup] 去重: %ld -> %ld\n", total_nnz, unique_nnz);
    total_nnz = unique_nnz;

    printf("[CSR] 构建 CSR...\n");
    std::vector<uint32_t> indptr(nrow + 1, 0);
    std::vector<uint32_t> indices(total_nnz);
    std::vector<float> values(total_nnz);

    for (int64_t i = 0; i < total_nnz; i++) indptr[entries[i].row + 1]++;
    for (int64_t i = 1; i <= nrow; i++) indptr[i] += indptr[i - 1];
    for (int64_t i = 0; i < total_nnz; i++) {
        indices[i] = entries[i].col;
        values[i]  = entries[i].val;
    }

    printf("[Write] 写入 %s ...\n", out_path);
    FILE* out = fopen(out_path, "wb");
    if (!out) { fprintf(stderr, "错误: 无法创建 %s\n", out_path); return 1; }

    uint32_t ptype = 0, dtype = 0, vtype = 2;
    fwrite(&ptype, 4, 1, out); fwrite(&dtype, 4, 1, out); fwrite(&vtype, 4, 1, out);
    fwrite(&nrow, 8, 1, out); fwrite(&ncol, 8, 1, out); fwrite(&total_nnz, 8, 1, out);
    fwrite(indptr.data(), 4, nrow + 1, out);
    fwrite(indices.data(), 4, total_nnz, out);
    fwrite(values.data(), 4, total_nnz, out);
    fclose(out);

    double size_mb = (36.0 + 4.0*(nrow+1) + 8.0*total_nnz) / (1024*1024);
    printf("[Done] nrow=%ld, ncol=%ld, nnz=%ld, 文件≈%.1f MB\n", nrow, ncol, total_nnz, size_mb);
    return 0;
}
