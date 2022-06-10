#ifndef ADMCHANNEL_H
#define ADMCHANNEL_H

#include <memory>
#include <string>

namespace adm {
class AudioObject;
class AudioChannelFormat;
class AudioPackFormat;
class AudioTrackUid;
}

namespace admplug {

class ADMChannel {
public:
    ADMChannel(std::shared_ptr<adm::AudioObject const> object,
               std::shared_ptr<adm::AudioChannelFormat const> channelFormat,
               std::shared_ptr<adm::AudioPackFormat const> packFormat,
               std::shared_ptr<adm::AudioTrackUid const> uid);

    friend inline bool operator== (ADMChannel const& lhs, ADMChannel const & rhs) {
        return (lhs.trackUid() == rhs.trackUid() && lhs.channelFormat() == rhs.channelFormat() && lhs.packFormat() == rhs.packFormat() && lhs.object() == rhs.object());
    }

    std::shared_ptr<const adm::AudioTrackUid> trackUid() const;
    std::shared_ptr<const adm::AudioObject> object() const;
    std::shared_ptr<const adm::AudioChannelFormat> channelFormat() const;
    std::shared_ptr<const adm::AudioPackFormat> packFormat() const;
    std::string name() const;
    std::string speakerLabel() const;
    std::string formatId() const;
private:
    std::shared_ptr<adm::AudioObject const> admObject;
    std::shared_ptr<adm::AudioChannelFormat const> admChannelFormat;
    std::shared_ptr<adm::AudioPackFormat const> admPackFormat;
    std::shared_ptr<adm::AudioTrackUid const> uid;

};

}
#endif // ADMCHANNEL_H
