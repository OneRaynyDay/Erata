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

namespace ert {
namespace writer {

class file_logger {
    // Logger type is single-threaded for performance. We copy a new
    // single-threaded logger upon creating a new thread.
    using logger_type = std::shared_ptr<spdlog::logger>;
    using json = nlohmann::json;

    logger_type logger;
public:
    // This is technically our constructor. This is bad practice to put a setup() function outside
    // of the constructor but we want a unified interface
    void setup(const std::string& logger_name, const std::string& file_name) {
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
