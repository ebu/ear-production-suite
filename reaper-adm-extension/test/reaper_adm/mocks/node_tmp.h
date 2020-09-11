namespace admplug {

class MockMediaTrackNode : public MediaTrackNode {
 public:
  MOCK_METHOD1(addChild,
      Node*(std::unique_ptr<Node> child));
  MOCK_CONST_METHOD0(getElement,
      adm::ElementConstVariant());
  MOCK_CONST_METHOD0(getChildren,
      const std::vector<std::unique_ptr>&());
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite& pluginSuite, ReaperAPI const& api));
  MOCK_CONST_METHOD0(getTrack,
      MediaTrack*());
};

}  // namespace admplug

namespace admplug {

class MockMediaTakeNode : public MediaTakeNode {
 public:
  MOCK_METHOD1(setSource,
      void(PCM_source* souce));
  MOCK_METHOD1(addChild,
      Node*(std::unique_ptr<Node> child));
  MOCK_CONST_METHOD0(getElement,
      adm::ElementConstVariant());
  MOCK_CONST_METHOD0(getChildren,
      const std::vector<std::unique_ptr>&());
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite& pluginSuite, ReaperAPI const& api));
  MOCK_CONST_METHOD0(getTrack,
      MediaTrack*());
};

}  // namespace admplug
