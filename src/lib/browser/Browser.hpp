#pragma once
#include <SFML/Graphics.hpp>
#include <common/common.hpp>
#include <functional>
#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../ui/Scrollbar.hpp"
#include "../ui/UiManager.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Window/Event.hpp"
#include "http/HttpRequest.hpp"
#include "logger.hpp"
#include "resource-loader/ResourceLoader.hpp"
#include "url/Url.hpp"
namespace my_b::browser {
using EventCallback = std::function<void(const sf::Event&)>;
class Browser {
 public:
  explicit Browser(sf::RenderWindow& window);
  void load(const url::URL& url);
  ~Browser();
  void spin();
  void draw();

 private:
  void relayout_for_current_window_width();
  void register_event_handlers();
  void register_callback(sf::Event::EventType event, const EventCallback& cb);
  void register_callback(std::initializer_list<sf::Event::EventType> events,
                         const EventCallback& cb);
  void dispatch_event(const sf::Event& event);
  void update_ui_elements();
  /**
   * @brief Perform the request for the URL
   * @return std::optional<http::HttpResponse> HTTP response if successful,
   * std::nullopt otherwise
   */
  http::HttpResult request(const url::URL& url);

  bool m_running{};
  std::unordered_map<sf::Event::EventType, std::vector<EventCallback>>
      m_event_callbacks;
  std::vector<layout::PositionTextPair> m_display_content;
  std::vector<layout::Token> m_text_content;
  sf::Font m_font;
  std::shared_ptr<http::IHttpClient> m_http_client{};
  std::unique_ptr<loader::ResourceLoader> m_loader;
  sf::RenderWindow& m_window;
  ui::UiManager m_ui_manager;
  ui::ScrollBar* m_top_scrollbar{};

  Logger& logger = Logger::getInstance();
};
}  // namespace my_b::browser
