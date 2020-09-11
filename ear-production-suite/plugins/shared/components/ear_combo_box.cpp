#include "ear_combo_box.hpp"

#include "../helper/graphics.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/shadows.hpp"

#include <vector>

namespace ear {
namespace plugin {
namespace ui {

EarComboBoxEntry::EarComboBoxEntry() {
  setColour(backgroundColourId, EarColours::Transparent);
  setColour(selectedColourId, EarColours::ComboBoxPopupBackground);
  setColour(highlightColourId, EarColours::Area04dp);
  setInterceptsMouseClicks(false, false);
}

bool EarComboBoxEntry::isSelectable() const { return isSelectable_; }
bool EarComboBoxEntry::isSelected() const { return selected_; }
void EarComboBoxEntry::setSelected(bool selected) { selected_ = selected; }

bool EarComboBoxEntry::isHighlighted() const { return highlighted_; }
void EarComboBoxEntry::setHighlighted(bool highlighted) {
  highlighted_ = highlighted;
}

void EarComboBoxEntry::paint(Graphics& g) {
  EarComboBoxPopup* popup = findParentComponentOfClass<EarComboBoxPopup>();
  auto transformPosition = getLocalPoint(popup, juce::Point<int>{0, 0});
  auto clipPath = popup->getLocalBoundsPath();
  clipPath.applyTransform(AffineTransform::translation(transformPosition));
  g.reduceClipRegion(clipPath);

  if (isSelected() && !isHighlighted()) {
    g.fillAll(findColour(selectedColourId));
  }
  drawEntryInList(g, getLocalBounds());
  if (isSelectable() && isHighlighted()) {
    g.fillAll(findColour(highlightColourId));
  }
}

EarComboBoxTextEntry::EarComboBoxTextEntry(const String& text) : text_(text) {
  setColour(textColourId, EarColours::ComboBoxText);
  // only necessary to set height -> width will be overwritten
  setSize(300, 40);
  searchString_ = text_;
}

void EarComboBoxTextEntry::setText(const String& text) { text_ = text; }
String EarComboBoxTextEntry::getText() const { return text_; }

void EarComboBoxTextEntry::drawEntryInList(Graphics& g,
                                           juce::Rectangle<int> area) {
  g.setColour(findColour(textColourId));
  g.setFont(EarFonts::Items);
  g.drawText(text_, area.withTrimmedLeft(14).withTrimmedRight(14),
             Justification::left);
}

void EarComboBoxTextEntry::drawEntryInButton(Graphics& g,
                                             juce::Rectangle<int> area) {
  g.setColour(findColour(textColourId));
  g.setFont(EarFonts::Items);
  g.drawText(text_, area.withTrimmedLeft(14).withTrimmedRight(28),
             Justification::left);
}

EarComboBoxSectionEntry::EarComboBoxSectionEntry(const String& text)
    : text_(text) {
  setColour(textColourId, EarColours::ComboBoxText);
  isSelectable_ = false;
  // only necessary to set height -> width will be overwritten
  setSize(300, 32);
}

void EarComboBoxSectionEntry::setText(const String& text) { text_ = text; }
String EarComboBoxSectionEntry::getText() const { return text_; }

void EarComboBoxSectionEntry::drawEntryInList(Graphics& g,
                                              juce::Rectangle<int> area) {
  g.setColour(findColour(textColourId));
  g.setFont(EarFonts::Values);
  g.drawText(text_,
             area.withTrimmedLeft(10).withTrimmedRight(28).withTrimmedBottom(5),
             Justification::bottomLeft);
  g.drawLine(0.f, getHeight() - 0.5f, getWidth(), getHeight() - 0.5f);
}

void EarComboBoxSectionEntry::drawEntryInButton(Graphics& g,
                                                juce::Rectangle<int> area) {
  drawEntryInList(g, area);
}

EarComboBoxColourEntry::EarComboBoxColourEntry(Colour colour)
    : colour_(colour) {
  setColour(backgroundColourId, EarColours::Transparent);
  setColour(highlightColourId, EarColours::Area04dp);
  setSize(100, 33);
}

void EarComboBoxColourEntry::setEntryColour(const Colour& colour) {
  colour_ = colour;
}
Colour EarComboBoxColourEntry::getEntryColour() const { return colour_; }

void EarComboBoxColourEntry::drawEntryInList(Graphics& g,
                                             juce::Rectangle<int> area) {
  g.setColour(colour_);
  fillRoundedRectangle(g, getLocalBounds().toFloat().reduced(6.5, 6.5), 4.f);
}

void EarComboBoxColourEntry::drawEntryInButton(Graphics& g,
                                               juce::Rectangle<int> area) {
  int size = std::min(area.getWidth(), area.getHeight());
  juce::Rectangle<int> circleBounds{0, 0, size, size};
  circleBounds.setCentre(area.withTrimmedRight(22).getCentre());
  circleBounds.reduce(6, 6);
  Path circle;
  circle.addEllipse(circleBounds.toFloat());
  g.setColour(colour_);
  g.fillPath(circle);
  Shadows::elevation04dp.drawForPath(g, circle);
}

EarComboBoxPopup::EarComboBoxPopup(EarComboBox* comboBox)
    : comboBox_(comboBox),
      viewport_(std::make_unique<Viewport>()),
      listView_(std::make_unique<Component>()) {
  setColour(backgroundColourId, EarColours::ComboBoxPopupBackground);
  setColour(textColourId, EarColours::ComboBoxText);
  setColour(outlineColourId, EarColours::ComboBoxText);
  setColour(buttonColourId, EarColours::ComboBoxText);
  setColour(arrowColourId, EarColours::Text.withAlpha(Emphasis::medium));
  setColour(focusedOutlineColourId, EarColours::Primary);

  viewport_->setInterceptsMouseClicks(false, true);
  listView_->setInterceptsMouseClicks(false, false);
  setWantsKeyboardFocus(true);
  setAlwaysOnTop(true);

  viewport_->setViewedComponent(listView_.get(), false);
  viewport_->setScrollBarsShown(true, false);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::backgroundColourId,
                                              EarColours::Transparent);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Area04dp);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::trackColourId,
                                              EarColours::Transparent);
  addAndMakeVisible(viewport_.get());
}

void EarComboBoxPopup::addEntry(std::unique_ptr<EarComboBoxEntry> entry) {
  entries_.push_back(std::move(entry));
}

bool EarComboBoxPopup::selectEntry(int index) {
  if (index < entries_.size()) {
    if (entries_.at(index)->isSelectable()) {
      clearSelection();
      entries_.at(index)->setSelected(true);
      return true;
    }
  }
  return false;
}

void EarComboBoxPopup::clearEntries() {
  hide();
  selectEntry(-1);
  listView_->removeAllChildren();
  entries_.clear();
}

void EarComboBoxPopup::clearSelection() {
  for (std::unique_ptr<EarComboBoxEntry>& entry : entries_) {
    entry->setSelected(false);
  }
}

void EarComboBoxPopup::clearHighlighted() {
  for (std::unique_ptr<EarComboBoxEntry>& entry : entries_) {
    entry->setHighlighted(false);
  }
  repaint();
}

void EarComboBoxPopup::show() {
  if (isVisible()) {
    return;
  }
  parent_ = comboBox_->getTopLevelComponent();
  if (!parent_) {
    return;
  }
  parent_->addChildComponent(this);

  listView_->setSize(comboBox_->getWidth(), getHeightOfAllEntries());
  auto listViewBounds = listView_->getLocalBounds();
  listViewBounds.removeFromTop(padding_);
  for (std::unique_ptr<EarComboBoxEntry>& entry : entries_) {
    listView_->addAndMakeVisible(entry.get());
    entry->setBounds(listViewBounds.removeFromTop(entry->getHeight()));
  }

  auto position = parent_->getLocalPoint(comboBox_, juce::Point<int>{0, 0});
  int popupWidth = comboBox_->getWidth();
  int popupHeight = comboBox_->getHeight() + listView_->getHeight();
  juce::Rectangle<int> popupBounds{position.getX(), position.getY(), popupWidth,
                                   popupHeight};
  parent_->getLocalBounds().reduced(10, 10).intersectRectangle(popupBounds);
  setBounds(popupBounds);

  auto viewportArea_ = getLocalBounds();
  viewportArea_.removeFromTop(comboBox_->getHeight() - 1);
  viewport_->setBounds(viewportArea_);
  setVisible(true);
  grabKeyboardFocus();
}

void EarComboBoxPopup::hide() {
  if (isVisible() && parent_) {
    parent_->removeChildComponent(this);
    parent_ = nullptr;
    setVisible(false);
  }
}

int EarComboBoxPopup::getHeightOfAllEntries() const {
  int ret = 0;
  for (auto& entry : entries_) {
    ret += entry->getHeight();
  }
  ret += 2 * padding_;
  return ret;
}

Path EarComboBoxPopup::getBoundsPath() const {
  auto area = getBounds();
  return createRoundedRectangle(area.getX(), area.getY(), area.getWidth(),
                                area.getHeight(), 4.f);
}

Path EarComboBoxPopup::getLocalBoundsPath() const {
  auto area = getLocalBounds();
  return createRoundedRectangle(area.getX(), area.getY(), area.getWidth(),
                                area.getHeight(), 4.f);
}

void EarComboBoxPopup::mouseUp(const MouseEvent& event) {
  if (comboBox_->contains(event.getPosition())) {
    hide();
    return;
  }
  for (auto& entry : entries_) {
    if (entry->getScreenBounds().contains(event.getScreenPosition())) {
      comboBox_->selectEntry(entry.get(), sendNotificationAsync);
      comboBox_->repaint();
      hide();
      return;
    }
  }
}

bool EarComboBoxPopup::keyPressed(const KeyPress& key) {
  if (key == KeyPress::spaceKey || key == KeyPress::returnKey) {
    for (auto& entry : entries_) {
      if (entry->isHighlighted()) {
        clearHighlighted();
        comboBox_->selectEntry(entry.get(), sendNotificationAsync);
        hide();
        return true;
      }
    }
    return true;
  }
  if (key == KeyPress::escapeKey) {
    clearHighlighted();
    hide();
    return true;
  }
  if (key == KeyPress::deleteKey || key == KeyPress::backspaceKey) {
    clearSelection();
    comboBox_->selectEntry(-1, sendNotificationAsync);
    hide();
    return true;
  }
  if (key == KeyPress::leftKey) {
    int index = -1;
    while (index + 1 < entries_.size()) {
      ++index;
      if (entries_.at(index)->isSelectable()) {
        clearHighlighted();
        entries_.at(index)->setHighlighted(true);
        scrollIfNecessaryTo(getHighlightedEntryIndex());
        repaint();
        return true;
      }
    }
    return true;
  }
  if (key == KeyPress::rightKey) {
    int index = entries_.size();
    if (index == -1) {
      index = entries_.size();
    }
    while (index - 1 >= 0) {
      --index;
      if (entries_.at(index)->isSelectable()) {
        clearHighlighted();
        entries_.at(index)->setHighlighted(true);
        scrollIfNecessaryTo(getHighlightedEntryIndex());
        repaint();
        return true;
      }
    }
    return true;
  }

  if (key == KeyPress::downKey) {
    int index = getHighlightedEntryIndex();
    while (index + 1 < entries_.size()) {
      ++index;
      if (entries_.at(index)->isSelectable()) {
        clearHighlighted();
        entries_.at(index)->setHighlighted(true);
        scrollIfNecessaryTo(getHighlightedEntryIndex());
        repaint();
        return true;
      }
    }
    return true;
  }
  if (key == KeyPress::upKey) {
    int index = getHighlightedEntryIndex();
    if (index == -1) {
      index = entries_.size();
    }
    while (index - 1 >= 0) {
      --index;
      if (entries_.at(index)->isSelectable()) {
        clearHighlighted();
        entries_.at(index)->setHighlighted(true);
        scrollIfNecessaryTo(getHighlightedEntryIndex());
        repaint();
        return true;
      }
    }
    return true;
  }
  auto character = String::charToString(key.getTextCharacter());
  if (String::fromUTF8("ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜabcdefghijklmnopqrstuv"
                       "wxyzäöü1234567890-.")
          .contains(character)) {
    auto currentTime = Time::getCurrentTime();
    if ((currentTime - lastTime_) > RelativeTime(1)) {
      searchString_.clear();
    }
    lastTime_ = currentTime;
    searchString_ += character;
    for (auto& entry : entries_) {
      if (entry->getSearchString().startsWithIgnoreCase(searchString_)) {
        clearHighlighted();
        entry->setHighlighted(true);
        repaint();
        scrollIfNecessaryTo(getHighlightedEntryIndex());
        viewport_->setViewPosition(entry->getPosition());
        return true;
      }
    }
  }
  return true;
}

void EarComboBoxPopup::focusLost(FocusChangeType cause) {
  if (!hasKeyboardFocus(true)) {
    hide();
  }
}

void EarComboBoxPopup::paint(Graphics& g) {
  g.setColour(findColour(backgroundColourId));
  fillRoundedRectangle(g, 0, 0, getWidth(), getHeight(), 4);
  g.setColour(EarColours::Area03dp);
  fillRoundedRectangle(g, 0, comboBox_->getHeight(), getWidth(),
                       getHeight() - comboBox_->getHeight(), 4.f);

  {
    float triangleWidth = 7.75f;
    float triangleHeight = 3.88f;
    Path arrowPath;
    arrowPath.addTriangle(-triangleWidth / 2.f, -triangleHeight / 2.f,
                          triangleWidth / 2.f, -triangleHeight / 2.f, 0.f,
                          triangleHeight / 2.f);
    arrowPath.applyTransform(
        AffineTransform::rotation(MathConstants<float>::pi));
    juce::Point<float> arrowPosition{comboBox_->getWidth() - 14.f,
                                     comboBox_->getHeight() / 2.f - 1.f};
    arrowPath.applyTransform(AffineTransform::translation(arrowPosition));
    g.setColour(findColour(arrowColourId));
    g.fillPath(arrowPath);
  }

  if (isEnabled() && hasKeyboardFocus(false)) {
    g.setColour(findColour(ComboBox::focusedOutlineColourId));
  } else {
    g.setColour(findColour(ComboBox::outlineColourId));
  }
  auto entry = getSelectedEntry();
  if (entry) {
    entry->drawEntryInButton(g, comboBox_->getLocalBounds());
  } else {
    g.setFont(EarFonts::Items);
    g.setColour(EarColours::ComboBoxText);
    g.drawText(
        comboBox_->getDefaultText(),
        comboBox_->getLocalBounds().withTrimmedLeft(14).withTrimmedRight(28),
        Justification::left);
  }
}

void EarComboBoxPopup::resized() {}

EarComboBox::EarComboBox() : popup_(std::make_unique<EarComboBoxPopup>(this)) {
  setColour(backgroundColourId, EarColours::Area01dp);
  setColour(textColourId, EarColours::ComboBoxText);
  setColour(outlineColourId, EarColours::ComboBoxText);
  setColour(buttonColourId, EarColours::ComboBoxText);
  setColour(arrowColourId, EarColours::Text.withAlpha(Emphasis::medium));
  setColour(focusedOutlineColourId, EarColours::Primary);

  selectEntry(-1, dontSendNotification);

  setWantsKeyboardFocus(true);
}

bool EarComboBox::isPopupActive() const { return popup_->isVisible(); }

void EarComboBox::paint(Graphics& g) {
  g.setColour(findColour(backgroundColourId));
  fillRoundedRectangle(g, 0, 0, getWidth(), getHeight(), 4);
  {
    float triangleWidth = 7.75f;
    float triangleHeight = 3.88f;
    Path arrowPath;
    arrowPath.addTriangle(-triangleWidth / 2.f, -triangleHeight / 2.f,
                          triangleWidth / 2.f, -triangleHeight / 2.f, 0.f,
                          triangleHeight / 2.f);
    juce::Point<float> arrowPosition{getWidth() - 14.f,
                                     getHeight() / 2.f + 1.f};
    arrowPath.applyTransform(AffineTransform::translation(arrowPosition));
    g.setColour(findColour(arrowColourId));
    g.fillPath(arrowPath);
  }

  // if (isEnabled() && hasKeyboardFocus(false)) {
  //   g.setColour(findColour(ComboBox::focusedOutlineColourId));
  // } else {
  //   g.setColour(findColour(ComboBox::outlineColourId));
  // }
  auto entry = getSelectedEntry();
  if (entry) {
    entry->drawEntryInButton(g, getLocalBounds());
  } else {
    g.setFont(EarFonts::Items);
    g.setColour(EarColours::ComboBoxText);
    g.drawText(defaultText_,
               getLocalBounds().withTrimmedLeft(14).withTrimmedRight(28),
               Justification::left);
  }
};

void EarComboBox::mouseUp(const MouseEvent& event) {
  if (isEnabled() && event.getNumberOfClicks() == 1) {
    openPopup();
    repaint();
  }
}

void EarComboBox::setDefaultText(const String& text) {
  defaultText_ = text;
  repaint();
}
String EarComboBox::getDefaultText() const { return defaultText_; }

void EarComboBox::addEntry(std::unique_ptr<EarComboBoxEntry> entry) {
  popup_->addEntry(std::move(entry));
}

void EarComboBox::addTextEntry(const String& text) {
  addEntry(std::make_unique<EarComboBoxTextEntry>(text));
}

void EarComboBox::addColourEntry(const Colour& colour) {
  addEntry(std::make_unique<EarComboBoxColourEntry>(colour));
}

void EarComboBox::addSectionEntry(const String& text) {
  addEntry(std::make_unique<EarComboBoxSectionEntry>(text));
}

void EarComboBox::clearEntries() {
  currentSelectedId_ = -1;
  popup_->clearEntries();
}

void EarComboBox::openPopup() { popup_->show(); }
void EarComboBox::closePopup() { popup_->hide(); }

void EarComboBox::resized() {}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
