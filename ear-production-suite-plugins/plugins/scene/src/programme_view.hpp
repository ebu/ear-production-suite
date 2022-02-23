#pragma once

#include <memory>

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/ear_tabbed_component.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "helper/iso_lang_codes.hpp"
#include "elements_container.hpp"
#include "element_overview.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ProgrammeView : public Component {
 public:
  explicit ProgrammeView(SceneFrontendBackendConnector* connector)
      : nameTextEditor_(std::make_shared<EarNameTextEditor>()),
        langLabel_(std::make_unique<Label>()),
        langComboBox_(std::make_shared<EarComboBox>()),
        addItemButton_(std::make_unique<EarButton>()),
        addGroupButton_(std::make_unique<EarButton>()),
        addToggleButton_(std::make_unique<EarButton>()),
        elementOverview_(std::make_unique<ElementOverview>(connector)),
        elementsContainer_(std::make_shared<ElementsContainer>()) {
    nameTextEditor_->setLabelText("Name");
    nameTextEditor_->setFont(EarFonts::Label);
    nameTextEditor_->onTextChange = [this]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker, [this](Listener& l) {
        l.nameChanged(this, this->nameTextEditor_->getText());
      });
      if (checker.shouldBailOut()) {
        return;
      }
    };
    addAndMakeVisible(nameTextEditor_.get());

    langLabel_->setFont(EarFonts::Label);
    langLabel_->setText("Language",
                        juce::NotificationType::dontSendNotification);
    langLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(langLabel_.get());

    for (std::size_t i = 0; i < LANGUAGES.size(); ++i) {
      langComboBox_->addTextEntry(
          String::fromUTF8(LANGUAGES[i].english.c_str()));
    }
    langComboBox_->setDefaultText("select language");
    langComboBox_->onValueChange = [this](int index) {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker, [this, index](Listener& l) {
        l.languageChanged(this, index);
      });
      if (checker.shouldBailOut()) {
        return;
      }
    };
    addAndMakeVisible(langComboBox_.get());

    addItemButton_->setButtonText("Item");
    addItemButton_->setJustification(Justification::left);
    addItemButton_->onClick = [this]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker,
                             [this](Listener& l) { l.addItemClicked(this); });
      if (checker.shouldBailOut()) {
        return;
      }
    };

    addItemButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::add_icon_svg, binary_data::add_icon_svgSize)));
    addItemButton_->setIconAlignment(EarButton::IconAlignment::left);
    addAndMakeVisible(addItemButton_.get());

    addGroupButton_->setButtonText("Group");
    addGroupButton_->setJustification(Justification::left);
    addGroupButton_->onClick = [this]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker,
                             [this](Listener& l) { l.addGroupClicked(this); });
      if (checker.shouldBailOut()) {
        return;
      }
    };
    addGroupButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::add_icon_svg, binary_data::add_icon_svgSize)));
    addGroupButton_->setIconAlignment(EarButton::IconAlignment::left);
    addGroupButton_->setEnabled(false);
    addGroupButton_->setAlpha(Emphasis::disabled);
    addAndMakeVisible(addGroupButton_.get());
    addGroupButton_->setVisible(false);

    addToggleButton_->setButtonText("Toggle");
    addToggleButton_->setJustification(Justification::left);
    addToggleButton_->onClick = [this]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker,
                             [this](Listener& l) { l.addToggleClicked(this); });
      if (checker.shouldBailOut()) {
        return;
      }
    };
    addToggleButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::add_icon_svg, binary_data::add_icon_svgSize)));
    addToggleButton_->setIconAlignment(EarButton::IconAlignment::left);
    addToggleButton_->setEnabled(false);
    addToggleButton_->setAlpha(Emphasis::disabled);
    addAndMakeVisible(addToggleButton_.get());
    addToggleButton_->setVisible(false);

    addAndMakeVisible(elementsContainer_.get());
    addAndMakeVisible(elementOverview_.get());
  }

  void paint(Graphics& g) { g.fillAll(EarColours::Area01dp); }

  void resized() {
    auto area = getLocalBounds();

    area.reduce(marginBig_, marginBig_);
    auto metadataArea = area.removeFromTop(43);
    nameTextEditor_->setBounds(metadataArea.removeFromLeft(230));

    metadataArea.removeFromTop(10);
    langLabel_->setBounds(metadataArea.removeFromLeft(100));
    metadataArea.removeFromLeft(marginSmall_);
    langComboBox_->setBounds(metadataArea.removeFromLeft(200));

    area.removeFromTop(marginBig_);
    auto addButtonArea = area.removeFromTop(28);
    addItemButton_->setBounds(addButtonArea.removeFromLeft(90));
    addButtonArea.removeFromLeft(marginSmall_);
    addGroupButton_->setBounds(addButtonArea.removeFromLeft(90));
    addButtonArea.removeFromLeft(marginSmall_);
    addToggleButton_->setBounds(addButtonArea.removeFromLeft(90));

    area.removeFromTop(10);
    elementsContainer_->setBounds(
        area.removeFromLeft(getWidth() / 2).withTrimmedRight(50));
    elementOverview_->setBounds(area.withTrimmedLeft(50));
  }

  std::shared_ptr<EarNameTextEditor> getNameTextEditor() {
    return nameTextEditor_;
  }
  std::shared_ptr<EarComboBox> getLanguageComboBox() { return langComboBox_; }
  std::shared_ptr<ElementsContainer> getElementsContainer() {
    return elementsContainer_;
  }
  std::shared_ptr<ElementOverview> getElementOverview() {
    return elementOverview_;
  }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void addItemClicked(ProgrammeView* view) = 0;
    virtual void addGroupClicked(ProgrammeView* view) = 0;
    virtual void addToggleClicked(ProgrammeView* view) = 0;
    virtual void nameChanged(ProgrammeView* view, const String& newName) = 0;
    virtual void languageChanged(ProgrammeView* view, int index) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  std::shared_ptr<EarNameTextEditor> nameTextEditor_;
  std::unique_ptr<Label> langLabel_;
  std::shared_ptr<EarComboBox> langComboBox_;
  std::unique_ptr<EarButton> addItemButton_;
  std::unique_ptr<EarButton> addGroupButton_;
  std::unique_ptr<EarButton> addToggleButton_;
  std::shared_ptr<ElementsContainer> elementsContainer_;
  std::shared_ptr<ElementOverview> elementOverview_;

  const float marginBig_ = 20.f;
  const float marginSmall_ = 5.f;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgrammeView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
