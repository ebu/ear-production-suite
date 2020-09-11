#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <gmock/gmock.h>

namespace {
   class GTestListener : public ::testing::EmptyTestEventListener {
       virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result) {
           INFO(test_part_result.summary());
           if(test_part_result.line_number() > 0) {
             INFO(test_part_result.file_name() << ", line " << test_part_result.line_number());
           }
           CHECK(!test_part_result.failed());
       }

   };
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::TestEventListeners& listeners =
            ::testing::UnitTest::GetInstance()->listeners();
      listeners.Append(new GTestListener);
    int result = Catch::Session().run(argc, argv);
    return result;
}
