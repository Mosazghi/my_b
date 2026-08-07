#include "common.hpp"
#include <fmt/base.h>
#include <openssl/evp.h>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "utils/utils.hpp"
using namespace my_b;
namespace my_b::common {

static void replace_entities(std::string& str) {
  str = std::regex_replace(str, std::regex("&lt;"), "<");
  str = std::regex_replace(str, std::regex("&gt;"), ">");
  str = std::regex_replace(str, std::regex("&amp;"), "&");
  str = std::regex_replace(str, std::regex("&quot;"), "\"");
  str = std::regex_replace(str, std::regex("&apos;"), "'");
  str = std::regex_replace(str, std::regex("&nbsp;"), " ");
}

#ifdef DEBUG
void print_token_tree(const std::vector<layout::Token>& tokens) {
  using namespace layout;

  // Tags that never get a matching closing tag (self-closing/void elements)
  static const std::unordered_set<std::string> void_tags = {
      "!doctype", "meta", "br", "br/", "img", "hr", "link", "input"};

  int depth = 0;

  auto indent = [&]() {
    for (int i = 0; i < depth; ++i) std::cout << "  ";
  };

  for (const auto& token : tokens) {
    std::visit(
        [&](const auto& t) {
          using T = std::decay_t<decltype(t)>;

          if constexpr (std::is_same_v<T, Text>) {
            indent();
            std::cout << "\"" << t.text << "\"\n";

          } else if constexpr (std::is_same_v<T, Tag>) {
            const bool is_closing = !t.tag.empty() && t.tag[0] == '/';
            const bool is_self_closing =
                !t.rest.empty() && t.rest.back() == '/';

            if (is_closing) {
              depth = std::max(0, depth - 1);
              indent();
              std::cout << "</" << t.tag.substr(1) << ">\n";
            } else {
              indent();
              std::cout << "<" << t.tag << ">"
                        << "> (parent=" << t.parent_tag << ")"
                        << (t.rest.empty() ? "" : "  attrs=\"" + t.rest + "\"")
                        << "\n";

              const bool is_void =
                  void_tags.count(t.tag) > 0 || is_self_closing;
              if (!is_void) {
                ++depth;
              }
            }
          }
        },
        token);
  }
}

#endif

std::vector<layout::Token> lex(std::string& body) {
  bool in_tag{};
  std::string buffer{};
  std::vector<layout::Token> result{};

  using namespace layout;

  replace_entities(body);

  std::vector<std::string> tag_stack{};
  static const std::unordered_set<std::string> void_tags = {
      "!doctype", "meta", "br", "br/", "img", "hr", "link", "input"};

  const auto current_parent = [&]() -> std::string {
    if (tag_stack.empty()) {
      return "";
    }
    return tag_stack.back();
  };

  for (const auto& c : body) {
    if (c == '<') {
      in_tag = true;
      if (!buffer.empty()) {
        // fmt::println("{} ", buffer);
        // utils::trim(buffer);
        result.emplace_back(Text(buffer));
      }
      buffer.clear();
    } else if (c == '>') {
      in_tag = false;
      std::string rest = "";
      const auto first_space = buffer.find(' ');
      if (first_space != std::string::npos) {
        rest = buffer.substr(first_space, buffer.size() - 1);
      }
      if (first_space != std::string::npos) {
        buffer = buffer.substr(0, first_space);
      } else {
        buffer = std::regex_replace(buffer, std::regex("\\s+"), "");
      }
      const bool is_closing = !buffer.empty() && buffer[0] == '/';
      const bool is_doctype = !buffer.empty() && buffer[0] == '!';
      const bool is_self_closing = !rest.empty() && rest.back() == '/';
      const bool is_void =
          void_tags.count(buffer) > 0 || is_self_closing || is_doctype;

      // if tag is closing itself, remove the opening first before adding
      // *correct* parent
      const std::string parent = current_parent();
      if (!buffer.empty() && buffer.length() > 1 &&
          parent == buffer.substr(1)) {
        tag_stack.pop_back();
      }

      result.emplace_back(Tag(buffer, rest, parent));
      if (!is_closing && !is_void) {
        tag_stack.push_back(buffer);
      }
      buffer.clear();
    } else {
      buffer += c;
    }
  }
  if (!in_tag && !buffer.empty()) {
    // utils::trim(buffer);
    result.emplace_back(Text(buffer));
  }

  // #ifdef DEBUG
  //   print_token_tree(result);
  // #endif
  return result;
}

bool isEmoji(std::uint32_t codepoint) {
  auto c = static_cast<UChar32>(codepoint);
  return u_hasBinaryProperty(c, UCHAR_EMOJI_PRESENTATION);
}

std::string get_emoji_id(std::uint32_t codepoint) {
  std::stringstream id_stream;
  id_stream << std::hex << std::uppercase << std::setfill('0') << std::setw(5)
            << static_cast<uint32_t>(codepoint) << std::dec;
  return id_stream.str();
}

}  // namespace my_b::common
