#include "PermMapping.hpp"

#include "ll/api/Expected.h"
#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>


#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

namespace permc {

decltype(PermMapping::mapping_) PermMapping::mapping_{};

ll::Expected<> PermMapping::loadUserExtension(std::filesystem::path const& path) {
    auto defaultJson = ll::reflection::serialize<nlohmann::json>(mapping_);
    if (!defaultJson) {
        return ll::makeStringError(defaultJson.error().message());
    }

    if (!std::filesystem::exists(path)) {
        auto dump = defaultJson.value().dump(2);
        if (!ll::file_utils::writeFile(path, dump)) {
            return ll::makeStringError("failed to write " + path.string());
        }
        return {};
    }

    auto rawJson = ll::file_utils::readFile(path);
    if (!rawJson) {
        return ll::makeStringError("failed to read " + path.string());
    }
    try {
        auto json = nlohmann::json::parse(*rawJson);
        defaultJson->merge_patch(json);
        return ll::reflection::deserialize(mapping_, defaultJson.value());
    } catch (nlohmann::json::exception const& exception) {
        return ll::makeStringError(exception.what());
    }
}
void                             PermMapping::clear() { mapping_.clear(); }
optional_ref<const HashedString> PermMapping::lookup(std::string_view typeName) {
    auto it = mapping_.find(typeName);
    if (it != mapping_.end()) {
        return {it->second};
    }
    return nullptr;
}

} // namespace permc