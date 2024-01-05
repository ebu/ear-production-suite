#pragma once

#include <vector>
#include <string>
#include <../src/dynamic_renderer.hpp>
#include "variable_block_adapter.hpp"
#include <chrono>
#include <mutex>

namespace ear {
namespace plugin {

enum BearStatusStates {
  NOT_ATTEMPTED = 0,
  FAILED,
  SUCCEEDED
};

struct BearStatus {
  bear::Config startupConfig;
  BearStatusStates startupSuccess{NOT_ATTEMPTED};
  std::string startupErrorDesc;
  BearStatusStates listenerDataSetSuccess{NOT_ATTEMPTED};
  std::string listenerDataSetErrorDesc;
};

/**
 * @brief Binaural monitoring plugin dsp implementation
 *
 * This class handles the actual DSP by wrapping BEAR
 */
class BinauralMonitoringAudioProcessor {
 public:
  /**
   * @brief
   * Initializes audio processor and BEAR.
   *
   * Number of output channels is always 2.
   *
   * Unlike the monitoring audio processor, the binaural monitoring audio
   * processor does not implement the VariableBlockSizeAdapter and therefore
   * only accepts fixed size blocks (specified on construction).
   * I don't think it is necessary to support variable block sizes as the host
   * will not vary the block size unless the audio device configuration is
   * changed - it which case this class can be reconstructed and a glitch in
   * audio is acceptable.
   *
   * @param objChannels number of object channels to support
   * @param dsChannels number of directspeakers channels to support
   * @param hoaChannels number of HOA channels to support
   * @param blockSize processing block size
   */
  BinauralMonitoringAudioProcessor(
      std::size_t objChannels, std::size_t dsChannels, std::size_t hoaChannels,
      std::size_t sampleRate, std::size_t blockSize, std::string dataFilePath);

  BinauralMonitoringAudioProcessor(const BinauralMonitoringAudioProcessor&) =
      delete;
  BinauralMonitoringAudioProcessor(BinauralMonitoringAudioProcessor&&) = delete;
  BinauralMonitoringAudioProcessor& operator=(
      const BinauralMonitoringAudioProcessor&) = delete;
  BinauralMonitoringAudioProcessor& operator=(
      BinauralMonitoringAudioProcessor&&) = delete;

  BearStatus getBearStatus() { return bearStatus; }

  template <typename InBuffer, typename OutBuffer>
  void process(const InBuffer& in, OutBuffer& out) {
    using InTraits = BufferTraits<InBuffer>;
    using OutTraits = BufferTraits<OutBuffer>;
    doProcess((float**)InTraits::getChannels(in), InTraits::channelCount(in));
  }

  bool pushBearMetadata(size_t channelNum, ear::ObjectsTypeMetadata* metadata);
  bool pushBearMetadata(size_t channelNum,
                        ear::DirectSpeakersTypeMetadata* metadata);
  bool pushBearMetadata(size_t channelNum, ear::HOATypeMetadata* metadata,
                        size_t arbitraryStreamIdentifier);

  std::size_t delayInSamples() const;

  bool configMatches(std::size_t sampleRate, std::size_t blockSize);

  void setListenerOrientation(float quatW, float quatX, float quatY,
                              float quatZ);

  bool rendererError() { return !bearRenderer; }

  void setIsPlaying(bool state) { isPlaying = state; }
  bool getIsPlaying() { return isPlaying; }
  bool updateChannelCounts(std::size_t objChannels, std::size_t dsChannels,
                           std::size_t hoaChannels);

 private:
  void doProcess(float** channelPointers, size_t maxChannels);

  uint64_t framesProcessed{0};
  bool isPlaying{false};

  bear::Config bearConfig;
  std::shared_ptr<bear::DynamicRenderer> bearRenderer;  // TODO - why shared?
  std::mutex bearListenerMutex_;
  bear::Listener bearListener;
  BearStatus bearStatus;

  bool listenerQuatsDirty{false};
  std::array<double, 4> listenerQuats{1.0, 0.0, 0.0, 0.0};

  bear::Time metadataRtime;
  bear::Time metadataDuration;

  // We need to map original channel numbers to contiguous channel numbers
  //   within the different type definitions
  std::vector<int> objChannelMappings;
  std::vector<int> dsChannelMappings;
  std::vector<int> hoaChannelMappings;

  // Bear temp buffers - Save redeclaring on each process call
  std::vector<float> reusableZeroedChannel;
  std::vector<float*>
      bearOutputBuffers_RawPointers;  // Bear wants array of raw pointers
  std::vector<float*> bearObjectInputBuffers_RawPointers;
  std::vector<float*> bearDirectSpeakersInputBuffers_RawPointers;
  std::vector<float*> bearHoaInputBuffers_RawPointers;
};

}  // namespace plugin
}  // namespace ear
