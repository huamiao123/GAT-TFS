#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <csrbin>\n", argv[0]);
        return 1;
    }
    
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
    
    printf("矩阵: %s\n", argv[1]);
    printf("nrow=%lu, ncol=%lu, nnz=%lu\n\n", nrow, ncol, nnz);
    
    std::vector<uint32_t> indptr(nrow + 1);
    fread(indptr.data(), 4, nrow + 1, fp);
    
    std::vector<uint32_t> indices(nnz);
    fread(indices.data(), 4, nnz, fp);
    fclose(fp);
    
    // === 分析 1: 列索引全局分布 (64 桶) ===
    int num_buckets = 64;
    uint64_t bucket_size = (ncol + num_buckets - 1) / num_buckets;
    std::vector<uint64_t> bucket_count(num_buckets, 0);
    
    for (uint64_t i = 0; i < nnz; i++) {
        int b = indices[i] / bucket_size;
        if (b >= num_buckets) b = num_buckets - 1;
        bucket_count[b]++;
    }
    
    printf("=== 列索引全局分布 (64 桶) ===\n");
    uint64_t max_bucket = *std::max_element(bucket_count.begin(), bucket_count.end());
    uint64_t min_bucket = *std::min_element(bucket_count.begin(), bucket_count.end());
    double avg_bucket = (double)nnz / num_buckets;
    printf("每桶平均 NNZ: %.0f\n", avg_bucket);
    printf("最大桶 NNZ: %lu (%.1f%%)\n", max_bucket, 100.0 * max_bucket / nnz);
    printf("最小桶 NNZ: %lu (%.1f%%)\n", min_bucket, 100.0 * min_bucket / nnz);
    printf("不均衡度 (max/avg): %.2f\n\n", max_bucket / avg_bucket);
    
    printf("桶分布 (每桶覆盖 %lu 列):\n", bucket_size);
    for (int b = 0; b < num_buckets; b++) {
        int bar_len = (int)(40.0 * bucket_count[b] / max_bucket);
        printf("  [%8lu-%8lu]: %8lu ", 
               (uint64_t)b * bucket_size,
               std::min((uint64_t)(b+1) * bucket_size - 1, ncol - 1),
               bucket_count[b]);
        for (int j = 0; j < bar_len; j++) printf("#");
        printf("\n");
    }
    
    // === 分析 2: 相邻行列索引重叠度 ===
    printf("\n=== 相邻行列索引重叠度 (抽样) ===\n");
    
    int sample_count = 0;
    double total_jaccard = 0;
    int total_overlap = 0;
    
    std::vector<uint64_t> nonempty;
    for (uint64_t i = 0; i < nrow; i++) {
        if (indptr[i] < indptr[i+1]) nonempty.push_back(i);
    }
    
    int step = std::max((int)(nonempty.size() / 1000), 1);
    for (int idx = 0; idx + 1 < (int)nonempty.size() && sample_count < 1000; idx += step) {
        uint64_t r1 = nonempty[idx];
        uint64_t r2 = nonempty[idx + 1];
        
        std::vector<uint32_t> cols1(indices.begin() + indptr[r1], 
                                      indices.begin() + indptr[r1+1]);
        std::vector<uint32_t> cols2(indices.begin() + indptr[r2], 
                                      indices.begin() + indptr[r2+1]);
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
    
    printf("抽样 %d 对相邻非空行\n", sample_count);
    printf("平均 Jaccard 相似度: %.4f\n", total_jaccard / sample_count);
    printf("有重叠的比例: %.1f%%\n", 100.0 * total_overlap / sample_count);
    
    // === 分析 3: 每行列索引 span ===
    printf("\n=== 每行列索引 span 分布 ===\n");
    
    std::vector<double> spans;
    for (uint64_t i = 0; i < nrow; i++) {
        if (indptr[i] >= indptr[i+1]) continue;
        uint32_t min_col = indices[indptr[i]];
        uint32_t max_col = indices[indptr[i+1] - 1];
        spans.push_back((double)(max_col - min_col + 1));
    }
    
    std::sort(spans.begin(), spans.end());
    int n = spans.size();
    printf("非空行数: %d\n", n);
    printf("span 中位数: %.0f (占 ncol 的 %.2f%%)\n", 
           spans[n/2], 100.0 * spans[n/2] / ncol);
    printf("span P10: %.0f\n", spans[n/10]);
    printf("span P90: %.0f\n", spans[n*9/10]);
    printf("span P99: %.0f\n", spans[n*99/100]);
    printf("span 最大: %.0f (占 ncol 的 %.2f%%)\n", 
           spans[n-1], 100.0 * spans[n-1] / ncol);
    printf("span < 1%%*ncol 的行比例: ");
    double threshold = 0.01 * ncol;
    int count_small = std::lower_bound(spans.begin(), spans.end(), threshold) - spans.begin();
    printf("%.1f%%\n", 100.0 * count_small / n);
    
    return 0;
}
