#include "binaural_monitoring_audio_processor.hpp"
#include <functional>

#include <iostream>

#define DEFAULT_TENSORFILE_NAME "default.tf"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {

BinauralMonitoringAudioProcessor::BinauralMonitoringAudioProcessor(
  std::size_t objChannels, std::size_t dsChannels, std::size_t hoaChannels, std::size_t sampleRate, std::size_t blockSize, std::string dataFilePath)
  /* : blockAdapter_(
      blockSize, dsChannels + hoaChannels + objChannels, 2,
      std::bind(&BinauralMonitoringAudioProcessor::doBlockedProcess, this, _1, _2)),*/
{
  assert(hoaChannels == 0); // Dev reminder that HOA isn't implemented (it's missing in a whole chunk of the EPS anyway so no point implementing at this early stage)

  framesProcessed = 0;
  metadataRtime = bear::Time{ framesProcessed, sampleRate };
  metadataDuration = bear::Time{ blockSize, sampleRate };

  reusableZeroedChannel = std::vector<float>(blockSize, 0.0);
  bearOutputBuffers_RawPointers = std::vector<float*>(2, nullptr);
  bearObjectInputBuffers_RawPointers = std::vector<float*>(objChannels, reusableZeroedChannel.data());
  bearDirectSpeakersInputBuffers_RawPointers = std::vector<float*>(dsChannels, reusableZeroedChannel.data());
  bearHoaInputBuffers_RawPointers = std::vector<float*>(hoaChannels, reusableZeroedChannel.data());

  objChannelMappings.reserve(objChannels);
  dsChannelMappings.reserve(dsChannels);
  hoaChannelMappings.reserve(hoaChannels);

  bearConfig.set_period_size(blockSize);
  bearConfig.set_sample_rate(sampleRate);
  bearConfig.set_num_direct_speakers_channels(dsChannels);
  bearConfig.set_num_objects_channels(objChannels);
  bearConfig.set_num_hoa_channels(hoaChannels);
  bearConfig.set_data_path(dataFilePath);
  bearConfig.set_fft_implementation("default");

  try {
    bearRenderer = std::make_shared<bear::Renderer>(bearConfig);
    try {
      bearRenderer->set_listener(bearListener);
    } catch(std::exception &e) {
      assert(false);
    }
  } catch(std::exception &e) {
    assert(false);
  }

}


void BinauralMonitoringAudioProcessor::doProcess(float ** channelPointers, size_t maxChannels)
{
  // Set buffer pointers
  size_t tdLimit = std::min(objChannelMappings.size(), bearConfig.get_num_objects_channels());
  for(size_t tdChannel = 0; tdChannel < tdLimit; tdChannel++) {
    if(objChannelMappings[tdChannel] < maxChannels) {
      bearObjectInputBuffers_RawPointers[tdChannel] = channelPointers[objChannelMappings[tdChannel]];
    }
  }

  tdLimit = std::min(dsChannelMappings.size(), bearConfig.get_num_direct_speakers_channels());
  for(size_t tdChannel = 0; tdChannel < dsChannelMappings.size(); tdChannel++) {
    if(dsChannelMappings[tdChannel] < maxChannels) {
      bearDirectSpeakersInputBuffers_RawPointers[tdChannel] = channelPointers[dsChannelMappings[tdChannel]];
    }
  }

  tdLimit = std::min(hoaChannelMappings.size(), bearConfig.get_num_hoa_channels());
  for(size_t tdChannel = 0; tdChannel < hoaChannelMappings.size(); tdChannel++) {
    if(hoaChannelMappings[tdChannel] < maxChannels) {
      bearHoaInputBuffers_RawPointers[tdChannel] = channelPointers[hoaChannelMappings[tdChannel]];
    }
  }

  // TODO: Output buffer - this might work???
  bearOutputBuffers_RawPointers[0] = channelPointers[0];
  bearOutputBuffers_RawPointers[1] = channelPointers[1];

  // Process
  bearRenderer->process(bearObjectInputBuffers_RawPointers.data(),
                        bearDirectSpeakersInputBuffers_RawPointers.data(),
                        bearHoaInputBuffers_RawPointers.data(),
                        bearOutputBuffers_RawPointers.data());

  // Prepare for next block
  framesProcessed += bearConfig.get_period_size();
  metadataRtime.assign(framesProcessed, bearConfig.get_sample_rate());

  objChannelMappings.clear();
  dsChannelMappings.clear();
  hoaChannelMappings.clear();

  std::fill(bearObjectInputBuffers_RawPointers.begin(), bearObjectInputBuffers_RawPointers.end(), reusableZeroedChannel.data());
  std::fill(bearDirectSpeakersInputBuffers_RawPointers.begin(), bearDirectSpeakersInputBuffers_RawPointers.end(), reusableZeroedChannel.data());
  std::fill(bearHoaInputBuffers_RawPointers.begin(), bearHoaInputBuffers_RawPointers.end(), reusableZeroedChannel.data());
}

bool BinauralMonitoringAudioProcessor::pushBearMetadata(size_t channelNum, ear::ObjectsTypeMetadata * metadata)
{
  bear::ObjectsInput bearMetadata;
  bearMetadata.rtime = metadataRtime;
  bearMetadata.duration = metadataDuration;
  bearMetadata.type_metadata = *metadata;
  objChannelMappings.push_back(channelNum);
  return bearRenderer->add_objects_block(objChannelMappings.size() - 1, bearMetadata);
}

bool BinauralMonitoringAudioProcessor::pushBearMetadata(size_t channelNum, ear::DirectSpeakersTypeMetadata * metadata)
{
  bear::DirectSpeakersInput bearMetadata;
  bearMetadata.rtime = metadataRtime;
  bearMetadata.duration = metadataDuration;
  bearMetadata.type_metadata = *metadata;
  dsChannelMappings.push_back(channelNum);
  return bearRenderer->add_direct_speakers_block(dsChannelMappings.size() - 1, bearMetadata);
}

std::size_t BinauralMonitoringAudioProcessor::delayInSamples() const {
  return 0;
}

bool BinauralMonitoringAudioProcessor::configMatches(std::size_t objChannels, std::size_t dsChannels, std::size_t hoaChannels, std::size_t sampleRate, std::size_t blockSize)
{
  if(bearConfig.get_sample_rate() != sampleRate) return false;
  if(bearConfig.get_period_size() != blockSize) return false;
  if(bearConfig.get_num_objects_channels() != objChannels) return false;
  if(bearConfig.get_num_direct_speakers_channels() != dsChannels) return false;
  if(bearConfig.get_num_hoa_channels() != hoaChannels) return false;
  return true;
}

bool BinauralMonitoringAudioProcessor::configSupports(std::size_t objChannels, std::size_t dsChannels, std::size_t hoaChannels, std::size_t sampleRate, std::size_t blockSize)
{
  if(bearConfig.get_sample_rate() != sampleRate) return false;
  if(bearConfig.get_period_size() != blockSize) return false;
  if(bearConfig.get_num_objects_channels() < objChannels) return false;
  if(bearConfig.get_num_direct_speakers_channels() < dsChannels) return false;
  if(bearConfig.get_num_hoa_channels() < hoaChannels) return false;
  return true;
}



}  // namespace plugin
}  // namespace ear
