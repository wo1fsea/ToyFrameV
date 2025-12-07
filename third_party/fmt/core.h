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

// Append literal text, handling }} escape sequences
inline void AppendLiteral(std::string& out, std::string_view literal) {
    size_t i = 0;
    while (i < literal.size()) {
        auto pos = literal.find("}}", i);
        if (pos == std::string_view::npos) {
            out.append(literal.data() + i, literal.size() - i);
            break;
        }
        out.append(literal.data() + i, pos - i);
        out.push_back('}');  // }} -> }
        i = pos + 2;
    }
}

// Returns true if a real placeholder was found, false if it was an escape or
// end of string
inline bool ReplaceNextPlaceholder(std::string &out, std::string_view fmt,
                                   size_t &pos) {
  while (pos < fmt.size()) {
    auto open = fmt.find('{', pos);
    if (open == std::string_view::npos) {
      AppendLiteral(out, fmt.substr(pos));
      pos = fmt.size();
      return false;
    }
    AppendLiteral(out, fmt.substr(pos, open - pos));

    // Look for closing brace or escape
    if (open + 1 < fmt.size() && fmt[open + 1] == '{') {
      // Escaped "{{" - output single '{' and continue searching
      out.push_back('{');
      pos = open + 2;
      continue;
    }

    auto close = fmt.find('}', open + 1);
    if (close == std::string_view::npos) {
      throw std::runtime_error("fmt: unmatched '{'");
    }

    // Found a real placeholder
    pos = close + 1;
    return true;
  }
  return false;
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
  if (ReplaceNextPlaceholder(out, fmt, pos)) {
    FormatOne(out, std::forward<T>(value));
    FormatImpl(out, fmt, pos, std::forward<Rest>(rest)...);
  } else {
    // No placeholder found but arguments remain - just continue with remaining
    // args
    FormatImpl(out, fmt, pos, std::forward<Rest>(rest)...);
  }
}

}  // namespace detail

template <typename... Args>
std::string format(std::string_view fmtStr, Args&&... args) {
    std::string result;
    result.reserve(fmtStr.size() + 32);
    size_t pos = 0;
    detail::FormatImpl(result, fmtStr, pos, std::forward<Args>(args)...);
    return result;
}

}  // namespace fmt
