# hack to patch imgui-SFML.cpp to use auto instead of std::basic_string<std::uint8_t>
set(FILE_PATH "${CMAKE_ARGV3}")

file(READ "${FILE_PATH}" FILE_CONTENTS)

string(REPLACE
  "std::basic_string<std::uint8_t> tmp = sf::Clipboard::getString().toUtf8();"
  "auto tmp = sf::Clipboard::getString().toUtf8();"
  FILE_CONTENTS_PATCHED
  "${FILE_CONTENTS}"
)

file(WRITE "${FILE_PATH}" "${FILE_CONTENTS_PATCHED}")
