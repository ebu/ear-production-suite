#pragma once

#include "JuceHeader.h"

#include "../helper/graphics.hpp"
#include "ear_bounds_constrainer.hpp"
#include "ear_button.hpp"
#include "ear_tab_button_bar.hpp"

#include <memory>
#include <vector>

namespace ear {
namespace plugin {
namespace ui {

struct EarTab {
  EarTabButton* button;
  Component* component;
};

class EarTabbedComponent : public Component {
 public:
  EarTabbedComponent();

  void addTab(const String& name, Component* component, bool select = true,
              bool scroll = true);
  void setTabName(int index, const String& name);
  void selectTab(int index, bool scroll = true);
  int getSelectedTabIndex();
  void moveTabTo(int oldIndex, int newIndex);
  void removeTab(int index);
  void removeAllTabs();

  void resized() override;

  void mouseDown(const MouseEvent& event) override;
  void mouseDrag(const MouseEvent& event) override;
  void mouseUp(const MouseEvent& event) override;

  int getIndexForTabButton(EarTabButton* button);
  int getIndexForComponent(Component* component);
  Component* getComponent(int index);

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void addTabClicked(EarTabbedComponent* tabbedComponent) = 0;
    virtual void tabSelected(EarTabbedComponent* tabbedComponent,
                             int index) = 0;
    virtual void tabMoved(EarTabbedComponent* tabbedComponent, int oldIndex,
                          int newIndex) = 0;
    virtual void removeTabClicked(EarTabbedComponent* tabbedComponent,
                                  int index) = 0;
    virtual void tabBarDoubleClicked(EarTabbedComponent* tabbedComponent) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  int getButtonIndexForXPosition(int xPos);
  void clearSelected();

  int selectedTabIndex_ = -1;

  const int buttonBarHeight = 40;
  juce::Rectangle<int> contentArea_;

  std::unique_ptr<EarTabButtonBar> buttonBar_;
  std::unique_ptr<EarButton> addTabButton_;
  std::vector<EarTab> tabs_;
  std::vector<std::unique_ptr<EarTabButton>> buttons_;

  int buttonWidth_;

  ComponentDragger tabDragger_;
  EarBoundsConstrainer tabDraggerConstrainer_;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabbedComponent)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
