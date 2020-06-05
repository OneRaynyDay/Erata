#pragma once

#include "record.hpp"

// Require writer concept
#include "writer_concept.hpp"

// Using spdlog to write to files
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

// Using fmt to format record structs into json
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/chrono.h"

// Using json to stringify ending maps
#include "nlohmann/json.hpp"

// Using filesystem to create directories for log files
#include <filesystem>

// Need to only create directory as main thread
#include <sys/types.h>

namespace ert {
namespace writer {

class file_logger {
    // Logger type is single-threaded for performance. We copy a new
    // single-threaded logger upon creating a new thread.
    using logger_type = std::shared_ptr<spdlog::logger>;
    using json = nlohmann::json;

    logger_type logger;
public:
    file_logger() = default;
    file_logger(const file_logger&) = default;
    file_logger& operator=(const file_logger&) = default;

    std::string get_tid() {
        // thread id cannot be converted to an int easily, so we just hack around it by getting the string repr
        // (since it supports <<)
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return ss.str();
    }

    /// Passes in a tag (so far only denoting whether this is an alloc/dealloc type writer to construct internals.
    /// NOTE: This is technically our constructor. This is bad practice to put a setup() function outside
    /// of the constructor but we want a unified interface
    void setup(const std::string& tag) {
        auto this_id = get_tid();
        std::string dir_path = fmt::format("{}/{}", "erata", tag);

        // If the thread is the main thread, we do this.
        // We haven't yet ran into a situation where gettid() does not exist yet.
        // https://stackoverflow.com/questions/20530218/check-if-current-thread-is-main-thread
        if (gettid() == getpid()) {
            // TODO: don't hardcode this
            if (std::filesystem::exists(dir_path))
                throw std::runtime_error(
                        fmt::format("Cannot create folder {} because the path already exists.", dir_path));
            std::filesystem::create_directories(dir_path);
        }

        // TODO We can use std::filesystem here partially
        std::string file_name = fmt::format("{}/{}.json", dir_path, this_id);
        std::string logger_name = fmt::format("{}_logger_{}", tag, this_id);

        logger = spdlog::basic_logger_st(logger_name, file_name);
        logger->set_pattern("%v");
        logger->info("{ \"values\": [");
    }
    void write(const record& record) const {
        logger->info(record);
    }
    void end(const timestamp_type& ts, const scope_map& scope_names, const type_map& type_names) const {
        json scope_json(scope_names);
        json type_json(type_names);
        std::string scope_dump = scope_json.dump();
        std::string type_dump = type_json.dump();
        logger->info("], ");
        logger->info("\"scopes\": {},", scope_dump);
        logger->info("\"types\": {},", type_dump);
        logger->info("\"start_ts\": {},", ts);
        logger->info("}");
        logger->flush();
    }
};

// TODO testing
namespace {
    template<typename T>
    requires is_writer<T>
    void test_concepts(T t) {
        // do nothing;
    }
}

} // namespace writer
} // namespace ert

// Populating fmt with necessary parsers for records
namespace fmt {
    template<>
    struct formatter<::ert::record> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(const ::ert::record &number, FormatContext &ctx) {
            return format_to(ctx.out(),
                             "{{ \"ts\":{0}, \"sh\": {1}, \"th\": {2}, \"s\": {3}, \"l\": {4} }},",
                             number.timestamp, number.scope_hash, number.type_hash, number.size, number.location);
        }
    };

} // namespace fmt
