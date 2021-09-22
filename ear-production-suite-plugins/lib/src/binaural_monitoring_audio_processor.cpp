#include "binaural_monitoring_audio_processor.hpp"
#include <functional>
#include <iostream>

#define DEFAULT_TENSORFILE_NAME "default.tf"

namespace ear {
namespace plugin {

BinauralMonitoringAudioProcessor::BinauralMonitoringAudioProcessor(
    std::size_t maxObjChannels, std::size_t maxDsChannels, std::size_t maxHoaChannels,
    std::size_t sampleRate, std::size_t blockSize, std::string dataFilePath) {

  isPlaying = false;
  framesProcessed = 0;
  metadataRtime = bear::Time{framesProcessed, sampleRate};
  metadataDuration = bear::Time{blockSize, sampleRate};

  reusableZeroedChannel = std::vector<float>(blockSize, 0.0);
  bearOutputBuffers_RawPointers = std::vector<float *>(2, nullptr);
  bearObjectInputBuffers_RawPointers =
      std::vector<float *>(maxObjChannels, reusableZeroedChannel.data());
  bearDirectSpeakersInputBuffers_RawPointers =
      std::vector<float *>(maxDsChannels, reusableZeroedChannel.data());
  bearHoaInputBuffers_RawPointers =
      std::vector<float *>(maxHoaChannels, reusableZeroedChannel.data());

  objChannelMappings.reserve(maxObjChannels);
  dsChannelMappings.reserve(maxDsChannels);
  hoaChannelMappings.reserve(maxHoaChannels);

  bearConfig.set_period_size(blockSize);
  bearConfig.set_sample_rate(sampleRate);
  bearConfig.set_num_objects_channels(0);
  bearConfig.set_num_direct_speakers_channels(0);
  bearConfig.set_num_hoa_channels(0);
  bearConfig.set_data_path(dataFilePath);
  bearConfig.set_fft_implementation("default");

  bearListener.set_position_cart(std::array<double, 3>{0.0, 0.0, 0.0});

  try {
    bearRenderer = std::make_shared<bear::DynamicRenderer>(blockSize, std::max(std::max(maxObjChannels, maxDsChannels), maxHoaChannels));
    try {
      bearRenderer->set_listener(bearListener);
    } catch (std::exception &e) {
      bearRenderer.reset();
      assert(false);
    }
  } catch (std::exception &e) {
    bearRenderer.reset();
    assert(false);
  }
}

void BinauralMonitoringAudioProcessor::doProcess(float **channelPointers,
                                                 size_t maxChannels) {
  if(!bearRenderer) return;

  if (listenerQuatsDirty) {
    std::lock_guard<std::mutex> lock(bearListenerMutex_);
    bearListener.set_orientation_quaternion(listenerQuats);
    bearRenderer->set_listener(bearListener);
    listenerQuatsDirty = false;
  }

  // Set buffer pointers
  for (size_t tdChannel = 0; tdChannel < objChannelMappings.size(); tdChannel++) {
    if (objChannelMappings[tdChannel] < maxChannels) {
      bearObjectInputBuffers_RawPointers[tdChannel] =
          channelPointers[objChannelMappings[tdChannel]];
    }
  }

  for (size_t tdChannel = 0; tdChannel < dsChannelMappings.size(); tdChannel++) {
    if (dsChannelMappings[tdChannel] < maxChannels) {
      bearDirectSpeakersInputBuffers_RawPointers[tdChannel] =
          channelPointers[dsChannelMappings[tdChannel]];
    }
  }

  for (size_t tdChannel = 0; tdChannel < hoaChannelMappings.size(); tdChannel++) {
    if (hoaChannelMappings[tdChannel] < maxChannels) {
      bearHoaInputBuffers_RawPointers[tdChannel] =
          channelPointers[hoaChannelMappings[tdChannel]];
    }
  }

  bearOutputBuffers_RawPointers[0] = channelPointers[0];
  bearOutputBuffers_RawPointers[1] = channelPointers[1];

  // Process
  bearRenderer->process(objChannelMappings.size(),
                        bearObjectInputBuffers_RawPointers.data(),
                        dsChannelMappings.size(),
                        bearDirectSpeakersInputBuffers_RawPointers.data(),
                        hoaChannelMappings.size(),
                        bearHoaInputBuffers_RawPointers.data(),
                        bearOutputBuffers_RawPointers.data());

  // Prepare for next block
  framesProcessed += bearConfig.get_period_size();
  metadataRtime.assign(framesProcessed, bearConfig.get_sample_rate());

  objChannelMappings.clear();
  dsChannelMappings.clear();
  hoaChannelMappings.clear();

  std::fill(bearObjectInputBuffers_RawPointers.begin(),
            bearObjectInputBuffers_RawPointers.end(),
            reusableZeroedChannel.data());
  std::fill(bearDirectSpeakersInputBuffers_RawPointers.begin(),
            bearDirectSpeakersInputBuffers_RawPointers.end(),
            reusableZeroedChannel.data());
  std::fill(bearHoaInputBuffers_RawPointers.begin(),
            bearHoaInputBuffers_RawPointers.end(),
            reusableZeroedChannel.data());
}

bool BinauralMonitoringAudioProcessor::pushBearMetadata(
    size_t channelNum, ear::ObjectsTypeMetadata *metadata) {
  bear::ObjectsInput bearMetadata;
  bearMetadata.rtime = metadataRtime;
  bearMetadata.duration = metadataDuration;
  bearMetadata.type_metadata = *metadata;
  objChannelMappings.push_back(channelNum);
  return bearRenderer->add_objects_block(objChannelMappings.size() - 1,
                                         bearMetadata);
}

bool BinauralMonitoringAudioProcessor::pushBearMetadata( // for DS, this is done once per speaker/channel so no need to loop round
    size_t channelNum, ear::DirectSpeakersTypeMetadata *metadata) {
  bear::DirectSpeakersInput bearMetadata;
  bearMetadata.rtime = metadataRtime;
  bearMetadata.duration = metadataDuration;
  bearMetadata.type_metadata = *metadata;
  dsChannelMappings.push_back(channelNum); // Here channelnum = starting channel + index
  return bearRenderer->add_direct_speakers_block(dsChannelMappings.size() - 1,
                                                 bearMetadata);
}

bool BinauralMonitoringAudioProcessor::pushBearMetadata( // for HOA, this is done per asset. Need to iterate round channels within this.
    size_t channelNum, ear::HOATypeMetadata *metadata, size_t arbitraryStreamIdentifier) {

  if(metadata->degrees.size() == 0) return false;
  bear::HOAInput bearMetadata;
  bearMetadata.channels.reserve(metadata->degrees.size());

  for (int i = 0; i < metadata->degrees.size(); i++) {
    hoaChannelMappings.push_back(channelNum + i);
    bearMetadata.channels.push_back(hoaChannelMappings.size() -1);
  }

  bearMetadata.rtime = metadataRtime;
  bearMetadata.duration = metadataDuration;
  bearMetadata.type_metadata = *metadata;

  return bearRenderer->add_hoa_block(arbitraryStreamIdentifier,
                                     bearMetadata);
}

std::size_t BinauralMonitoringAudioProcessor::delayInSamples() const {
  return 0;
}

bool BinauralMonitoringAudioProcessor::configMatches(std::size_t sampleRate,
                                                     std::size_t blockSize) {
  if (bearConfig.get_sample_rate() != sampleRate) return false;
  if (bearConfig.get_period_size() != blockSize) return false;
  return true;
}

void BinauralMonitoringAudioProcessor::setListenerOrientation(float quatW,
                                                              float quatX,
                                                              float quatY,
                                                              float quatZ) {
  // X and Y need swapping and inverting for BEAR
  if (listenerQuats[0] != quatW) {
    listenerQuats[0] = quatW;
    listenerQuatsDirty = true;
  }
  if (listenerQuats[1] != -quatY) {
    listenerQuats[1] = -quatY;
    listenerQuatsDirty = true;
  }
  if (listenerQuats[2] != -quatX) {
    listenerQuats[2] = -quatX;
    listenerQuatsDirty = true;
  }
  if (listenerQuats[3] != quatZ) {
    listenerQuats[3] = quatZ;
    listenerQuatsDirty = true;
  }
}

bool BinauralMonitoringAudioProcessor::updateChannelCounts(std::size_t objChannels, std::size_t dsChannels, std::size_t hoaChannels)
{
  if(!bearRenderer) return false;

  if(bearConfig.get_num_objects_channels() == objChannels &&
     bearConfig.get_num_direct_speakers_channels() == dsChannels &&
     bearConfig.get_num_hoa_channels() == hoaChannels) {
    return true;
  }

  bearConfig.set_num_objects_channels(objChannels);
  bearConfig.set_num_direct_speakers_channels(dsChannels);
  bearConfig.set_num_hoa_channels(hoaChannels);

  if(isPlaying) {
    // Do dynamic update
    bearRenderer->set_config(bearConfig);
    return true; // TODO - this is a total assumption!

  } else {
    // Do immediate update
    bearRenderer->set_config_blocking(bearConfig);

  }
}

}  // namespace plugin
}  // namespace ear
