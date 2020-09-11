#pragma once

#include "communication/common_types.hpp"
#include "programme_element_visitor.hpp"

#include <vector>
#include <string>

namespace ear {
namespace plugin {

class ProgrammeElement {
 public:
  virtual void visit(ProgrammeElementVisitor& visitor) = 0;
  virtual std::shared_ptr<ProgrammeElement> clone() = 0;

 protected:
  virtual ~ProgrammeElement();
};

template <typename T>
std::shared_ptr<ProgrammeElement> shared_element_copy(T const& el) {
  return std::make_shared<T>(el);
}

class ProgrammeHelper {
 public:
  static bool hasChild(
      const std::shared_ptr<ProgrammeElement> element,
      const std::vector<std::shared_ptr<ProgrammeElement>>& vec) {
    return std::find(vec.begin(), vec.end(), element) != vec.end();
  }

  static void addChild(const std::shared_ptr<ProgrammeElement> element,
                       std::vector<std::shared_ptr<ProgrammeElement>>& vec) {
    if (hasChild(element, vec) == true) {
      throw std::runtime_error("ProgrammeElement entry already exists");
    }

    vec.push_back(element);
  }

  static void removeChild(const std::shared_ptr<ProgrammeElement> element,
                          std::vector<std::shared_ptr<ProgrammeElement>>& vec) {
    auto it = std::find(vec.begin(), vec.end(), element);

    if (it == vec.end()) {
      throw std::runtime_error("ProgrammeElement does not exist.");
    } else {
      vec.erase(it);
    }
  }
};

enum class ContentKind {
  NON_DIALOGUE = 0,
  NON_DIALOGUE_UNDEFINED = 1,
  NON_DIALOGUE_MUSIC = 2,
  NON_DIALOGUE_EFFECT = 3,
  DIALOGUE = 4,
  DIALOGUE_UNDEFINED = 5,
  DIALOGUE_STORYLINE = 6,
  DIALOGUE_VOICEOVER = 7,
  DIALOGUE_SPOKEN_SUBTITLE = 8,
  DIALOGUE_AUDIO_DESCRIPTION = 9,
  DIALOGUE_COMMENTARY = 10,
  DIALOGUE_EMERGENCY = 11,
  MIXED = 12,
  MIXED_UNDEFINED = 13,
  MIXED_COMPLETE_MAIN = 14,
  MIXED_MIXED = 15,
  MIXED_HEARING_IMPAIRED = 16
};

class BinauralElement : public ProgrammeElement {
 public:
  BinauralElement() {}
  BinauralElement(communication::ConnectionId id) { connectionId_ = id; }
  virtual ~BinauralElement();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    return shared_element_copy(*this);
  }

  communication::ConnectionId connectionId() const { return connectionId_; }
  void connectionId(const communication::ConnectionId id) {
    connectionId_ = id;
  }

 private:
  communication::ConnectionId connectionId_;
};

class DirectSpeakersElement : public ProgrammeElement {
 public:
  DirectSpeakersElement() {}
  DirectSpeakersElement(communication::ConnectionId id) : connectionId_(id) {}
  virtual ~DirectSpeakersElement();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    return shared_element_copy(*this);
  }

  communication::ConnectionId connectionId() const { return connectionId_; }
  void connectionId(const communication::ConnectionId id) {
    connectionId_ = id;
  }

 private:
  communication::ConnectionId connectionId_;
};

class HoaElement : public ProgrammeElement {
 public:
  HoaElement() {}
  HoaElement(communication::ConnectionId id) : connectionId_(id) {}
  virtual ~HoaElement();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    return shared_element_copy(*this);
  }

  communication::ConnectionId connectionId() const { return connectionId_; }
  void connectionId(const communication::ConnectionId id) {
    connectionId_ = id;
  }

 private:
  communication::ConnectionId connectionId_;
};

class MatrixElement : public ProgrammeElement {
 public:
  MatrixElement() {}
  MatrixElement(communication::ConnectionId id) : connectionId_(id) {}
  virtual ~MatrixElement();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    return shared_element_copy(*this);
  }

  communication::ConnectionId connectionId() const { return connectionId_; }
  void connectionId(const communication::ConnectionId id) {
    connectionId_ = id;
  }

 private:
  communication::ConnectionId connectionId_;
};

class ObjectElement : public ProgrammeElement {
 public:
  ObjectElement() {}
  ObjectElement(communication::ConnectionId id) : connectionId_(id) {}
  virtual ~ObjectElement();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    return shared_element_copy(*this);
  }

  communication::ConnectionId connectionId() const { return connectionId_; }
  void connectionId(const communication::ConnectionId id) {
    connectionId_ = id;
  }

 private:
  communication::ConnectionId connectionId_;
};

class Group : public ProgrammeElement {
 public:
  Group(std::string name, std::string language, ContentKind kind)
      : name_(name), language_(language), kind_(kind) {}
  virtual ~Group();

  virtual void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    auto el = std::make_shared<Group>(name_, language_, kind_);
    for (auto const& child : children_) {
      el->addChild(child->clone());
    }
    return el;
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator begin() const {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator begin() {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator end() const {
    return children_.end();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator end() {
    return children_.end();
  }

  const std::vector<std::shared_ptr<ProgrammeElement>>& data() const {
    return children_;
  }

  void name(const std::string& name) { name_ = name; }
  std::string name() const { return name_; }

  void language(const std::string& language) { language_ = language; }
  std::string language() const { return language_; }

  void kind(ContentKind kind) { kind_ = kind; }
  ContentKind kind() const { return kind_; }

  bool hasChild(const std::shared_ptr<ProgrammeElement> element) const {
    return ProgrammeHelper::hasChild(element, children_);
  }
  void addChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::addChild(element, children_);
  }
  void removeChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::removeChild(element, children_);
  }

 private:
  std::string name_;
  std::string language_;
  ContentKind kind_;
  std::vector<std::shared_ptr<ProgrammeElement>> children_;
};

class Toggle : public ProgrammeElement {
 public:
  Toggle(std::string name, std::shared_ptr<ProgrammeElement> element)
      : name_(name), default_(element) {}
  virtual ~Toggle();

  void visit(ProgrammeElementVisitor& visitor) { visitor(*this); }

  std::shared_ptr<ProgrammeElement> clone() {
    auto el = std::make_shared<Toggle>(name_, default_->clone());
    for (auto const& child : children_) {
      el->addChild(child->clone());
    }
    return el;
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator begin() const {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator begin() {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator end() const {
    return children_.end();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator end() {
    return children_.end();
  }

  const std::vector<std::shared_ptr<ProgrammeElement>>& data() const {
    return children_;
  }

  void name(const std::string& value) { name_ = value; }
  std::string name() const { return name_; }

  std::shared_ptr<ProgrammeElement> getDefault() { return default_; }

  void setDefault(std::shared_ptr<ProgrammeElement> element) {
    default_ = element;
  }

  bool hasChild(const std::shared_ptr<ProgrammeElement> element) const {
    return ProgrammeHelper::hasChild(element, children_);
  }
  void addChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::addChild(element, children_);
  }
  void removeChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::removeChild(element, children_);
  }

 private:
  std::string name_;
  std::shared_ptr<ProgrammeElement> default_;
  std::vector<std::shared_ptr<ProgrammeElement>> children_;
};

class Programme {
 public:
  Programme(std::string name, std::string language)
      : name_(name), language_(language) {}

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator begin() const {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator begin() {
    return children_.begin();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::const_iterator end() const {
    return children_.end();
  }

  std::vector<std::shared_ptr<ProgrammeElement>>::iterator end() {
    return children_.end();
  }

  const std::vector<std::shared_ptr<ProgrammeElement>>& data() const {
    return children_;
  }

  void name(const std::string& value) { name_ = value; }
  std::string name() const { return name_; }

  void language(const std::string& value) { language_ = value; }
  std::string language() const { return language_; }

  bool hasChild(const std::shared_ptr<ProgrammeElement> element) const {
    return ProgrammeHelper::hasChild(element, children_);
  }
  void addChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::addChild(element, children_);
  }
  void removeChild(const std::shared_ptr<ProgrammeElement> element) {
    ProgrammeHelper::removeChild(element, children_);
  }

  std::shared_ptr<Programme> clone() {
    auto copy = std::make_shared<Programme>(name_, language_);
    for (auto const& child : children_) {
      copy->addChild(child->clone());
    }
    return copy;
  }

 private:
  std::string name_;
  std::string language_;
  std::vector<std::shared_ptr<ProgrammeElement>> children_;
};

}  // namespace plugin
}  // namespace ear
