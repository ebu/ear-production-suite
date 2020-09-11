#pragma once
#include <memory>
#include <functional>
#include <vector>
#include "automationpoint.h"

namespace admplug {
    class ReaperAPI;

    /**
    * @brief The ParameterValueMapping interface
    * ParameterValueMappings are responsible for converting from values extracted from adm parameters
    * to the range of values supported by an automation envelope, and vice versa
    */
    class ParameterValueMapping {
    public:
        virtual ~ParameterValueMapping() = default;
        AutomationPoint operator()(AutomationPoint point) const;
        AutomationPoint forwardMap(AutomationPoint point) const;
        AutomationPoint reverseMap(AutomationPoint point) const;
        virtual double forwardMap(double val) const = 0;
        virtual double reverseMap(double val) const = 0;
    };




    class FunctionalMapping : public ParameterValueMapping {
    public:
        FunctionalMapping();
        FunctionalMapping(std::function<double(double)> forwardMapper, std::function<double(double)> reverseMapper);
        using ParameterValueMapping::forwardMap;
        using ParameterValueMapping::reverseMap;
        double forwardMap(double val) const override;
        double reverseMap(double val) const override;
    private:
        std::function<double(double)> forwardMapper;
        std::function<double(double)> reverseMapper;
    };




    class CompositeMapping : public ParameterValueMapping {
    public:
        CompositeMapping() = default;
        CompositeMapping(std::vector<std::shared_ptr<ParameterValueMapping const>> mappings); // I presume with composites, the mappings are the same, we just called the reversemapper of each in reverse order through the vector.
        using ParameterValueMapping::forwardMap;
        using ParameterValueMapping::reverseMap;
        double forwardMap(double val) const override;
        double reverseMap(double val) const override;
        void addMapping(std::shared_ptr<ParameterValueMapping> mapping);
    private:
        std::vector<std::shared_ptr<ParameterValueMapping const>> mappings;
    };




    class Inversion : public ParameterValueMapping {
        using ParameterValueMapping::forwardMap;
        using ParameterValueMapping::reverseMap;
        double forwardMap(double val) const override;
        double reverseMap(double val) const override;
    };

    class LinearToDb : public ParameterValueMapping {
    public:
        using ParameterValueMapping::forwardMap;
        using ParameterValueMapping::reverseMap;
        double forwardMap(double val) const override;
        double reverseMap(double val) const override;
    };



    std::shared_ptr<CompositeMapping> getCombinedMapping(std::initializer_list<std::shared_ptr<ParameterValueMapping const>> list);

    /**
    * @brief The ParameterRange class
    * The range of values a parameter can take and mappings useful for conforming to that range.
    */
    class ParameterRange {
    public:
        ParameterRange(double min, double max);
        std::shared_ptr<ParameterValueMapping const> modulus() const;
        std::shared_ptr<ParameterValueMapping const> normaliser() const;
        std::shared_ptr<ParameterValueMapping const> clipper() const;
    private:
        double size;
        double min;
        double max;
    };

    namespace map {
        std::shared_ptr<ParameterValueMapping const> sequence(std::initializer_list<std::shared_ptr<ParameterValueMapping const>> mappings);
        std::shared_ptr<ParameterValueMapping const> normalise(ParameterRange const& range);
        std::shared_ptr<ParameterValueMapping const> clip();
        std::shared_ptr<ParameterValueMapping const> clip(ParameterRange const& range);
        std::shared_ptr<ParameterValueMapping const> wrap(ParameterRange const& range);
        std::shared_ptr<ParameterValueMapping const> invert();
        std::shared_ptr<ParameterValueMapping const> linearToDb(ParameterRange const& range);


    }

}
