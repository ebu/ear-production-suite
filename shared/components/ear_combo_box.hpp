#pragma once

#include "JuceHeader.h"

#include "look_and_feel/colours.hpp"

#include <vector>
#include <cassert>

namespace ear {
namespace plugin {
namespace ui {

class EarComboBox;

class EarComboBoxEntry : public Component {
public:
	EarComboBoxEntry(int id = 0);

	bool isSelected() const;
	void setSelected(bool selected);
	bool isSelectable() const;
	void setSelectable(bool selectable);
	bool isHighlighted() const;
	void setHighlighted(bool highlighted);

	int getId();

	String getSearchString() { return searchString_; }

	void paint(Graphics& g) override;

	virtual void drawEntryInList(Graphics& g, juce::Rectangle<int> area) = 0;
	virtual void drawEntryInButton(Graphics& g, juce::Rectangle<int> area) = 0;

	enum ColourIds {
		backgroundColourId = 0x741615fb,
		selectedColourId = 0x4ddb7c7a,
		highlightColourId = 0x5f3c3ed6,
	};

protected:
	bool isSelectable_ = true;
	String searchString_ = "";
	int id_;

private:
	juce::Point<int> mouseXyRelative_;
	bool selected_ = false;
	bool highlighted_ = false;
};

class EarComboBoxTextEntry : public EarComboBoxEntry {
public:
	EarComboBoxTextEntry(const String& text = "", int id = 0);

	void setText(const String& text);
	String getText() const;

	virtual void drawEntryInList(Graphics& g, juce::Rectangle<int> area) override;
	virtual void drawEntryInButton(Graphics& g,
		juce::Rectangle<int> area) override;

	enum ColourIds {
		textColourId = 0xb0f09c73,
	};

private:
	String text_;
};

class EarComboBoxColourEntry : public EarComboBoxEntry {
public:
	EarComboBoxColourEntry(Colour colour = Colour{}, int id = 0);

	void setEntryColour(const Colour& colour);
	Colour getEntryColour() const;

	virtual void drawEntryInList(Graphics& g, juce::Rectangle<int> area) override;
	virtual void drawEntryInButton(Graphics& g,
		juce::Rectangle<int> area) override;

private:
	Colour colour_;
};

class EarComboBoxSectionEntry : public EarComboBoxEntry {
public:
	EarComboBoxSectionEntry(const String& text = "", int id = 0);

	void setText(const String& text);
	String getText() const;

	virtual void drawEntryInList(Graphics& g, juce::Rectangle<int> area) override;
	virtual void drawEntryInButton(Graphics& g,
		juce::Rectangle<int> area) override;

	enum ColourIds {
		textColourId = 0x3530140b,
	};

private:
	String text_;
};
enum class PopupDirection { UP, DOWN };

class EarComboBoxPopup : public Component {
public:
	EarComboBoxPopup(EarComboBox* comboBox);

	enum ColourIds {
		backgroundColourId = 0x3fdd2f50,
		textColourId = 0x6da3296a,
		outlineColourId = 0x9fc1b078,
		buttonColourId = 0xe9a4a679,
		arrowColourId = 0xf8e6df14,
		focusedOutlineColourId = 0x68d3e45e,
	};

	void addEntry(std::unique_ptr<EarComboBoxEntry> entry);
	bool selectEntry(int index);
	int getIndexForEntry(const EarComboBoxEntry* newEntry) const {
		auto it = std::find_if(
			entries_.begin(), entries_.end(),
			[&newEntry](const std::unique_ptr<EarComboBoxEntry>& entry) {
				return entry.get() == newEntry;
			});
		return std::distance(entries_.begin(), it);
	}

	void clearEntries();

	void clearSelection();
	void clearHighlighted();

	EarComboBoxEntry* getSelectedEntry() {
		int index = getSelectedEntryIndex();
		if (index != -1) {
			return entries_.at(index).get();
		}
		return nullptr;
	}

	int getSelectedEntryIndex() {
		for (int i = 0; i < entries_.size(); ++i) {
			if (entries_.at(i)->isSelected()) {
				return i;
			}
		}
		return -1;
	}

	int getHighlightedEntryIndex() {
		for (int i = 0; i < entries_.size(); ++i) {
			if (entries_.at(i)->isHighlighted()) {
				return i;
			}
		}
		return -1;
	}

	int getEntryById(int id) {
		for (int i = 0; i < entries_.size(); ++i) {
			if (entries_.at(i)->getId() == id) {
				return i;
			}
		}
		return -1;
	}

	void show();
	void hide();

	int getHeightOfAllEntries() const;
	Path getBoundsPath() const;
	Path getLocalBoundsPath() const;

	void mouseEnter(const MouseEvent& event) override {
		clearHighlighted();
		repaint();
	}

	void mouseMove(const MouseEvent& event) override {
		if (lastMousePosition_ == event.getScreenPosition()) {
			return;
		}
		lastMousePosition_ = event.getScreenPosition();
		for (auto& entry : entries_) {
			if (entry->getScreenBounds().contains(event.getScreenPosition())) {
				clearHighlighted();
				entry->setHighlighted(true);
				repaint();
				return;
			}
		}
	}

	static int rescaleMouseWheelDistance(float distance,
		int singleStepSize) noexcept {
		if (distance == 0.0f) return 0;
		distance *= 14.0f * singleStepSize;
		return roundToInt(distance < 0 ? jmin(distance, -1.0f)
			: jmax(distance, 1.0f));
	}

	void mouseWheelMove(const MouseEvent& e,
		const MouseWheelDetails& wheel) override {
		if (!(e.mods.isAltDown() || e.mods.isCtrlDown() ||
			e.mods.isCommandDown())) {
			const bool canScrollVert = viewport_->isVerticalScrollBarShown();
			const bool canScrollHorz = viewport_->isHorizontalScrollBarShown();

			if (canScrollHorz || canScrollVert) {
				auto deltaX = rescaleMouseWheelDistance(wheel.deltaX, 40);
				auto deltaY = rescaleMouseWheelDistance(wheel.deltaY, 40);

				auto pos = viewport_->getViewPosition();

				if (deltaX != 0 && deltaY != 0 && canScrollHorz && canScrollVert) {
					pos.x -= deltaX;
					pos.y -= deltaY;
				}
				else if (canScrollHorz &&
					(deltaX != 0 || e.mods.isShiftDown() || !canScrollVert)) {
					pos.x -= deltaX != 0 ? deltaX : deltaY;
				}
				else if (canScrollVert && deltaY != 0) {
					pos.y -= deltaY;
				}

				if (pos != viewport_->getViewPosition()) {
					viewport_->setViewPosition(pos);
				}
			}
		}
		for (auto& entry : entries_) {
			if (entry->getScreenBounds().contains(e.getScreenPosition())) {
				clearHighlighted();
				entry->setHighlighted(true);
			}
		}
	}

	void mouseUp(const MouseEvent& event) override;

	bool keyStateChanged(const bool isKeyDown) override {
		if ((KeyPress(KeyPress::escapeKey).isCurrentlyDown() ||
			KeyPress(KeyPress::returnKey).isCurrentlyDown())) {
			return false;
		}
		return false;
	}

	virtual bool keyPressed(const KeyPress& key) override;
	virtual void focusLost(FocusChangeType cause) override;

	void scrollIfNecessaryTo(int index) {
		if (index < entries_.size()) {
			auto& entry = entries_.at(index);
			auto entryBounds = entry->getBounds();
			entryBounds = entryBounds.withWidth(viewport_->getViewArea().getWidth());
			auto viewportBounds = viewport_->getViewArea();
			if (!viewportBounds.contains(entryBounds)) {
				if (viewportBounds.getY() > entryBounds.getY()) {
					viewport_->setViewPosition(entry->getPosition());
				}
				else {
					viewport_->setViewPosition(entry->getPosition().translated(
						0, -viewportBounds.getHeight() + entry->getHeight()));
				}
			}
		}
	}

	void paint(Graphics& g) override;
	void resized() override;

private:
	juce::Rectangle<int> buttonArea_;
	int padding_ = 10;
	juce::Rectangle<int> scrollArea_;
	Component* parent_;
	EarComboBox* comboBox_;
	std::unique_ptr<Viewport> viewport_;
	std::unique_ptr<Component> listView_;
	std::vector<std::unique_ptr<EarComboBoxEntry>> entries_;
	juce::Point<int> lastMousePosition_;
	Time lastTime_;
	String searchString_ = "";
};

class EarComboBox : public Component,
	public Value::Listener,
	private AsyncUpdater {
public:
	EarComboBox();

	enum ColourIds {
		backgroundColourId = 0x472ed91f,
		textColourId = 0xe4afe538,
		outlineColourId = 0xf78afc85,
		buttonColourId = 0x5749a4d7,
		arrowColourId = 0xadfa74fe,
		focusedOutlineColourId = 0x0f78f581,
	};

	bool isPopupActive() const;

	void paint(Graphics& g) override;

	void mouseUp(const MouseEvent& event) override;

	void setDefaultText(const String& text);
	String getDefaultText() const;

	void addEntry(std::unique_ptr<EarComboBoxEntry> entry);
	void addTextEntry(const String& text, int id = 0);
	void addColourEntry(const Colour& colour, int id = 0);
	void addSectionEntry(const String& text, int id = 0);

	void selectEntry(int newIndex, NotificationType notification) {
		if (newIndex != currentSelectedIndex_) {
			currentSelectedIndex_ = newIndex;
			popup_->selectEntry(newIndex);
			if (currentSelectedIndexValue_ != newIndex) {
				currentSelectedIndexValue_ = newIndex;
			}
			repaint();
			triggerChangeMessage(notification);
		}
	}
	void selectEntry(const EarComboBoxEntry* entry,
		NotificationType notification) {
		selectEntry(popup_->getIndexForEntry(entry), notification);
	}
	int getSelectedEntryIndex() { return currentSelectedIndexValue_.getValue(); }
	EarComboBoxEntry* getSelectedEntry() { return popup_->getSelectedEntry(); }

	bool hasSelection() { return getSelectedEntryIndex() >= 0; }

	bool setSelectedId(int id, NotificationType notification) {
		auto index = popup_->getEntryById(id);
		if (index < 0) return false;
		selectEntry(index, notification);
		return true;
	}
	int getSelectedId() {
		auto selectedEntry = getSelectedEntry();
		assert(selectedEntry);
		return selectedEntry->getId();
	}

	void clearEntries();

	void openPopup();
	void closePopup();

	virtual bool keyPressed(const KeyPress& key) override {
		if (key == KeyPress::deleteKey || key == KeyPress::backspaceKey) {
			selectEntry(-1, sendNotificationAsync);
			repaint();
			return true;
		}
		return false;
	}

	void resized() override;

	Value& getSelectedValueObject() noexcept {
		return currentSelectedIndexValue_;
	}

	void triggerChangeMessage(NotificationType notification) {
		if (notification != dontSendNotification) {
			if (notification == sendNotificationSync)
				handleAsyncUpdate();
			else
				triggerAsyncUpdate();
		}
	}

	void handleAsyncUpdate() override {
		Component::BailOutChecker checker(this);
		listeners_.callChecked(checker,
			[this](Listener& l) { l.comboBoxChanged(this); });
		if (checker.shouldBailOut()) return;
		if (onValueChange) {
			onValueChange(getSelectedEntryIndex());
		}
		cancelPendingUpdate();
	}

	void valueChanged(Value& value) override {
		if (value.refersToSameSourceAs(currentSelectedIndexValue_)) {
			selectEntry(currentSelectedIndexValue_.getValue(), dontSendNotification);
		}
	}

	void enablementChanged() override {
		if (isEnabled()) {
			setAlpha(1.f);
		}
		else {
			setAlpha(0.38f);
			closePopup();
		}
	}

	std::function<void(int)> onValueChange;

	class Listener {
	public:
		virtual ~Listener() = default;
		virtual void comboBoxChanged(EarComboBox* comboBoxThatHasChanged) = 0;
	};

	void addListener(Listener* l) { listeners_.add(l); }
	void removeListener(Listener* l) { listeners_.remove(l); }

private:
	int currentSelectedIndex_ = 0;
	Value currentSelectedIndexValue_;
	String defaultText_;
	ListenerList<Listener> listeners_;
	std::unique_ptr<EarComboBoxPopup> popup_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarComboBox)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
