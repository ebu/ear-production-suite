#pragma once

namespace ear {

class MockupPlugin {
 public:
  MockupPlugin() = default;
  virtual ~MockupPlugin() = default;

  MockupPlugin(const MockupPlugin&) = delete;
  MockupPlugin& operator=(const MockupPlugin&) = delete;
  MockupPlugin(MockupPlugin&&) = delete;
  MockupPlugin& operator=(MockupPlugin&&) = delete;
};

}  // namespace ear
