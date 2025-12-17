#pragma once
#include <mc/deps/core/string/HashedString.h>

#include <compare>
#include <string_view>

namespace permc {

class HashedStringView {
    uint64_t         hash_;
    std::string_view str_;

public:
    constexpr HashedStringView() noexcept : hash_(0), str_("") {}
    consteval HashedStringView(const char* str) noexcept : hash_(HashedString::computeHash(str)), str_(str) {}
    consteval HashedStringView(std::string_view str) noexcept : hash_(HashedString::computeHash(str)), str_(str) {}
    HashedStringView(HashedString const& str) noexcept : hash_(str.getHash()), str_(str.getString()) {}

    [[nodiscard]] constexpr uint64_t         getHash() const noexcept { return hash_; }
    [[nodiscard]] constexpr std::string_view getString() const noexcept { return str_; }
    [[nodiscard]] constexpr const char*      c_str() const noexcept { return str_.data(); }

    [[nodiscard]] constexpr bool operator==(HashedStringView const& rhs) const noexcept { return hash_ == rhs.hash_; }

    [[nodiscard]] constexpr std::strong_ordering operator<=>(HashedStringView const& rhs) const noexcept {
        return hash_ <=> rhs.hash_;
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept { return str_; }
};

consteval HashedStringView operator""_hsv(const char* str, std::size_t len) {
    return {std::string_view(str, len)};
}

struct HashedStringHasher {
    using is_transparent = void;

    std::size_t operator()(HashedStringView const& v) const noexcept { return v.getHash(); }

    std::size_t operator()(HashedString const& v) const noexcept { return v.getHash(); }

    std::size_t operator()(std::string_view v) const noexcept { return HashedString::computeHash(v); }

    // const char*
    std::size_t operator()(char const* v) const noexcept { return HashedString::computeHash(v); }
};

struct HashedStringEqual {
    using is_transparent = void;

    bool operator()(HashedStringView const& lhs, HashedString const& rhs) const noexcept {
        return lhs.getHash() == rhs.getHash();
    }
    bool operator()(HashedString const& lhs, HashedStringView const& rhs) const noexcept {
        return lhs.getHash() == rhs.getHash();
    }
    bool operator()(HashedString const& lhs, HashedString const& rhs) const noexcept {
        return lhs.getHash() == rhs.getHash();
    }
    bool operator()(HashedStringView const& lhs, HashedStringView const& rhs) const noexcept {
        return lhs.getHash() == rhs.getHash();
    }

    bool operator()(HashedString const& lhs, std::string_view rhs) const noexcept {
        if (lhs.getHash() != HashedString::computeHash(rhs)) return false;
        return lhs.getString() == rhs;
    }
    bool operator()(std::string_view lhs, HashedString const& rhs) const noexcept {
        if (HashedString::computeHash(lhs) != rhs.getHash()) return false;
        return lhs == rhs.getString();
    }

    // const char*
    bool operator()(HashedString const& lhs, char const* rhs) const noexcept {
        return (*this)(lhs, std::string_view(rhs));
    }
    bool operator()(char const* lhs, HashedString const& rhs) const noexcept {
        return (*this)(std::string_view(lhs), rhs);
    }
};

} // namespace permc

template <>
struct std::hash<permc::HashedStringView> {
    std::size_t operator()(permc::HashedStringView const& v) const noexcept { return v.getHash(); }
};