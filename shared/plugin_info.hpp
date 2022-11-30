#include <string>
#include <vector>
#include <memory>


namespace ear {
namespace plugin {

class ParameterInfo {
public:
    ParameterInfo(const char* name, const char* niceName, const char* desc) :
        name_{ name }, niceName_{ niceName }, desc_{ desc }
    {}

    virtual ~ParameterInfo() {}; //even virtual destructor makes a class polymorphic! (for dynamic_cast)

    const std::string name() {
        return name_;
    }

    const std::string niceName() {
        return niceName_;
    }

    const std::string description() {
        return desc_;
    }

protected:
    std::string name_;
    std::string niceName_;
    std::string desc_;
};

class ParameterInfoInt : public ParameterInfo {
public:
    ParameterInfoInt(const char* name, const char* niceName, const char* desc, int minVal, int maxVal, int defVal) :
        ParameterInfo(name, niceName, desc), minVal_{ minVal }, maxVal_{ maxVal }, defVal_{ defVal }
    {}

    const int minVal() {
        return minVal_;
    }

    const int maxVal() {
        return maxVal_;
    }

    const int defVal() {
        return defVal_;
    }

private:
    int minVal_ = 0;
    int maxVal_ = 0;
    int defVal_ = 0;
};

class ParameterInfoFloat : public ParameterInfo {
public:
    ParameterInfoFloat(const char* name, const char* niceName, const char* desc, float minVal, float maxVal, float defVal) :
        ParameterInfo(name, niceName, desc), minVal_{ minVal }, maxVal_{ maxVal }, defVal_{ defVal }
    {}

    const float minVal() {
        return minVal_;
    }

    const float maxVal() {
        return maxVal_;
    }

    const float defVal() {
        return defVal_;
    }

private:
    float minVal_ = 0.0f;
    float maxVal_ = 0.0f;
    float defVal_ = 0.0f;
};

class ParameterInfoBool : public ParameterInfo {
public:
    ParameterInfoBool(const char* name, const char* niceName, const char* desc, bool defVal) :
        ParameterInfo(name, niceName, desc), defVal_{ defVal }
    {}

    const bool defVal() {
        return defVal_;
    }

private:
    bool defVal_ = false;
};

class PluginInfo {
public:
    PluginInfo(const char* name) :
        name_{ name }
    {}

    const std::string name() {
        return name_;
    }

    int parameterIndex(const char* name) {
        std::string nameStr{ name };
        for(size_t i = 0; i < params_.size(); ++i) {
            if(params_[i]->name() == nameStr) {
                return i;
            }
        }
        return -1;
    }

    std::shared_ptr<ParameterInfo> parameter(int index) {
        return params_[index];
    }

    template<typename T>
    std::shared_ptr<T> parameter(int index) {
        return std::dynamic_pointer_cast<T>(params_[index]);
    }

    size_t parameterCount() {
        return params_.size();
    }

protected:
    template<typename T>
    std::shared_ptr<T> parameterInfo(const char* name) {
        auto index = parameterIndex(name);
        if(index < 0) return nullptr;
        return std::dynamic_pointer_cast<T>(params_[index]);
    }

    template<typename T>
    void addParameter(T param) {
        params_.push_back(std::make_shared<T>(param));
    }

private:
    std::vector<std::shared_ptr<ParameterInfo>> params_;
    std::string name_;

};

// Actual plugin definitions (as singletons)

class ObjectPluginInfo : public PluginInfo {
private:
    ObjectPluginInfo() : PluginInfo("EAR Object")
    {
        // Ordering sets expected order in plugin and hence parameter indices
        addParameter(ParameterInfoInt{
            "ROUTING", "Routing", "Receiving channel at Scene for this AudioObject",
            -1, 63, -1 });
        addParameter(ParameterInfoFloat{
            "GAIN", "Gain", "ADM Gain parameter for AudioBlockFormat",
            -100.f, 6.f, 0.f });
        addParameter(ParameterInfoFloat{
            "AZIMUTH", "Azimuth", "ADM Azimuth parameter for AudioBlockFormat",
            -180.f, 180.f, 0.f });
        addParameter(ParameterInfoFloat{
            "ELEVATION", "Elevation", "ADM Elevation parameter for AudioBlockFormat",
            -90.f, 90.f, 0.f });
        addParameter(ParameterInfoFloat{
            "DISTANCE", "Distance", "ADM Distance parameter for AudioBlockFormat",
            0.f, 1.f, 1.f });
        addParameter(ParameterInfoBool{
            "LINKSIZE", "Link Size", "Whether to use Size parameter, or individual Width, Height and Depth",
            false });
        addParameter(ParameterInfoFloat{
            "SIZE", "Size", "Value for ADM Width, Height and Depth parameters for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoFloat{
            "WIDTH", "Width", "ADM Width parameter for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoFloat{
            "HEIGHT", "Height", "ADM Height parameter for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoFloat{
            "DEPTH", "Depth", "ADM Depth parameter for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoFloat{
            "DIFFUSE", "Diffuse", "ADM Diffuse parameter for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoBool{
            "DIVERGENCE", "Divergence", "Whether ADM Divergence-related parameters should be used",
            false });
        addParameter(ParameterInfoFloat{
            "FACTOR", "Factor", "ADM ObjectDivergence parameter for AudioBlockFormat",
            0.f, 1.f, 0.f });
        addParameter(ParameterInfoFloat{
            "RANGE", "Range", "ADM AzimuthRange parameter for ObjectDivergence for AudioBlockFormat",
            0.f, 180.f, 45.f });
        addParameter(ParameterInfoBool{
            "BYPASS", "Bypass", "Used simply as a toggle when we want the host to mark the plugin state as dirty",
            false });
        addParameter(ParameterInfoBool{
            "USETRACKNAME", "Use Track Name", "Use the track name this plugin is on, or a custom name for this AudioObject",
            true });
        addParameter(ParameterInfoInt{
            "INPUTINSTANCEID", "Input Instance ID", "ID auto-set by REAPER extension to to uniquely identify this plugin instance",
            0, 65535, 0 });
    };

public:
    static ObjectPluginInfo& pluginInfo()
    {
        static ObjectPluginInfo singleton{};
        return singleton;
    }

    // Only public methods are specific methods for each parameter (rather than using a generic lookup)
    // Helps with IDE autocomplete and makes errors outside this file obvious at compile-time

    std::shared_ptr<ParameterInfoInt> routing() {
        return parameterInfo<ParameterInfoInt>("ROUTING");
    }

    std::shared_ptr<ParameterInfoFloat> gain() {
        return parameterInfo<ParameterInfoFloat>("GAIN");
    }

    std::shared_ptr<ParameterInfoFloat> azimuth() {
        return parameterInfo<ParameterInfoFloat>("AZIMUTH");
    }

    std::shared_ptr<ParameterInfoFloat> elevation() {
        return parameterInfo<ParameterInfoFloat>("ELEVATION");
    }

    std::shared_ptr<ParameterInfoFloat> distance() {
        return parameterInfo<ParameterInfoFloat>("DISTANCE");
    }

    std::shared_ptr<ParameterInfoBool> linkSize() {
        return parameterInfo<ParameterInfoBool>("LINKSIZE");
    }

    std::shared_ptr<ParameterInfoFloat> size() {
        return parameterInfo<ParameterInfoFloat>("SIZE");
    }

    std::shared_ptr<ParameterInfoFloat> width() {
        return parameterInfo<ParameterInfoFloat>("WIDTH");
    }

    std::shared_ptr<ParameterInfoFloat> height() {
        return parameterInfo<ParameterInfoFloat>("HEIGHT");
    }

    std::shared_ptr<ParameterInfoFloat> depth() {
        return parameterInfo<ParameterInfoFloat>("DEPTH");
    }

    std::shared_ptr<ParameterInfoFloat> diffuse() {
        return parameterInfo<ParameterInfoFloat>("DIFFUSE");
    }

    std::shared_ptr<ParameterInfoBool> divergence() {
        return parameterInfo<ParameterInfoBool>("DIVERGENCE");
    }

    std::shared_ptr<ParameterInfoFloat> factor() {
        return parameterInfo<ParameterInfoFloat>("FACTOR");
    }

    std::shared_ptr<ParameterInfoFloat> range() {
        return parameterInfo<ParameterInfoFloat>("RANGE");
    }

    std::shared_ptr<ParameterInfoBool> bypass() {
        return parameterInfo<ParameterInfoBool>("BYPASS");
    }

    std::shared_ptr<ParameterInfoBool> useTrackName() {
        return parameterInfo<ParameterInfoBool>("USETRACKNAME");
    }

    std::shared_ptr<ParameterInfoInt> inputInstanceId() {
        return parameterInfo<ParameterInfoInt>("INPUTINSTANCEID");
    }

};

}
}