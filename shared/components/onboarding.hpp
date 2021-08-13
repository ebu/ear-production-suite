#pragma once

#include "JuceHeader.h"

#include "../binary_data.hpp"
#include "ear_button.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/slider.hpp"
#include "onboarding_slide.hpp"
#include "overlay.hpp"
#include "segment_progress_bar.hpp"

namespace ear {
namespace plugin {
namespace ui {

class Onboarding : public Component {
 public:
  Onboarding()
      : progressBar_(std::make_unique<SegmentProgressBar>()),
        prevButton_(std::make_unique<EarButton>()),
        nextButton_(std::make_unique<EarButton>()),
        endButton_(std::make_unique<EarButton>()),
        moreButton_(std::make_unique<EarButton>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    auto slide1 = std::make_unique<OnboardingSlide>();
    slide1->setHeading("First things first");
    AttributedString slide1Text(
        "Create your first object-based production with the EAR Production "
        "Suite and discover the possibilities of the open standards Audio "
        "Definition Model, Broadcast Wave 64 and the ITU ADM Monitoring. This "
        "short introduction will show you how to set up your project in "
        "REAPER.");
    slide1Text.setFont(font::RobotoSingleton::instance().getRegular(16.f));
    slide1Text.setColour(EarColours::Text);
    slide1Text.setLineSpacing(5.f);
    slide1->setText(slide1Text);
    slide1->setBackgroundImage(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::studio_mockup_png,
                                      binary_data::studio_mockup_pngSize)));
    addTab(std::move(slide1));

    auto slide2 = std::make_unique<OnboardingSlide>();
    slide2->setHeading("Create an object-based scene");
    AttributedString slide2Text(
        "The EAR Production Suite consists of the VST plugins EAR Object, EAR "
        "Direct Speakers, EAR Scene and EAR Monitoring. Each has to be "
        "insterted on a separate track and then routed accordingly in REAPER. "
        "The EAR Scene plugin can only be inserted once per session. The "
        "Master send should be disabled for all tracks.");
    slide2Text.setFont(font::RobotoSingleton::instance().getRegular(16.f));
    slide2Text.setColour(EarColours::Text);
    slide2Text.setLineSpacing(5.f);
    slide2->setText(slide2Text);
    slide2->setSlideImage(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::onboarding_slide2_png,
                                      binary_data::onboarding_slide2_pngSize)));
    addTab(std::move(slide2));

    auto slide3 = std::make_unique<OnboardingSlide>();
    slide3->setHeading("Discover Objects and Direct Speakers");
    AttributedString slide3Text;
    slide3Text.append(
        "Create audio objects or insert common channel-based audio into your "
        "project.\n\n",
        font::RobotoSingleton::instance().getMedium(17.f));
    slide3Text.append(
        "Each EAR Object and EAR Direct Speakers plugin must be "
        "routed to the track of the EAR Scene. Route for each EAR Object one "
        "mono channel, for each EAR Direct Speakers the appropriate number of "
        "channels into the EAR Scene. Each input channel can only be used "
        "once.",
        font::RobotoSingleton::instance().getRegular(16.f));
    slide3Text.setColour(EarColours::Text);
    slide3Text.setLineSpacing(5.f);
    slide3->setText(slide3Text);
    slide3->setSlideImage(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::onboarding_slide3_png,
                                      binary_data::onboarding_slide3_pngSize)));
    addTab(std::move(slide3));

    auto slide4 = std::make_unique<OnboardingSlide>();
    slide4->setHeading("Explore EAR Scene and Monitoring");
    AttributedString slide4Text;
    slide4Text.append(
        "Create object-based audioscenes and listen to your project.\n\n",
        font::RobotoSingleton::instance().getMedium(17.f));
    slide4Text.append(
        "Route all 64 channels of the EAR Scene into the EAR Monitoring plugin "
        "with the setup of your choice. Set the hardware routing in the EAR "
        "Monitoring track according to your monitoring setup.",
        font::RobotoSingleton::instance().getRegular(16.f));
    slide4Text.setColour(EarColours::Text);
    slide4Text.setLineSpacing(5.f);
    slide4->setText(slide4Text);
    slide4->setSlideImage(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::onboarding_slide4_png,
                                      binary_data::onboarding_slide4_pngSize)));
    addTab(std::move(slide4));

    auto slide5 = std::make_unique<OnboardingSlide>();
    slide5->setHeading("Final steps");
    AttributedString slide5Text(
        "For each EAR Object and EAR Direct Speakers plugin, set the routing "
        "in the routing dropdown to match the previously configured REAPER "
        "audio routing. For more information about the EAR Production Suite "
        "please visit our website.");
    slide5Text.setFont(font::RobotoSingleton::instance().getRegular(16.f));
    slide5Text.setColour(EarColours::Text);
    slide5Text.setLineSpacing(5.f);
    slide5->setText(slide5Text);
    slide5->setSlideImage(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::onboarding_slide5_png,
                                      binary_data::onboarding_slide5_pngSize)));

    endButton_->setButtonText("Got it!");
    endButton_->setToggleState(true, NotificationType::dontSendNotification);
    endButton_->setBounds(juce::Rectangle<int>{365, 413, 120, 26});
    endButton_->onClick = [&]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker,
                             [this](Listener& l) { l.endButtonClicked(this); });
      if (checker.shouldBailOut()) {
        return;
      }
    };

    slide5->addAndMakeVisible(endButton_.get());
    moreButton_->setButtonText("More help");
    moreButton_->setBounds(juce::Rectangle<int>{495, 413, 120, 26});
    moreButton_->setEnabled(true);
    moreButton_->onClick = [&]() {
      Component::BailOutChecker checker(this);
      listeners_.callChecked(
          checker, [this](Listener& l) { l.moreButtonClicked(this); });
      if (checker.shouldBailOut()) {
        return;
      }
    };

    slide5->addAndMakeVisible(moreButton_.get());
    addTab(std::move(slide5));

    prevButton_->setOffStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::previous_icon_svg,
                                      binary_data::previous_icon_svgSize)));
    prevButton_->setOnStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::previous_icon_svg,
                                      binary_data::previous_icon_svgSize)));
    prevButton_->onClick = [this]() {
      int index = this->getSelected();
      --index;
      index = index <= 0 ? 0 : index;
      this->setSelected(index);
    };

    nextButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::next_icon_svg, binary_data::next_icon_svgSize)));
    nextButton_->setOnStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::next_icon_svg, binary_data::next_icon_svgSize)));
    nextButton_->onClick = [this]() {
      int index = this->getSelected();
      int size = this->getNumberOfTabs();
      ++index;
      index = index >= size ? size : index;
      this->setSelected(index);
    };

    for (auto& tab : tabs_) {
      addChildComponent(tab.get());
    }

    addChildComponent(prevButton_.get());
    addChildComponent(nextButton_.get());
    addAndMakeVisible(progressBar_.get());

    setSelected(0);
  }

  ~Onboarding() {}

  void addTab(std::unique_ptr<Component> component) {
    tabs_.push_back(std::move(component));
    addChildComponent(component.get());
    progressBar_->setSize(tabs_.size());
  }

  void setSelected(int index) {
    if (index != index_) {
      index_ = index;
      progressBar_->setSelected(index_);
      repaint();
    }
  }

  int getSelected() { return index_; }

  int getNumberOfTabs() { return tabs_.size(); }

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));
    for (std::size_t i = 0; i < tabs_.size(); ++i) {
      tabs_[i]->setVisible(i == index_);
    }
    prevButton_->setVisible(index_ != 0);
    nextButton_->setVisible(index_ != tabs_.size() - 1);
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
  };

  void resized() override {
    auto area = getLocalBounds();
    for (auto& tab : tabs_) {
      tab->setBounds(area);
    }

    prevButton_->setBounds(
        area.removeFromLeft(70.f).withSizeKeepingCentre(48, 30));
    nextButton_->setBounds(
        area.removeFromRight(70.f).withSizeKeepingCentre(48, 30));
    progressBar_->setBounds(
        area.removeFromBottom(50).withSizeKeepingCentre(200, 50));
  }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void endButtonClicked(Onboarding* onboarding) = 0;
    virtual void moreButtonClicked(Onboarding* onboarding) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  int index_;
  std::vector<std::unique_ptr<Component>> tabs_;
  std::unique_ptr<SegmentProgressBar> progressBar_;
  std::unique_ptr<EarButton> nextButton_;
  std::unique_ptr<EarButton> prevButton_;
  std::unique_ptr<EarButton> endButton_;
  std::unique_ptr<EarButton> moreButton_;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Onboarding)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
