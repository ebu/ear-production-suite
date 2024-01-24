#include "include_gmock.h"
#include <array>
#include <vector>
#include <catch2/catch_all.hpp>
#include <bw64/bw64.hpp>
#include <adm/document.hpp>
#include <adm/utilities/object_creation.hpp>
#include "testfiles.h"
#include "testhelpers.h"
#include "tempdir.h"

#include "channelindexer.h"
#include "pluginsuite.h"
#include "pcmsourcecreator.h"
#include "mocks/reaperapi.h"
#include "mocks/pcmgroup.h"
#include "mocks/pcmgroupregistry.h"
#include "mocks/projectelements.h"
#include "mocks/pcmreader.h"
#include "mocks/pcmwriter.h"
#include "mocks/pcmwriterfactory.h"
#include "mocks/channelrouter.h"
#include "mocks/importlistener.h"
#include "mocks/importreporter.h"
#include "mocks/pluginsuite.h"

using namespace admplug;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ReturnRefOfCopy;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::Mock;
using ::testing::AnyNumber;
using ::testing::StrEq;
using Catch::Approx;

namespace {
std::vector<ADMChannel> getTestChannels(int count) {
    std::vector<ADMChannel> channels;
    for(auto i = 0; i != count; ++i) {
        channels.emplace_back(adm::AudioObject::create(
                                adm::AudioObjectName{"Test " + std::to_string(i)}),
                              adm::AudioChannelFormat::create(
                                adm::AudioChannelFormatName{"Test " + std::to_string(i)}, adm::TypeDefinition::OBJECTS),
                              adm::AudioPackFormat::create(
                                adm::AudioPackFormatName{"Test " + std::to_string(i)}, adm::TypeDefinition::OBJECTS),
                               adm::AudioTrackUid::create(), i);
    }
    return channels;
}

std::vector<std::shared_ptr<adm::AudioTrackUid const>> getTestAudioTrackUids(int count) {
    std::vector<std::shared_ptr<adm::AudioTrackUid const>> tuids;
    for(auto i = 0; i != count; ++i) {
        tuids.emplace_back(adm::AudioTrackUid::create());
    }
    return tuids;
}

}

TEST_CASE("Catch library present") {
    REQUIRE(true);
}

TEST_CASE("GMock library correctly hooked in", "[!shouldfail] [.]") {
    auto api = std::make_unique< NiceMock<MockReaperAPI>>();
    EXPECT_CALL(*api, UpdateArrange()).Times(1);
}

TEST_CASE("Mono chna / doc returns 0 index") {
    auto metadata = ADMMetaData("data/channels_mono_adm.wav");
    auto uidRange = metadata.adm()->getElements<adm::AudioTrackUid>();
    REQUIRE(uidRange.size() > 0);
    auto uid = *uidRange.begin();
    auto channelIndexer = std::make_unique<ChannelIndexer>(metadata);
    REQUIRE(channelIndexer->indexOf(uid) == 0);
}

TEST_CASE("Stereo chna / doc returns 1 index for second channel") {
    auto metadata = ADMMetaData("data/channels_stereo_adm.wav");
    auto uidRange = metadata.adm()->getElements<adm::AudioTrackUid>();
    REQUIRE(uidRange.size() == 2);
    auto rightUid = *(uidRange.begin() + 1);
    auto channelIndexer = std::make_unique<ChannelIndexer>(metadata);
    REQUIRE(channelIndexer->indexOf(rightUid) == 1);
}

TEST_CASE("PCMGroup can be instantiated") {
    std::vector<int> chans{ 1 };
    auto pcmGroup = std::make_unique<PCMGroup>(chans);
}

TEST_CASE("PCMGroup returns name based on single channel") {
    std::string name("ch2");
    auto pcmGroup = PCMGroup({ 2 });
    REQUIRE(pcmGroup.name() == name);
}

TEST_CASE("PCMGroup returns name based on channel range of 2") {
    std::string name("ch2-5");
    auto pcmGroup = PCMGroup({ 2,5 });
    REQUIRE(pcmGroup.name() == name);
}

TEST_CASE("PCMGroup returns name based on channel range of 5") {
    std::string name("ch4-1-2-3-5");
    auto pcmGroup = PCMGroup({ 4,1,2,3,5 });
    REQUIRE(pcmGroup.name() == name);
}

TEST_CASE("AudioObject with single channel of audio returns PCMGroup with one channel") {
    auto pcmGroup = PCMGroup({ 1 });
    REQUIRE(pcmGroup.trackIndices().size() == 1);
}

TEST_CASE("AudioObject with two channels of audio returns PCMGroup with two channels") {
    auto pcmGroup = PCMGroup({ 2, 5 });
    REQUIRE(pcmGroup.trackIndices().size() == 2);
}

TEST_CASE("PCMGroups with identical channels of audio are equal") {
    auto pcmGroup = PCMGroup({ 3,2,1 });
    auto pcmGroup2 = PCMGroup({ 3,2,1 });
    REQUIRE(pcmGroup == pcmGroup2);
}

TEST_CASE("PCMGroups with different channels of audio, or different ordering, are not equal") {
    auto pcmGroup = PCMGroup({ 1,2,3 });
    auto pcmGroup2 = PCMGroup({ 3,2,1 });
    REQUIRE(pcmGroup != pcmGroup2);
}

TEST_CASE("PCMGroupRegistry tests") {

    SECTION("PCMGroupRegistry can be instantiated") {
        PCMGroupRegistry registry;
        REQUIRE(true);
    }

    auto audioObject = testhelper::createAudioObject(1);
    auto mediaTakeElement = std::make_shared<NiceMock<MockTakeElement>>();
    PCMGroupRegistry registry;

    NiceMock<MockIPCMGroup> pcmGroup;
    ON_CALL(pcmGroup, trackIndices()).WillByDefault(ReturnRefOfCopy(std::vector<int>({ 0 })));
    NiceMock<MockIPCMGroup> differentGroup;
    ON_CALL(differentGroup, trackIndices()).WillByDefault(ReturnRefOfCopy(std::vector<int>({ 1 })));

    SECTION("PCMGroupRegistry returns allGroups of size 1 after single group added") {
        registry.add(mediaTakeElement, pcmGroup);
        REQUIRE(registry.allGroups().size() == 1);
    }

    SECTION("PCMGroupRegistry returns allGroups of size 1 after two identical groups added") {
        registry.add(mediaTakeElement, pcmGroup);
        registry.add(mediaTakeElement, pcmGroup);
        REQUIRE(registry.allGroups().size() == 1);
    }

    SECTION("PCMGroupRegistry returns allGroups of size 2 after two different groups added") {
        auto audioObject2 = testhelper::createAudioObject(2);

        registry.add(mediaTakeElement, pcmGroup);
        registry.add(mediaTakeElement, differentGroup);
        REQUIRE(registry.allGroups().size() == 2);
    }

    SECTION("PCMGroupRegistry setTakeSourceFor") {
        auto takeElement = std::make_shared < NiceMock<MockTakeElement>>();
        int testVal = 1;
        PCM_source* source = reinterpret_cast<PCM_source*>(&testVal);
        auto fakeFile = std::make_unique<NiceMock<MockIChannelRouter>>();
        ON_CALL(*fakeFile, group()).WillByDefault(Return(PCMGroup{ pcmGroup }));
        ON_CALL(*fakeFile, source()).WillByDefault(Return(source));

        SECTION("PCMGroupRegistry will setSource() on single matched MediaTakeElement when asked to setTakeSourceFor() group") {
            EXPECT_CALL(*takeElement, setSource(source)).Times(1);
            registry.add(takeElement, pcmGroup);
            registry.setTakeSourceFor(PCMGroup{ pcmGroup }, source);
        }

        SECTION("PCMGroupRegistry will setSource() on all matched MediaTakeElements when asked to setTakeSourceFor() group") {
            auto takeElement2 = std::make_shared < NiceMock<MockTakeElement>>();
            EXPECT_CALL(*takeElement, setSource(source)).Times(1);
            EXPECT_CALL(*takeElement2, setSource(source)).Times(1);
            registry.add(takeElement, pcmGroup);
            registry.add(takeElement2, pcmGroup);
            registry.setTakeSourceFor(PCMGroup{ pcmGroup }, source);
        }

        SECTION("PCMGroupRegistry will not setSource() on non-matched MediaTakeElement when asked to setTakeSourceFor() group") {
            auto audioObject2 = testhelper::createAudioObject(2);
            auto takeElement = std::make_shared < NiceMock<MockTakeElement>>();
            EXPECT_CALL(*takeElement, setSource(_)).Times(0);
            registry.add(takeElement, differentGroup);
            registry.setTakeSourceFor(PCMGroup{ pcmGroup }, source);
        }
    }
}

TEST_CASE("PCMReader can be instantiated") {
    Bw64PCMReader reader{ "data/channels_mono_adm.wav" };
}

TEST_CASE("Bw64PCMReader with non-existant file throws exception") {
    std::unique_ptr<Bw64PCMReader> reader;
    REQUIRE_THROWS(reader = std::make_unique<Bw64PCMReader>("nonexistant"));
}

TEST_CASE("PCMReader mono file tests") {

    Bw64PCMReader reader{ "data/channels_mono_adm.wav" };
    auto block = reader.read();
    SECTION("PCMReader reads blocks of 4096 bytes by default") {
        REQUIRE(block->frameCount() == 4096);
    }

    SECTION("PCMReader returns block with file's sample rate") {
        REQUIRE(block->sampleRate() == 48000);
    }

    SECTION("PCMReader returns block with file's sample rate") {
        REQUIRE(block->channelCount() == 1);
    }
}

TEST_CASE("PCMReader stereo file tests") {

    Bw64PCMReader reader{ "data/channels_stereo_adm.wav" };
    auto block = reader.read();
    SECTION("PCMReader reads blocks of 4096 bytes by default") {
        REQUIRE(block->frameCount() == 4096);
    }

    SECTION("PCMReader returns block with 2 channels") {
        REQUIRE(block->channelCount() == 2);
    }
}

namespace {
    std::unique_ptr<NiceMock<MockIPCMWriter>> createWriter() {
        auto writer = std::make_unique<NiceMock<MockIPCMWriter>>();
        EXPECT_CALL(*writer, Die()).Times(AnyNumber());
        return writer;
    }
}

TEST_CASE("BWFRead") {
    test::TempDir dir;
    auto tempFile = dir.path() / boost::filesystem::unique_path();
    auto tempFileStr = tempFile.string();

    constexpr int frameCount{ 4 };
    std::array<float, frameCount> writeBuffer = { 0.1f, 0.2f, 0.3f, 0.4f };
    auto writer = bw64::writeFile(tempFileStr, 1, 48000, 24);
    writer->write(writeBuffer.data(), frameCount);
    writer = nullptr;

    bw64::Bw64Reader reader(tempFileStr.c_str());
    std::array<float, frameCount> readBuffer{};
    auto readFrames = reader.read(readBuffer.data(), frameCount);
    REQUIRE(readFrames == frameCount);
    for (std::size_t i = 0; i != frameCount; ++i) {
        REQUIRE(readBuffer[i] == Approx(writeBuffer[i]));
    }

}

TEST_CASE("PCMWriter") {
    test::TempDir dir;
    auto tempFile = dir.path() / boost::filesystem::unique_path();

    SECTION("Returns filename provided on construction") {
        PCMWriter writer{ tempFile.string() };
        REQUIRE(writer.fileName() == tempFile.string());
    }
    SECTION("Writes blocks to file with correct values and format") {
        int sampleRate = 48000;
        int channelCount = 1;
        constexpr int frameCount = 4;
        NiceMock<MockIPCMBlock> fakeBlock;
        std::vector<float> data({ 0.1f, 0.2f, 0.3f, 0.4f });
        ON_CALL(fakeBlock, sampleRate()).WillByDefault(Return(sampleRate));
        ON_CALL(fakeBlock, channelCount()).WillByDefault(Return(channelCount));
        ON_CALL(fakeBlock, frameCount()).WillByDefault(Return(frameCount));
        ON_CALL(fakeBlock, data()).WillByDefault(ReturnRef(data));
        auto writer = std::make_unique<PCMWriter>(tempFile.string());
        writer->write(fakeBlock);
        writer = nullptr;

#ifdef _WIN32
        char fstring[1024];
        wcstombs(fstring, tempFile.c_str(), 1024);
        bw64::Bw64Reader reader(fstring);
#else
        bw64::Bw64Reader reader(tempFile.c_str());
#endif

        REQUIRE(reader.sampleRate() == sampleRate);
        REQUIRE(reader.channels() == channelCount);
        REQUIRE(reader.numberOfFrames() == frameCount);
        std::array<float, frameCount> readBuffer{};
        auto readFrames = reader.read(readBuffer.data(), frameCount);
        REQUIRE(readFrames == frameCount);
        for (std::size_t i = 0; i != static_cast<std::size_t>(frameCount); ++i) {
            REQUIRE(readBuffer[i] == Approx(data[i]));
        }
    }
}

TEST_CASE("ChannelRouter tests") {
    std::vector<int> indices({ 0 });
    auto fakeWriter = createWriter();
    std::string fileName{ "Test_file" };
    ON_CALL(*fakeWriter, fileName()).WillByDefault(Return(fileName));

    SECTION("ChannelRouter can be instantiated") {
        auto file = ChannelRouter(std::move(fakeWriter), indices);
        REQUIRE(true);
    }

    SECTION("ChannelRouter returns filname of wrapped writer") {
        auto file = ChannelRouter(std::move(fakeWriter), indices);
        REQUIRE(file.fileName() == fileName);
    }

    NiceMock<MockIPCMBlock> fakeBlock;
    ON_CALL(fakeBlock, channelCount()).WillByDefault(Return(2));
    ON_CALL(fakeBlock, frameCount()).WillByDefault(Return(3));
    auto inputData = std::vector<float>{ 1,2,1,2,1,2 };
    ON_CALL(fakeBlock, data()).WillByDefault(ReturnRef(inputData));

    SECTION("On write(), PCMGroupBlock for first channel group writes data extracted from first channel") {
        EXPECT_CALL(*fakeWriter, write(_)).Times(1).WillOnce(Invoke(testhelper::getIPMCBlockMatcherFn(3, 1.0f)));
        auto file = ChannelRouter(std::move(fakeWriter), indices);
        file.write(fakeBlock);
    }

    SECTION("On write(), PCMGroupBlock for double second channel group writes data extracted from first channel twice") {
        std::vector<int> otherIndices({ 1, 1 });
        EXPECT_CALL(*fakeWriter, write(_)).Times(1).WillOnce(Invoke(testhelper::getIPMCBlockMatcherFn(6, 2.0f)));
        auto file = ChannelRouter(std::move(fakeWriter), otherIndices);
        file.write(fakeBlock);
    }
}

namespace {
    std::unique_ptr<NiceMock<MockIPCMWriter>> writerCreationFn(IPCMGroup const&, std::string, std::string) {
        return createWriter();
    }
}

TEST_CASE("PCMSourceCreator", "[.]") {
    NiceMock<MockIADMMetaData> metaData{};
    auto registry = std::make_unique<NiceMock<MockIPCMGroupRegistry>>();
    auto audioObject = adm::createSimpleObject("Test").audioObject;
    NiceMock<MockTrackElement> track;
    auto take = std::make_shared<NiceMock<MockTakeElement>>();
    auto factory = std::make_unique<NiceMock<MockPCMWriterFactory>>();
    NiceMock<MockReaperAPI> api;
    auto readBlock = std::make_shared<NiceMock<MockIPCMBlock>>();
    auto readVec = std::vector<float>({ 1.0f, 2.0f });
    ON_CALL(*readBlock, frameCount()).WillByDefault(Return(readVec.size()));
    ON_CALL(*readBlock, channelCount()).WillByDefault(Return(1));
    ON_CALL(*readBlock, data()).WillByDefault(ReturnRef(readVec));
    ON_CALL(*readBlock, frameCount()).WillByDefault(Return(readVec.size()));
    auto emptyBlock = std::make_shared<NiceMock<MockIPCMBlock>>();
    auto reader = std::make_unique<NiceMock<MockPCMReader>>();
    ON_CALL(*reader, read()).WillByDefault(Return(emptyBlock));
    ON_CALL(*factory, createGroupWriter(_, _, _)).WillByDefault(Invoke(writerCreationFn));
    SECTION("Can be created") {
        PCMSourceCreator creator(std::move(registry),
            std::move(reader),
            std::move(factory),
            metaData);
        REQUIRE(true);
    }

    SECTION("addTake() Registers node with registry") {
        EXPECT_CALL(*registry, add(_, _)).Times(1);
        PCMSourceCreator creator(std::move(registry),
            std::move(reader),
            std::move(factory),
            metaData);
        creator.addTake(take);
    }

}

TEST_CASE("Old source creator tests", "[.]") {
    auto fakeListener = std::make_shared<NiceMock<MockImportListener>>();
    auto fakeReporter = std::make_shared<NiceMock<MockImportReporter>>();
    NiceMock<MockIADMMetaData> metaData{};
    auto registry = std::make_unique<NiceMock<MockIPCMGroupRegistry>>();
    auto audioObject = adm::createSimpleObject("Test").audioObject;
    NiceMock<MockTrackElement> track;
    auto take = std::make_shared<NiceMock<MockTakeElement>>();
    auto factory = std::make_unique<NiceMock<MockPCMWriterFactory>>();
    NiceMock<MockReaperAPI> api;
    auto pluginSet = std::make_shared<NiceMock<MockPluginSuite>>();
    ImportContext fakeContext{fakeListener, fakeReporter, pluginSet, api};
    auto readBlock = std::make_shared<NiceMock<MockIPCMBlock>>();
    auto readVec = std::vector<float>({ 1.0f, 2.0f });
    ON_CALL(*readBlock, frameCount()).WillByDefault(Return(readVec.size()));
    ON_CALL(*readBlock, channelCount()).WillByDefault(Return(1));
    ON_CALL(*readBlock, data()).WillByDefault(ReturnRef(readVec));
    ON_CALL(*readBlock, frameCount()).WillByDefault(Return(readVec.size()));
    auto emptyBlock = std::make_shared<NiceMock<MockIPCMBlock>>();
    auto reader = std::make_unique<NiceMock<MockPCMReader>>();
    ON_CALL(*reader, read()).WillByDefault(Return(emptyBlock));
    ON_CALL(*factory, createGroupWriter(_, _, _)).WillByDefault(Invoke(writerCreationFn));
    SECTION("createSources()", "[.]") {
        SECTION("creates writer for each group") {
            std::vector<IPCMGroup const*> groups;
            NiceMock<MockIPCMGroup> fakeGroup;
            NiceMock<MockIPCMGroup> fakeGroup2;
            groups.push_back(&fakeGroup);
            groups.push_back(&fakeGroup2);
            ON_CALL(*registry, allGroups()).WillByDefault(Return(groups));
            EXPECT_CALL(*factory, createGroupWriter(_, _, _)).Times(2);
            PCMSourceCreator creator(std::move(registry),
                std::move(reader),
                std::move(factory),
                metaData);
            creator.extractSources("test", fakeContext);
        }

        SECTION("with non-empty input file") {
            std::vector<IPCMGroup const*> groups;
            NiceMock<MockIPCMGroup> fakeGroup;
            groups.push_back(&fakeGroup);
            ON_CALL(*registry, allGroups()).WillByDefault(Return(groups));
            auto& factoryRef = *factory;
            auto& readerRef = *reader;
            auto& registryRef = *registry;
            PCMSourceCreator creator(std::move(registry),
                std::move(reader),
                std::move(factory),
                metaData);

            SECTION("reads blocks from input file until it receives an empty block") {
                EXPECT_CALL(readerRef, read()).Times(3).WillOnce(Return(readBlock)).WillOnce(Return(readBlock)).WillOnce(Return(emptyBlock));
                creator.extractSources("test", fakeContext);
            }
            SECTION("With two non-empty blocks") {
                EXPECT_CALL(readerRef, read()).WillOnce(Return(readBlock)).WillOnce(Return(readBlock)).WillOnce(Return(emptyBlock));

                SECTION("writes those blocks to created writers") {
                    auto getCountingWriterCreationFn = [](int writeCount) {
                        auto writeFn = [writeCount](IPCMGroup const&, std::string, std::string) {
                            auto writer = createWriter();
                            EXPECT_CALL(*writer, write(_)).Times(writeCount);
                            return writer;
                        };
                        return writeFn;
                    };
                    ON_CALL(factoryRef, createGroupWriter(_, _, _)).WillByDefault(Invoke(getCountingWriterCreationFn(2)));
                    creator.extractSources("test", fakeContext);
                }

                SECTION("closes writers") {
                    auto getCloseCheckingWriterFn = [](std::vector<NiceMock<MockIPCMWriter>*>& writers) {
                        return [&writers](IPCMGroup const&, std::string, std::string) {
                            auto writer = std::make_unique<NiceMock<MockIPCMWriter>>();
                            EXPECT_CALL(*writer, Die()).Times(1);
                            writers.push_back(writer.get());
                            return writer;
                        };
                    };
                    std::vector<NiceMock<MockIPCMWriter>*> writers;
                    ON_CALL(factoryRef, createGroupWriter(_, _, _)).WillByDefault(Invoke(getCloseCheckingWriterFn(writers)));
                    creator.extractSources("test", fakeContext);
                    for (auto writer : writers) {
                        Mock::VerifyAndClearExpectations(writer);
                    }
                }

                SECTION("Creates PCM_Source with group filename") {
                    auto getPCM_sourceCheckingWriterFn = [](MockReaperAPI const& api) {
                        return [&api](IPCMGroup const&, std::string, std::string) {
                            auto writer = createWriter();
                            int testVal{ 0 };
                            std::string fileName{ "TestFile.wav" };
                            ON_CALL(*writer, fileName()).WillByDefault(Return(std::string{ fileName }));
                            EXPECT_CALL(api, PCM_Source_CreateFromFile(StrEq(std::string{ fileName }))).WillOnce(Return(reinterpret_cast<PCM_source*>(&testVal)));
                            return writer;
                        };
                    };
                    ON_CALL(factoryRef, createGroupWriter(_, _, _)).WillByDefault(Invoke(getPCM_sourceCheckingWriterFn(api)));
                    creator.extractSources("test", fakeContext);
                }

                SECTION("Asks registry to supply source for each group") {
                    EXPECT_CALL(registryRef, setTakeSourceFor(_, _)).Times(static_cast<int>(groups.size()));
                    creator.extractSources("test", fakeContext);
                }
            }

        }
    }
}

TEST_CASE("RoutingWriterFactory") {
    RoutingWriterFactory factory;
    NiceMock<MockIPCMGroup> fakeGroup{};
    std::vector<int> indices({ 0 });
    ON_CALL(fakeGroup, trackIndices()).WillByDefault(ReturnRef(indices));
    std::string groupName{ "group" };
    ON_CALL(fakeGroup, name()).WillByDefault(Return(groupName));
    SECTION("Creates writer") {
        std::string directory{ "dir" };
        std::string fnPrefix{ "prefix" };
        auto writer = factory.createGroupWriter(fakeGroup, directory, fnPrefix);
        REQUIRE(writer);
        SECTION("with wav file made up from path and group name") {
#ifdef _WIN32
            REQUIRE(writer->fileName() == "dir\\prefix---group.wav");
#else
            REQUIRE(writer->fileName() == "dir/prefix---group.wav");
#endif

        }
    }

}
