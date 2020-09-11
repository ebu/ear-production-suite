namespace admplug {

class MockIADMMetaData : public IADMMetaData {
 public:
  MOCK_CONST_METHOD0(chna,
      std::shared_ptr<bw64::ChnaChunk const>());
  MOCK_CONST_METHOD0(axml,
    std::shared_ptr<bw64::AxmlChunk const>());
  MOCK_CONST_METHOD0(adm,
      std::shared_ptr<adm::Document const>());
  MOCK_CONST_METHOD0(fileName,
      std::string());
};

}  // namespace admplug
