#pragma once

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cpuid.h>
#include <dirent.h>
#include <iterator>
#include <linux/perf_event.h>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

// Diagnostic only. These are generic userspace hardware events, not DRAM-byte
// counters or a complete accounting of misses at every cache level. A sum of
// cycles over threads is CPU work and must never be called wall-clock cycles.
// Each thread has its own group and its own multiplexing correction. The
// sequential enable/disable skew is explicitly reported, especially for small
// graphs. The measured region also includes this helper's switching overhead.
namespace gcn_extra_projection_pmu {

constexpr size_t STANDARD_EVENT_COUNT = 4;
constexpr size_t EVENT_COUNT = 5;
enum Event : size_t { CYCLES, INSTRUCTIONS, CACHE_MISSES, CACHE_REFERENCES, AMX_BUSY };
inline constexpr const char* event_name[EVENT_COUNT] = {
    "cycles", "instructions", "cache_misses", "cache_references", "amx_busy"
};
inline constexpr uint64_t event_config[EVENT_COUNT] = {
    PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS,
    PERF_COUNT_HW_CACHE_MISSES, PERF_COUNT_HW_CACHE_REFERENCES, 0x02b7
};

// Intel's official SPR core event definition identifies EXE.AMX_BUSY as
// EventCode 0xb7, UMask 0x02 (raw config 0x02b7), speculative arithmetic-busy
// cycles. It is not a TDP instruction count, FLOP count or wall-time fraction.
// https://raw.githubusercontent.com/intel/perfmon/main/SPR/events/sapphirerapids_core.json
inline bool sapphire_rapids_cpu(unsigned& family, unsigned& model) {
    unsigned a = 0, b = 0, c = 0, d = 0;
    family = model = 0;
    const bool intel_vendor = __get_cpuid(0, &a, &b, &c, &d) &&
        b == 0x756e6547 && d == 0x49656e69 && c == 0x6c65746e;
    if(!__get_cpuid(1, &a, &b, &c, &d)) return false;
    const unsigned base_family = (a >> 8) & 0x0f;
    family = base_family;
    if(base_family == 0x0f) family += (a >> 20) & 0xff;
    model = (a >> 4) & 0x0f;
    if(base_family == 0x06 || base_family == 0x0f) model |= ((a >> 16) & 0x0f) << 4;
    return intel_vendor && family == 6 && model == 143;
}

inline double monotonic_seconds() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

inline std::vector<pid_t> task_snapshot(std::string& error, int& error_number) {
    std::vector<pid_t> tids;
    DIR* dir = opendir("/proc/self/task");
    if(dir == nullptr) {
        error_number = errno;
        error = std::string("opendir /proc/self/task: ") + std::strerror(error_number);
        return tids;
    }
    for(;;) {
        errno = 0;
        dirent* entry = readdir(dir);
        if(entry == nullptr) {
            if(errno != 0) {
                error_number = errno;
                error = std::string("readdir /proc/self/task: ") + std::strerror(error_number);
            }
            break;
        }
        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(entry->d_name, &end, 10);
        if(errno == 0 && end != entry->d_name && *end == '\0' && value > 0 && value <= INT_MAX)
            tids.push_back(static_cast<pid_t>(value));
    }
    closedir(dir);
    std::sort(tids.begin(), tids.end());
    tids.erase(std::unique(tids.begin(), tids.end()), tids.end());
    return tids;
}

struct Thread {
    pid_t tid = 0;
    bool available = false;          // Group enabled, disabled and read correctly.
    bool present_at_stop = false;
    bool scaled_valid = false;       // No division by zero or unknown scaling.
    bool zero_running_time = false;
    int errno_code = 0;
    std::string error;
    uint64_t time_enabled = 0;       // Nanoseconds returned by perf, per group.
    uint64_t time_running = 0;
    long double scale = 0;
    long double running_fraction = 0;
    std::array<uint64_t, EVENT_COUNT> raw{};
    std::array<long double, EVENT_COUNT> scaled{};
};

struct Result {
    bool available = false;          // All initial groups collected successfully.
    bool coverage_complete = false;  // No failed or newly observed thread groups.
    bool scaled_complete = false;
    int errno_code = 0;
    std::string error;
    std::vector<Thread> threads;
    std::vector<pid_t> initial_tids;
    std::vector<pid_t> final_tids;
    std::vector<pid_t> new_tids;
    std::vector<pid_t> disappeared_tids;
    uint64_t counted_threads = 0;
    uint64_t failed_threads = 0;
    uint64_t scaled_threads = 0;
    uint64_t zero_running_threads = 0;
    unsigned cpu_family = 0;
    unsigned cpu_model = 0;
    size_t event_count = STANDARD_EVENT_COUNT;
    bool amx_busy_requested = false;
    std::array<uint64_t, EVENT_COUNT> sums{};
    // Each valid thread is scaled individually before summing. A thread with
    // positive enabled time but zero running time is omitted, never divided
    // by zero; scaled_complete=false marks such sums as incomplete.
    std::array<long double, EVENT_COUNT> scaled_sums{};
    double region_begin_s = 0;
    double region_end_s = 0;
    double region_wall_ms = 0;
    double start_enable_span_ms = 0;
    double stop_disable_span_ms = 0;
    double start_control_ms = 0;      // Reset plus enable all groups.
    double stop_control_ms = 0;       // Disable, snapshot, read and bookkeeping.
};

class Region {
    struct Group {
        Thread thread;
        std::array<int, EVENT_COUNT> fd{};
        std::array<uint64_t, EVENT_COUNT> id{};
        bool enabled = false;

        Group() { fd.fill(-1); }
        Group(const Group&) = delete;
        Group& operator=(const Group&) = delete;
        Group(Group&& other) noexcept :
            thread(std::move(other.thread)), fd(other.fd), id(other.id), enabled(other.enabled) {
            other.fd.fill(-1);
            other.enabled = false;
        }
        Group& operator=(Group&& other) noexcept {
            if(this != &other) {
                close_all();
                thread = std::move(other.thread);
                fd = other.fd;
                id = other.id;
                enabled = other.enabled;
                other.fd.fill(-1);
                other.enabled = false;
            }
            return *this;
        }
        ~Group() { close_all(); }

        void close_all() noexcept {
            if(enabled && fd[0] >= 0)
                ioctl(fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
            enabled = false;
            for(int& handle : fd) {
                if(handle >= 0) close(handle);
                handle = -1;
            }
        }
        void fail(const char* action, int code) {
            if(thread.error.empty()) {
                thread.errno_code = code;
                thread.error = std::string(action) + ": " + std::strerror(code);
            }
            thread.available = false;
        }
    };

    std::vector<Group> groups_;
    Result result_;
    bool started_ = false;
    size_t active_events_ = STANDARD_EVENT_COUNT;

    void open_group(Group& group) {
        for(size_t e = 0; e < active_events_; e++) {
            perf_event_attr attr{};
            attr.type = e == AMX_BUSY ? PERF_TYPE_RAW : PERF_TYPE_HARDWARE;
            attr.size = sizeof(attr);
            attr.config = event_config[e];
            attr.disabled = 1;
            attr.exclude_kernel = 1;
            attr.exclude_hv = 1;
            attr.inherit = 0;
            attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_TOTAL_TIME_ENABLED |
                               PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_ID;
            const int leader = e == 0 ? -1 : group.fd[0];
            const long handle = syscall(SYS_perf_event_open, &attr, group.thread.tid,
                                        -1, leader, PERF_FLAG_FD_CLOEXEC);
            if(handle < 0) {
                const int code = errno;
                const std::string action = std::string("perf_event_open ") + event_name[e];
                group.fail(action.c_str(), code);
                group.close_all();
                return;
            }
            group.fd[e] = static_cast<int>(handle);
            if(ioctl(group.fd[e], PERF_EVENT_IOC_ID, &group.id[e]) != 0) {
                const int code = errno;
                group.fail("PERF_EVENT_IOC_ID", code);
                group.close_all();
                return;
            }
        }
    }

    void read_group(Group& group) {
        // perf's group layout with PERF_FORMAT_ID: nr, enabled, running,
        // followed by active_events_ {value,id} pairs. Map IDs; never assume read order.
        std::array<uint64_t, 3 + EVENT_COUNT * 2> words{};
        ssize_t bytes;
        do { bytes = read(group.fd[0], words.data(), sizeof(words)); }
        while(bytes < 0 && errno == EINTR);
        if(bytes < 0) {
            group.fail("read perf group", errno);
            return;
        }
        const size_t expected_bytes = (3 + active_events_ * 2) * sizeof(uint64_t);
        if(bytes != static_cast<ssize_t>(expected_bytes) || words[0] != active_events_) {
            group.fail("invalid perf group read size/count", EIO);
            return;
        }
        group.thread.time_enabled = words[1];
        group.thread.time_running = words[2];
        if(words[2] > words[1]) {
            group.fail("perf time_running exceeds time_enabled", EIO);
            return;
        }
        std::array<bool, EVENT_COUNT> seen{};
        for(size_t v = 0; v < active_events_; v++) {
            const uint64_t value = words[3 + v * 2];
            const uint64_t id = words[4 + v * 2];
            size_t e = 0;
            while(e < active_events_ && group.id[e] != id) e++;
            if(e == active_events_ || seen[e]) {
                group.fail("unknown/duplicate perf event ID", EIO);
                return;
            }
            seen[e] = true;
            group.thread.raw[e] = value;
        }

        group.thread.available = group.thread.error.empty();
        group.thread.zero_running_time = words[2] == 0;
        if(words[2] > 0) {
            group.thread.scale = static_cast<long double>(words[1]) / words[2];
            group.thread.running_fraction = static_cast<long double>(words[2]) / words[1];
            group.thread.scaled_valid = true;
            for(size_t e = 0; e < EVENT_COUNT; e++)
                group.thread.scaled[e] = group.thread.raw[e] * group.thread.scale;
        } else if(words[1] == 0 &&
                  std::all_of(group.thread.raw.begin(), group.thread.raw.end(),
                              [](uint64_t v) { return v == 0; })) {
            // A warmed worker may remain dormant for this whole region.
            group.thread.scale = 1;
            group.thread.scaled_valid = true;
        }
        // enabled>0/running=0 is deliberately not assigned a fabricated scale.
        // It remains a successful raw read, with scaled_valid=false.
    }

public:
    Region() = default;
    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;
    ~Region() = default;             // Group destructors disable and close.

    // Call after warming all execution teams. This snapshots existing TIDs;
    // transient workers born and gone entirely inside the region cannot be
    // discovered and are a documented limitation of non-inheriting groups.
    bool start() {
        if(started_) stop();
        groups_.clear();
        result_ = Result{};
        result_.amx_busy_requested = sapphire_rapids_cpu(result_.cpu_family, result_.cpu_model);
        active_events_ = result_.amx_busy_requested ? EVENT_COUNT : STANDARD_EVENT_COUNT;
        result_.event_count = active_events_;
        int snapshot_errno = 0;
        result_.initial_tids = task_snapshot(result_.error, snapshot_errno);
        result_.errno_code = snapshot_errno;
        groups_.reserve(result_.initial_tids.size());
        for(pid_t tid : result_.initial_tids) {
            Group group;
            group.thread.tid = tid;
            open_group(group);
            groups_.push_back(std::move(group));
        }

        const double control_begin = monotonic_seconds();
        // Reset every group before enabling any group.
        for(Group& group : groups_) {
            if(group.fd[0] < 0) continue;
            if(ioctl(group.fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP) != 0) {
                const int code = errno;
                group.fail("PERF_EVENT_IOC_RESET", code);
                group.close_all();
            }
        }
        const double enable_begin = monotonic_seconds();
        size_t enabled_count = 0;
        for(Group& group : groups_) {
            if(group.fd[0] < 0) continue;
            if(ioctl(group.fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP) != 0) {
                const int code = errno;
                group.fail("PERF_EVENT_IOC_ENABLE", code);
                group.close_all();
            } else {
                group.enabled = true;
                enabled_count++;
            }
        }
        const double enable_end = monotonic_seconds();
        result_.start_enable_span_ms = (enable_end - enable_begin) * 1000;
        result_.start_control_ms = (enable_end - control_begin) * 1000;
        result_.region_begin_s = enable_end;
        started_ = true;
        return enabled_count != 0;
    }

    Result stop() {
        if(!started_) {
            Result result;
            result.error = "PMU region was not started";
            result.errno_code = EINVAL;
            return result;
        }
        const double disable_begin = monotonic_seconds();
        result_.region_end_s = disable_begin;
        result_.region_wall_ms = (disable_begin - result_.region_begin_s) * 1000;
        for(Group& group : groups_) {
            if(!group.enabled) continue;
            if(ioctl(group.fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP) != 0)
                group.fail("PERF_EVENT_IOC_DISABLE", errno);
            else group.enabled = false;
        }
        const double disable_end = monotonic_seconds();
        result_.stop_disable_span_ms = (disable_end - disable_begin) * 1000;

        std::string snapshot_error;
        int snapshot_errno = 0;
        result_.final_tids = task_snapshot(snapshot_error, snapshot_errno);
        if(!snapshot_error.empty()) {
            if(result_.error.empty()) {
                result_.error = snapshot_error;
                result_.errno_code = snapshot_errno;
            }
        }
        std::set_difference(result_.final_tids.begin(), result_.final_tids.end(),
                            result_.initial_tids.begin(), result_.initial_tids.end(),
                            std::back_inserter(result_.new_tids));
        std::set_difference(result_.initial_tids.begin(), result_.initial_tids.end(),
                            result_.final_tids.begin(), result_.final_tids.end(),
                            std::back_inserter(result_.disappeared_tids));

        for(Group& group : groups_) {
            group.thread.present_at_stop = std::binary_search(
                result_.final_tids.begin(), result_.final_tids.end(), group.thread.tid);
            if(group.fd[0] >= 0) read_group(group);
            if(group.thread.available) {
                result_.counted_threads++;
                result_.zero_running_threads += group.thread.zero_running_time;
                result_.scaled_threads += group.thread.scaled_valid;
                for(size_t e = 0; e < EVENT_COUNT; e++) {
                    result_.sums[e] += group.thread.raw[e];
                    if(group.thread.scaled_valid) result_.scaled_sums[e] += group.thread.scaled[e];
                }
            } else {
                result_.failed_threads++;
                if(result_.error.empty()) {
                    result_.error = group.thread.error;
                    result_.errno_code = group.thread.errno_code;
                }
            }
            result_.threads.push_back(group.thread);
            group.close_all();
        }
        result_.available = result_.counted_threads != 0 &&
                            result_.failed_threads == 0 && result_.error.empty();
        result_.coverage_complete = result_.available && result_.new_tids.empty() &&
                                    snapshot_error.empty();
        result_.scaled_complete = result_.available &&
                                  result_.scaled_threads == result_.counted_threads;
        result_.stop_control_ms = (monotonic_seconds() - disable_begin) * 1000;
        started_ = false;
        return result_;
    }
};

inline std::string quoted(const std::string& value) {
    std::string out = "\"";
    for(unsigned char c : value) {
        if(c == '\\' || c == '"') { out += '\\'; out += static_cast<char>(c); }
        else if(c == '\n') out += "\\n";
        else if(c == '\r') out += "\\r";
        else if(c == '\t') out += "\\t";
        else if(c < 32) out += '?';
        else out += static_cast<char>(c);
    }
    out += '"';
    return out;
}

inline std::string tid_list(const std::vector<pid_t>& tids) {
    if(tids.empty()) return "none";
    std::ostringstream out;
    for(size_t i = 0; i < tids.size(); i++) {
        if(i != 0) out << ',';
        out << tids[i];
    }
    return out.str();
}

inline void print(const Result& result, const std::string& graph,
                  const std::string& method, int repeat, FILE* out = stdout) {
    const std::string g = quoted(graph), m = quoted(method), error = quoted(result.error);
    std::fprintf(out,
        "METHOD_PMU graph=%s method=%s repeat=%d available=%d coverage_complete=%d "
        "scaled_complete=%d initial_threads=%zu final_threads=%zu counted_threads=%llu "
        "failed_threads=%llu scaled_threads=%llu zero_running_threads=%llu "
        "new_threads=%zu disappeared_threads=%zu errno=%d error=%s "
        "region_begin_s=%.9f region_end_s=%.9f region_wall_ms=%.6f "
        "start_enable_span_ms=%.6f stop_disable_span_ms=%.6f "
        "start_control_ms=%.6f stop_control_ms=%.6f "
        "cpu_family=%u cpu_model=%u event_count=%zu amx_busy_requested=%d "
        "amx_busy_type=raw amx_busy_config=0x02b7 "
        "amx_busy_semantics=speculative_arithmetic_busy_cycles "
        "event_scope=generic_userspace_hardware cycles_scope=thread_sum "
        "coverage_definition=initial_final_snapshots transient_thread_coverage=unverified",
        g.c_str(), m.c_str(), repeat, result.available, result.coverage_complete,
        result.scaled_complete, result.initial_tids.size(), result.final_tids.size(),
        static_cast<unsigned long long>(result.counted_threads),
        static_cast<unsigned long long>(result.failed_threads),
        static_cast<unsigned long long>(result.scaled_threads),
        static_cast<unsigned long long>(result.zero_running_threads),
        result.new_tids.size(), result.disappeared_tids.size(), result.errno_code, error.c_str(),
        result.region_begin_s, result.region_end_s, result.region_wall_ms,
        result.start_enable_span_ms, result.stop_disable_span_ms,
        result.start_control_ms, result.stop_control_ms,
        result.cpu_family, result.cpu_model, result.event_count, result.amx_busy_requested);
    for(size_t e = 0; e < result.event_count; e++) {
        std::fprintf(out, " %s_raw=%llu %s_scaled=%.6Lf", event_name[e],
            static_cast<unsigned long long>(result.sums[e]), event_name[e], result.scaled_sums[e]);
    }
    std::fprintf(out, " new_tids=%s disappeared_tids=%s\n",
                 tid_list(result.new_tids).c_str(), tid_list(result.disappeared_tids).c_str());

    for(const Thread& thread : result.threads) {
        const std::string te = quoted(thread.error);
        std::fprintf(out,
            "METHOD_PMU_THREAD graph=%s method=%s repeat=%d tid=%ld available=%d "
            "present_at_stop=%d scaled_valid=%d zero_running_time=%d errno=%d error=%s "
            "time_enabled_ns=%llu time_running_ns=%llu scale=%.9Lf running_fraction=%.9Lf",
            g.c_str(), m.c_str(), repeat, static_cast<long>(thread.tid), thread.available,
            thread.present_at_stop, thread.scaled_valid, thread.zero_running_time,
            thread.errno_code, te.c_str(),
            static_cast<unsigned long long>(thread.time_enabled),
            static_cast<unsigned long long>(thread.time_running),
            thread.scale, thread.running_fraction);
        for(size_t e = 0; e < result.event_count; e++) {
            std::fprintf(out, " %s_raw=%llu %s_scaled=%.6Lf", event_name[e],
                static_cast<unsigned long long>(thread.raw[e]), event_name[e], thread.scaled[e]);
        }
        std::fputc('\n', out);
    }
}

} // namespace gcn_extra_projection_pmu
