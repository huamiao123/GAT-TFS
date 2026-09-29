#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "用法: %s <输入.csrbin> <输出.csrbin>\n", argv[0]);
        return 1;
    }

    // === 读取 csrbin ===
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) { fprintf(stderr, "无法打开 %s\n", argv[1]); return 1; }

    uint32_t ptype, dtype, vtype;
    uint64_t nrow, ncol, nnz;
    fread(&ptype, 4, 1, fp);
    fread(&dtype, 4, 1, fp);
    fread(&vtype, 4, 1, fp);
    fread(&nrow, 8, 1, fp);
    fread(&ncol, 8, 1, fp);
    fread(&nnz, 8, 1, fp);

    printf("读取: %s\n", argv[1]);
    printf("nrow=%lu, ncol=%lu, nnz=%lu\n", nrow, ncol, nnz);

    std::vector<uint32_t> indptr(nrow + 1);
    fread(indptr.data(), 4, nrow + 1, fp);

    std::vector<uint32_t> indices(nnz);
    fread(indices.data(), 4, nnz, fp);

    std::vector<float> values(nnz);
    fread(values.data(), 4, nnz, fp);
    fclose(fp);

    // === 计算每行的排序键：列索引中位数 ===
    // 空行的键设为 ncol（排到最后面，集中在一起）
    printf("计算每行列索引中位数...\n");

    std::vector<uint64_t> sort_key(nrow);
    uint64_t empty_count = 0;

    for (uint64_t i = 0; i < nrow; i++) {
        uint32_t start = indptr[i];
        uint32_t end = indptr[i + 1];
        if (start == end) {
            sort_key[i] = ncol;  // 空行排到最后
            empty_count++;
        } else {
            // 列索引在 csrbin 中通常已排序，直接取中间位置
            uint32_t mid = start + (end - start) / 2;
            sort_key[i] = indices[mid];
        }
    }
    printf("空行: %lu, 非空行: %lu\n", empty_count, nrow - empty_count);

    // === 按 sort_key 排序，得到新的行顺序 ===
    printf("排序行...\n");

    std::vector<uint64_t> perm(nrow);
    std::iota(perm.begin(), perm.end(), 0);  // perm = [0, 1, 2, ..., nrow-1]

    std::sort(perm.begin(), perm.end(), [&](uint64_t a, uint64_t b) {
        return sort_key[a] < sort_key[b];
    });

    // === 构建重排后的 CSR ===
    printf("构建重排后的 CSR...\n");

    std::vector<uint32_t> new_indptr(nrow + 1);
    std::vector<uint32_t> new_indices(nnz);
    std::vector<float> new_values(nnz);

    new_indptr[0] = 0;
    uint64_t pos = 0;
    for (uint64_t i = 0; i < nrow; i++) {
        uint64_t old_row = perm[i];
        uint32_t start = indptr[old_row];
        uint32_t end = indptr[old_row + 1];
        uint32_t rnnz = end - start;

        memcpy(&new_indices[pos], &indices[start], rnnz * sizeof(uint32_t));
        memcpy(&new_values[pos], &values[start], rnnz * sizeof(float));
        pos += rnnz;
        new_indptr[i + 1] = (uint32_t)pos;
    }

    // === 统计重排效果 ===
    // 计算相邻非空行的 B 区域重叠度
    int sample_count = 0;
    double total_jaccard = 0;
    int total_overlap = 0;

    std::vector<uint64_t> nonempty;
    for (uint64_t i = 0; i < nrow; i++) {
        if (new_indptr[i] < new_indptr[i + 1]) nonempty.push_back(i);
    }

    int step_s = std::max((int)(nonempty.size() / 1000), 1);
    for (int idx = 0; idx + 1 < (int)nonempty.size() && sample_count < 1000; idx += step_s) {
        uint64_t r1 = nonempty[idx];
        uint64_t r2 = nonempty[idx + 1];

        std::vector<uint32_t> cols1(new_indices.begin() + new_indptr[r1],
                                    new_indices.begin() + new_indptr[r1 + 1]);
        std::vector<uint32_t> cols2(new_indices.begin() + new_indptr[r2],
                                    new_indices.begin() + new_indptr[r2 + 1]);
        std::sort(cols1.begin(), cols1.end());
        std::sort(cols2.begin(), cols2.end());

        std::vector<uint32_t> inter;
        std::set_intersection(cols1.begin(), cols1.end(),
                              cols2.begin(), cols2.end(),
                              std::back_inserter(inter));
        std::vector<uint32_t> uni;
        std::set_union(cols1.begin(), cols1.end(),
                       cols2.begin(), cols2.end(),
                       std::back_inserter(uni));

        if (!uni.empty()) {
            double jaccard = (double)inter.size() / uni.size();
            total_jaccard += jaccard;
            if (!inter.empty()) total_overlap++;
        }
        sample_count++;
    }

    printf("\n重排后相邻行重叠度:\n");
    printf("  抽样 %d 对\n", sample_count);
    printf("  平均 Jaccard: %.4f\n", total_jaccard / sample_count);
    printf("  有重叠比例: %.1f%%\n", 100.0 * total_overlap / sample_count);

    // === 写出重排后的 csrbin ===
    printf("\n写出: %s\n", argv[2]);

    FILE* out = fopen(argv[2], "wb");
    if (!out) { fprintf(stderr, "无法创建 %s\n", argv[2]); return 1; }

    fwrite(&ptype, 4, 1, out);
    fwrite(&dtype, 4, 1, out);
    fwrite(&vtype, 4, 1, out);
    fwrite(&nrow, 8, 1, out);
    fwrite(&ncol, 8, 1, out);
    fwrite(&nnz, 8, 1, out);
    fwrite(new_indptr.data(), 4, nrow + 1, out);
    fwrite(new_indices.data(), 4, nnz, out);
    fwrite(new_values.data(), 4, nnz, out);
    fclose(out);

    printf("完成!\n");
    return 0;
}
