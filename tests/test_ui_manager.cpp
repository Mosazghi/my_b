#include <gtest/gtest.h>
#include "UiManager.hpp"

using my_b::ui::UiElement;
using my_b::ui::UiManager;

namespace {

class RecordingElement : public UiElement {
 public:
  RecordingElement(int* draw_count, int* event_count)
      : m_draw_count(draw_count), m_event_count(event_count) {}

  std::string get_name() const override { return "RecordingElement"; }

  void handle_event(const sf::Event& /*event*/,
                    sf::RenderWindow& /*window*/) override {
    ++(*m_event_count);
  }

 private:
  void draw(sf::RenderTarget& /*target*/,
            sf::RenderStates /*states*/) const override {
    ++(*m_draw_count);
  }

  int* m_draw_count;
  int* m_event_count;
};

}  // namespace

class UiManagerTest : public ::testing::Test {
 protected:
  sf::RenderWindow window;
  UiManager manager{window};
};

TEST_F(UiManagerTest, DrawInvokesEachCreatedElement) {
  int draw_count = 0;
  int event_count = 0;
  manager.create_element<RecordingElement>(&draw_count, &event_count);

  manager.draw();

  EXPECT_EQ(draw_count, 1);
}

TEST_F(UiManagerTest, HandleEventDispatchesToEachCreatedElement) {
  int draw_count = 0;
  int event_count = 0;
  manager.create_element<RecordingElement>(&draw_count, &event_count);

  sf::Event event{};
  manager.handle_event(event);

  EXPECT_EQ(event_count, 1);
}

TEST_F(UiManagerTest, RemoveElementStopsFurtherDispatch) {
  int draw_count = 0;
  int event_count = 0;
  auto* element =
      manager.create_element<RecordingElement>(&draw_count, &event_count);

  manager.remove_element(element);

  manager.draw();
  sf::Event event{};
  manager.handle_event(event);

  EXPECT_EQ(draw_count, 0);
  EXPECT_EQ(event_count, 0);
}

TEST_F(UiManagerTest, RemoveElementOnlyAffectsTargetElement) {
  int draw_count_a = 0;
  int event_count_a = 0;
  int draw_count_b = 0;
  int event_count_b = 0;
  auto* a =
      manager.create_element<RecordingElement>(&draw_count_a, &event_count_a);
  manager.create_element<RecordingElement>(&draw_count_b, &event_count_b);

  manager.remove_element(a);
  manager.draw();

  EXPECT_EQ(draw_count_a, 0);
  EXPECT_EQ(draw_count_b, 1);
}
