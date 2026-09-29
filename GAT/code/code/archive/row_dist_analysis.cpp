#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

// csrbin 文件格式（参考 JitSpMM 的 SpMat.h）:
//   uint32 ptype, dtype, vtype
//   uint64 nrow, ncol, nnz
//   indptr[nrow+1], indices[nnz], values[nnz]

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("用法: %s <input.csrbin>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) { perror("无法打开文件"); return 1; }

    // 读 header
    uint32_t ptype, dtype, vtype;
    uint64_t nrow, ncol, nnz;
    fread(&ptype, sizeof(uint32_t), 1, fp);  // indptr 类型 (0=uint32)
    fread(&dtype, sizeof(uint32_t), 1, fp);  // indices 类型
    fread(&vtype, sizeof(uint32_t), 1, fp);  // values 类型
    fread(&nrow,  sizeof(uint64_t), 1, fp);
    fread(&ncol,  sizeof(uint64_t), 1, fp);
    fread(&nnz,   sizeof(uint64_t), 1, fp);

    printf("=== 矩阵基本信息 ===\n");
    printf("行数: %lu, 列数: %lu, 非零元: %lu\n", nrow, ncol, nnz);
    printf("平均每行非零元: %.2f\n", (double)nnz / nrow);
    printf("ptype=%u dtype=%u vtype=%u\n\n", ptype, dtype, vtype);

    // 读 indptr (只需要 indptr 来算每行 nnz，不需要读 indices 和 values)
    std::vector<uint64_t> indptr(nrow + 1);
    if (ptype == 0) {
        // uint32 indptr
        std::vector<uint32_t> indptr32(nrow + 1);
        fread(indptr32.data(), sizeof(uint32_t), nrow + 1, fp);
        for (uint64_t i = 0; i <= nrow; i++) indptr[i] = indptr32[i];
    } else {
        // uint64 indptr
        fread(indptr.data(), sizeof(uint64_t), nrow + 1, fp);
    }
    fclose(fp);

    // 计算每行的非零元数量
    std::vector<uint64_t> row_nnz(nrow);
    for (uint64_t i = 0; i < nrow; i++) {
        row_nnz[i] = indptr[i + 1] - indptr[i];
    }

    // 排序用于统计分位数
    std::vector<uint64_t> sorted_nnz = row_nnz;
    std::sort(sorted_nnz.begin(), sorted_nnz.end());

    // 基本统计
    uint64_t max_nnz = sorted_nnz.back();
    uint64_t min_nnz = sorted_nnz.front();
    double mean = (double)nnz / nrow;
    uint64_t median = sorted_nnz[nrow / 2];

    // 标准差
    double var_sum = 0;
    for (uint64_t i = 0; i < nrow; i++) {
        double diff = (double)row_nnz[i] - mean;
        var_sum += diff * diff;
    }
    double stddev = sqrt(var_sum / nrow);

    // 分位数
    uint64_t p90 = sorted_nnz[(uint64_t)(nrow * 0.90)];
    uint64_t p95 = sorted_nnz[(uint64_t)(nrow * 0.95)];
    uint64_t p99 = sorted_nnz[(uint64_t)(nrow * 0.99)];
    uint64_t p999 = sorted_nnz[(uint64_t)(nrow * 0.999)];

    printf("=== 行非零元分布统计 ===\n");
    printf("最小值:   %lu\n", min_nnz);
    printf("中位数:   %lu\n", median);
    printf("平均值:   %.2f\n", mean);
    printf("最大值:   %lu\n", max_nnz);
    printf("标准差:   %.2f\n", stddev);
    printf("P90:      %lu\n", p90);
    printf("P95:      %lu\n", p95);
    printf("P99:      %lu\n", p99);
    printf("P99.9:    %lu\n\n", p999);

    // 空行统计
    uint64_t empty_rows = 0;
    for (uint64_t i = 0; i < nrow; i++) {
        if (row_nnz[i] == 0) empty_rows++;
    }
    printf("空行数: %lu (%.2f%%)\n\n", empty_rows, 100.0 * empty_rows / nrow);

    // 分桶直方图
    // 桶: 0, 1, 2-5, 6-10, 11-50, 51-100, 101-500, 501-1000, 1001-5000, 5000+
    struct Bucket {
        const char* label;
        uint64_t lo, hi;      // [lo, hi]
        uint64_t count;
        uint64_t total_nnz;
    };
    Bucket buckets[] = {
        {"0",          0,     0,     0, 0},
        {"1",          1,     1,     0, 0},
        {"2-5",        2,     5,     0, 0},
        {"6-10",       6,     10,    0, 0},
        {"11-50",      11,    50,    0, 0},
        {"51-100",     51,    100,   0, 0},
        {"101-500",    101,   500,   0, 0},
        {"501-1000",   501,   1000,  0, 0},
        {"1001-5000",  1001,  5000,  0, 0},
        {"5001+",      5001,  UINT64_MAX, 0, 0},
    };
    int nbuckets = sizeof(buckets) / sizeof(buckets[0]);

    for (uint64_t i = 0; i < nrow; i++) {
        for (int b = 0; b < nbuckets; b++) {
            if (row_nnz[i] >= buckets[b].lo && row_nnz[i] <= buckets[b].hi) {
                buckets[b].count++;
                buckets[b].total_nnz += row_nnz[i];
                break;
            }
        }
    }

    printf("=== 行非零元分布直方图 ===\n");
    printf("%-12s %10s %8s %12s %8s\n", "范围", "行数", "行占比", "NNZ贡献", "NNZ占比");
    printf("---------------------------------------------------------------\n");
    for (int b = 0; b < nbuckets; b++) {
        if (buckets[b].count > 0) {
            printf("%-12s %10lu %7.2f%% %12lu %7.2f%%\n",
                   buckets[b].label,
                   buckets[b].count,
                   100.0 * buckets[b].count / nrow,
                   buckets[b].total_nnz,
                   100.0 * buckets[b].total_nnz / nnz);
        }
    }

    // Top-N 大 V 行（对我们的负载均衡分析最关键）
    printf("\n=== Top-20 大 V 行 ===\n");
    printf("%-10s %10s %10s\n", "行号", "NNZ", "占总NNZ%");
    // 找 top-20
    std::vector<std::pair<uint64_t, uint64_t>> row_pairs(nrow); // (nnz, row_id)
    for (uint64_t i = 0; i < nrow; i++) {
        row_pairs[i] = {row_nnz[i], i};
    }
    std::partial_sort(row_pairs.begin(), row_pairs.begin() + 20, row_pairs.end(),
                      [](auto& a, auto& b) { return a.first > b.first; });
    uint64_t top20_total = 0;
    for (int i = 0; i < 20; i++) {
        top20_total += row_pairs[i].first;
        printf("%-10lu %10lu %9.4f%%\n",
               row_pairs[i].second, row_pairs[i].first,
               100.0 * row_pairs[i].first / nnz);
    }
    printf("Top-20 合计: %lu (%.2f%%)\n", top20_total, 100.0 * top20_total / nnz);

    // 负载均衡分析：模拟 row-split 64 线程
    printf("\n=== 负载均衡分析 (模拟 64 线程 row-split, step=256) ===\n");
    // 简化模拟：将行按顺序分成 64 段（按行数均分）
    int nthreads = 64;
    uint64_t rows_per_thread = (nrow + nthreads - 1) / nthreads;
    uint64_t max_thread_nnz = 0, min_thread_nnz = UINT64_MAX;
    for (int t = 0; t < nthreads; t++) {
        uint64_t start = t * rows_per_thread;
        uint64_t end = std::min(start + rows_per_thread, nrow);
        uint64_t thread_nnz = indptr[end] - indptr[start];
        if (thread_nnz > max_thread_nnz) max_thread_nnz = thread_nnz;
        if (thread_nnz < min_thread_nnz) min_thread_nnz = thread_nnz;
    }
    double ideal = (double)nnz / nthreads;
    printf("理想每线程 NNZ: %.0f\n", ideal);
    printf("实际最大: %lu (%.2fx 理想值)\n", max_thread_nnz, max_thread_nnz / ideal);
    printf("实际最小: %lu (%.2fx 理想值)\n", min_thread_nnz, min_thread_nnz / ideal);
    printf("不均衡比 (max/min): %.2fx\n", (double)max_thread_nnz / min_thread_nnz);

    // 模拟 nnz-split：64 线程按 nnz 均分，统计被切割的行数
    printf("\n=== NNZ-Split 切割分析 (64 线程) ===\n");
    uint64_t seg_len = (nnz + nthreads - 1) / nthreads;
    int split_rows = 0;
    uint64_t split_rows_total_nnz = 0;
    for (int t = 1; t < nthreads; t++) {
        uint64_t boundary = t * seg_len;
        // 二分找 boundary 落在哪一行
        auto it = std::upper_bound(indptr.begin(), indptr.end(), boundary);
        uint64_t row_id = (it - indptr.begin()) - 1;
        // 如果 boundary 不恰好在行边界上，这行就被切割了
        if (indptr[row_id] != boundary && indptr[row_id + 1] != boundary) {
            split_rows++;
            split_rows_total_nnz += row_nnz[row_id];
        }
    }
    printf("被切割的行数: %d / %d 个边界\n", split_rows, nthreads - 1);
    printf("被切割行的总 NNZ: %lu (平均每行 %.1f)\n",
           split_rows_total_nnz,
           split_rows > 0 ? (double)split_rows_total_nnz / split_rows : 0);
    printf("这些行若退化为标量，指令膨胀约 %lu × 17 = %lu 亿条额外指令\n",
           split_rows_total_nnz,
           split_rows_total_nnz * 17 * 32 / 100000000);  // k=32 时的估算

    return 0;
}
