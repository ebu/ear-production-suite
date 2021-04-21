#pragma once

#include "parameter.h"
#include <utility>
#include <unordered_map>

namespace PluginParameterRanges {

  static const std::unordered_map<AdmParameter, std::pair<double, double>> values {
    {AdmParameter::OBJECT_AZIMUTH,   { -180.0, 180.0}},
    {AdmParameter::OBJECT_DISTANCE,  {    0.0,   1.0}},
    {AdmParameter::OBJECT_ELEVATION, {  -90.0,  90.0}},
    {AdmParameter::OBJECT_GAIN,      { -100.0,   6.0}},
    {AdmParameter::OBJECT_HEIGHT,    {    0.0,  90.0}},
    {AdmParameter::OBJECT_WIDTH,     {    0.0, 360.0}},
    {AdmParameter::OBJECT_DEPTH,     {    0.0,   1.0}},
    {AdmParameter::OBJECT_DIFFUSE,   {    0.0,   1.0}},
    //{AdmParameter::OBJECT_DIVERGENCE, {0.0, 1.0}},
    //{AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE, {0.0, 180.0}},
  };
  
};

