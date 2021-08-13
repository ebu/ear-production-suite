#pragma once

namespace ear {
namespace plugin {
class SceneStore;
class ProgrammeStore;
class Group;
class Toggle;
class BinauralElement;
class DirectSpeakersElement;
class HoaElement;
class MatrixElement;
class ObjectElement;

class ProgrammeElementVisitor {
 public:
  virtual ~ProgrammeElementVisitor() = default;
  virtual void operator()(Group const& group) = 0;
  virtual void operator()(Toggle const& toggle) = 0;
  virtual void operator()(BinauralElement const& directSpeakersElement) = 0;
  virtual void operator()(DirectSpeakersElement const& directSpeakersElement) = 0;
  virtual void operator()(HoaElement const& hoaElement) = 0;
  virtual void operator()(MatrixElement const& matrixElement) = 0;
  virtual void operator()(ObjectElement const& objectElement) = 0;
};
}
}
