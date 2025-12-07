// Minimal header-only fmt-style formatting for ToyFrameV
// Supports "{}" positional replacement only; ignores format specs.
// This is intentionally tiny to avoid external dependencies.
#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace fmt {

namespace detail {

inline void AppendLiteral(std::string& out, std::string_view literal) {
    out.append(literal.data(), literal.size());
}

inline void ReplaceNextPlaceholder(std::string& out, std::string_view fmt, size_t& pos) {
    auto open = fmt.find('{', pos);
    if (open == std::string_view::npos) {
        AppendLiteral(out, fmt.substr(pos));
        pos = fmt.size();
        return;
    }
    AppendLiteral(out, fmt.substr(pos, open - pos));

    // Look for closing brace
    if (open + 1 < fmt.size() && fmt[open + 1] == '{') {
        // Escaped "{{"
        out.push_back('{');
        pos = open + 2;
        return;
    }

    auto close = fmt.find('}', open + 1);
    if (close == std::string_view::npos) {
        throw std::runtime_error("fmt: unmatched '{'");
    }

    // Only "{}" supported; ignore contents between braces
    pos = close + 1;
}

template <typename T>
void FormatOne(std::string& out, T&& value) {
    std::ostringstream oss;
    oss << std::forward<T>(value);
    out += oss.str();
}

inline void FormatImpl(std::string& out, std::string_view fmt, size_t& pos) {
    // Append remaining literal
    AppendLiteral(out, fmt.substr(pos));
    pos = fmt.size();
}

template <typename T, typename... Rest>
void FormatImpl(std::string& out, std::string_view fmt, size_t& pos, T&& value, Rest&&... rest) {
    ReplaceNextPlaceholder(out, fmt, pos);
    FormatOne(out, std::forward<T>(value));
    FormatImpl(out, fmt, pos, std::forward<Rest>(rest)...);
}

}  // namespace detail

template <typename... Args>
std::string format(std::string_view fmtStr, Args&&... args) {
    std::string result;
    result.reserve(fmtStr.size() + 32);
    size_t pos = 0;
    detail::FormatImpl(result, fmtStr, pos, std::forward<Args>(args)...);

    // If there are still unmatched placeholders, detect them
    if (pos < fmtStr.size()) {
        // Scan for any unmatched '}'
        auto extraClose = fmtStr.find('}', pos);
        if (extraClose != std::string_view::npos) {
            throw std::runtime_error("fmt: unmatched '}'");
        }
    }
    return result;
}

}  // namespace fmt
