#pragma once

#include "JuceHeader.h"

#include "binary_data.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "store_metadata.hpp"
#include "programme_store.pb.h"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {
class SceneFrontendBackendConnector;

class ElementOverview : public Component, private Timer {
 public:
  ElementOverview()
      : rotateLeftButton_(std::make_unique<EarButton>()),
        rotateRightButton_(std::make_unique<EarButton>()),
        startViewingAngle_(0.f),
        currentViewingAngle_(180.f),
        endViewingAngle_(180.f) {
    rotateLeftButton_->setOffStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::previous_icon_svg,
                                      binary_data::previous_icon_svgSize)));
    rotateLeftButton_->setOnStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::previous_icon_svg,
                                      binary_data::previous_icon_svgSize)));
    rotateLeftButton_->onClick = [this]() { this->rotate(90.f); };

    rotateRightButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::next_icon_svg, binary_data::next_icon_svgSize)));
    rotateRightButton_->setOnStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::next_icon_svg, binary_data::next_icon_svgSize)));
    rotateRightButton_->onClick = [this]() { this->rotate(-90.f); };

    addAndMakeVisible(rotateLeftButton_.get());
    addAndMakeVisible(rotateRightButton_.get());
  }

  int getSectorIndex(float value) {
    float limitedRange = static_cast<int>(std::roundf(value)) % 360;
    if ((limitedRange >= -315 && limitedRange < -225)) {
      return 3;
    } else if ((limitedRange >= -225 && limitedRange < -135)) {
      return 2;
    } else if ((limitedRange >= -135 && limitedRange < -45)) {
      return 1;
    } else if ((limitedRange >= -45 && limitedRange < 45)) {
      return 0;
    } else if (limitedRange >= 45 && limitedRange < 135) {
      return 3;
    } else if (limitedRange >= 135 && limitedRange < 225) {
      return 2;
    } else if (limitedRange >= 225 && limitedRange < 315) {
      return 1;
    } else {
      return 0;
    }
  }

  void paintDirectSpeakers(Graphics& g,
                           proto::DirectSpeakersTypeMetadata const& data,
                           juce::Colour const& colour) {
    auto const& speakers = data.speakers();
    for (auto const& speaker : speakers) {
      if (!speaker.is_lfe()) {
        auto const& position = speaker.position();
        drawCircle(g, position, colour);
      }
    }
  }

  void paintObject(Graphics& g,
                    proto::ObjectsTypeMetadata const& data,
                    juce::Colour const& colour) {
    auto const& position = data.position();
    drawCircle(g, position, colour);
  }

  void paintBackGround(juce::Graphics& g) {
    g.drawImageAt(cachedBackground_, 0, 0);
  }

  void paintElements(juce::Graphics& g) {
    for (auto const& item : visibleItems_) {
      auto const& inputData = item.second.inputMetadata;
      auto colour = Colour(inputData.colour());
      if (inputData.has_ds_metadata()) {
        paintDirectSpeakers(g, inputData.ds_metadata(), colour);
      } else if (inputData.has_obj_metadata()) {
        paintObject(g, inputData.obj_metadata(), colour);
      }
    }
  }

  void paintSphere(Graphics& g) {
    auto sphere = getLocalBounds()
        .toFloat()
        .removeFromBottom(126.f)
        .removeFromLeft(156.f)
        .withTrimmedLeft(50.f)
        .withTrimmedBottom(20.f);
    g.setColour(EarColours::Sphere);
    drawSphereInRect(g, sphere, 1.f);
    g.setFont(EarFonts::Description);
    g.setColour(EarColours::Label);
    auto depthSphere = sphere.withSizeKeepingCentre(
        sphere.getWidth(), depthFactor_ * sphere.getHeight());

    std::vector<String> labels = {
        String::fromUTF8("Front"), String::fromUTF8("+90°"),
        String::fromUTF8("Back"), String::fromUTF8("–90°")};

    int startIndex = getSectorIndex(endViewingAngle_);
    auto frontLabelArea = depthSphere.withY(depthSphere.getBottom() + 2.f);
    g.drawText(labels.at(startIndex), frontLabelArea,
               Justification::centredTop);
    auto rightLabelArea = sphere.withX(sphere.getRight() + 5.f);
    g.drawText(labels.at((startIndex + 1) % 4), rightLabelArea,
               Justification::left);
    auto backLabelArea = depthSphere.withBottom(depthSphere.getY() - 2.f);
    g.drawText(labels.at((startIndex + 2) % 4), backLabelArea,
               Justification::centredBottom);
    auto leftLabelArea = sphere.withRightX(sphere.getX() - 5.f);
    g.drawText(labels.at((startIndex + 3) % 4), leftLabelArea,
               Justification::right);
  }

  void paint(Graphics& g) override {
    g.setOpacity(1.0f);
    paintBackGround(g);
    paintElements(g);
    paintSphere(g);
  }

  void drawSphereInRect(Graphics& g, Rectangle<float> rect, float linewidth) {
    g.drawEllipse(rect, linewidth);
    g.drawEllipse(rect.withSizeKeepingCentre(rect.getWidth(),
                                             depthFactor_ * rect.getHeight()),
                  linewidth);
  }

  void drawCircle(Graphics& g, proto::PolarPosition position, Colour colour) {
    position.set_azimuth(position.azimuth() + currentViewingAngle_);

    float x = std::sin(degreesToRadians(position.azimuth())) *
              std::cos(degreesToRadians(position.elevation())) *
              position.distance();
    float y = std::cos(-degreesToRadians(position.azimuth())) *
              std::cos(degreesToRadians(position.elevation())) *
              position.distance();
    float z =
        std::sin(degreesToRadians(position.elevation())) * position.distance();

    float circleRadiusScaled = circleRadius_ * ((y + 4.f) / 5.f);
    float xProjected = x * sphereArea_.getWidth() / 2.f;
    float yProjected = y * sphereArea_.getHeight() * depthFactor_ / 2.f -
                       z * sphereArea_.getHeight() / 2.f;

    juce::Rectangle<float> circleArea(0.f, 0.f, circleRadiusScaled * 2.f,
                               circleRadiusScaled * 2.f);
    circleArea.translate(-circleRadiusScaled, -circleRadiusScaled);

    circleArea.translate(sphereArea_.getCentreX(), sphereArea_.getCentreY());
    circleArea.translate(xProjected, yProjected);
    g.setColour(
        colour.withAlpha(Emphasis::medium).darker(1.f - (y + 1.f) / 2.f));
    g.fillEllipse(circleArea);
  }

  void setProgramme(proto::Programme programme) {
    programme_ = programme;
    repaint();
  }

  void rotate(float angle) {
    if (isTimerRunning()) {
      stopTimer();
    }
    startViewingAngle_ = currentViewingAngle_;
    endViewingAngle_ += angle;
    float animationRange = endViewingAngle_ - startViewingAngle_;
    if (std::abs(animationRange) >= 1.f) {
      startTimerHz(50);
    } else {
      currentViewingAngle_ = endViewingAngle_;
    }
  }

  void timerCallback() override {
    updateViewingAngle();
    repaint();
  }

  void updateViewingAngle() {
    float animationRange = endViewingAngle_ - startViewingAngle_;
    currentViewingAngle_ += animationRange / 10.f;
    if (animationRange >= 0.f && currentViewingAngle_ > endViewingAngle_) {
      currentViewingAngle_ = endViewingAngle_;
      stopTimer();
    } else if (animationRange < 0.f &&
               currentViewingAngle_ < endViewingAngle_) {
      currentViewingAngle_ = endViewingAngle_;
      stopTimer();
    }
  }

  void resized() override {
    {
      Image img(Image::PixelFormat::RGB, getWidth(), getHeight(), false);
      Graphics g(img);

      // draw background
      auto area = getLocalBounds().toFloat();
      g.setOpacity(1.0f);
      g.setColour(EarColours::Background);
      g.fillRect(area);
      float factorTop = 0.9;
      float factorBottom = 0.8;
      g.setColour(EarColours::Sphere);

      auto xPositionTop = getWidth() / 2.f;
      auto xPositionBottom = getWidth() / 2.f;
      float distanceTop = getWidth() / 30.f;
      float distanceBottom = getWidth() / 1.5f;
      for (int i = 0; i < 10; ++i) {
        g.drawLine(xPositionTop, 0.f, xPositionBottom, (float)getHeight());
        distanceTop *= factorTop;
        xPositionTop += distanceTop;
        distanceBottom *= factorBottom;
        xPositionBottom += distanceBottom;
      }
      xPositionTop = getWidth() / 2.f;
      xPositionBottom = getWidth() / 2.f;
      distanceTop = getWidth() / 20.f;
      distanceBottom = getWidth() / 1.5f;
      for (int i = 1; i < 10; ++i) {
        g.drawLine(xPositionTop, 0.f, xPositionBottom, (float)getHeight());
        distanceTop *= factorTop;
        xPositionTop -= distanceTop;
        distanceBottom *= factorBottom;
        xPositionBottom -= distanceBottom;
      }

      float yPosition = (3.f * getHeight()) / 4.f;
      float distance = getHeight() / 4.f;
      float factor = 0.68f;
      for (int i = 0; i < 15; ++i) {
        g.drawLine(0, yPosition, getWidth(), yPosition);
        distance *= factor;
        yPosition -= distance;
      }
      g.setGradientFill(ColourGradient::vertical(
          EarColours::Background, EarColours::Background.withAlpha(0.f),
          area.withTrimmedTop(getHeight() / 4)
              .withTrimmedBottom(getHeight() / 4)));
      g.fillRect(area);
      g.setColour(EarColours::ComboBoxPopupBackground);
      g.drawRect(area, 2);

      // draw sphere
      area.removeFromBottom(getHeight() / 8.f);

      float sphereSize;
      if (area.getWidth() > area.getHeight()) {
        sphereSize = area.getHeight() - 100;
      } else {
        sphereSize = area.getWidth() - 100;
      }
      sphereArea_ = area.withSizeKeepingCentre(sphereSize, sphereSize);
      g.setColour(EarColours::Sphere);
      drawSphereInRect(g, sphereArea_, 4.f);

      // save Image
      cachedBackground_ = img;
    }
    auto area = getLocalBounds().removeFromBottom(50).withTrimmedBottom(20);
    rotateLeftButton_->setBounds(area.removeFromLeft(50).withTrimmedLeft(20));
    area.removeFromLeft(106);  // skip sphere
    rotateRightButton_->setBounds(area.removeFromLeft(30));
  }

  void itemsAdded(std::vector<ProgrammeObject> const& items) {
    for(auto const& item : items) {
      visibleItems_.insert({getUuid(item), item});
    }
    repaint();
  }

  void itemRemoved(communication::ConnectionId id) {
    visibleItems_.erase(id.getUuid());
    repaint();
  }

  void itemChanged(ProgrammeObject const& item) {
    auto it = visibleItems_.find(getUuid(item));
    if(it != visibleItems_.end()) {
      it->second = item;
      repaint();
    }
  }

  void resetItems(ProgrammeObjects const& objects) {
    visibleItems_.clear();
    for(auto const& object : objects) {
      visibleItems_.insert({getUuid(object), object});
    }
    repaint();
  }

 private:
  static boost::uuids::uuid getUuid(ProgrammeObject const& pair) {
    return communication::ConnectionId(pair.inputMetadata.connection_id()).getUuid();
  }
  proto::Programme programme_;

  std::unique_ptr<EarButton> rotateLeftButton_;
  std::unique_ptr<EarButton> rotateRightButton_;

  Image cachedBackground_;
  std::unordered_map<boost::uuids::uuid,
                     ProgrammeObject,
                     boost::hash<boost::uuids::uuid>> visibleItems_;

  juce::Rectangle<float> sphereArea_;
  const float depthFactor_ = 0.3f;
  const float circleRadius_ = 15.f;

  float startViewingAngle_;
  float endViewingAngle_;
  float currentViewingAngle_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementOverview)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
