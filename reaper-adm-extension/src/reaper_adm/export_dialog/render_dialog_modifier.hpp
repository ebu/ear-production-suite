//
// Created by Richard Bailey on 16/05/2025.
//

#ifndef RENDER_DIALOG_MODIFIER_HPP
#define RENDER_DIALOG_MODIFIER_HPP


class RenderDialogModifier {

public:
struct ControlsState {
  std::optional<HWND> boundsControlHwnd{};
  std::optional<HWND> sourceControlHwnd{};
  std::optional<HWND> presetsControlHwnd{};
  std::optional<HWND> sampleRateControlHwnd{};
  std::optional<HWND> channelsControlHwnd{};
  std::optional<HWND> channelsLabelHwnd{};
  std::optional<HWND> secondPassControlHwnd{};
  std::optional<HWND> normalizeCheckboxControlHwnd{};
  std::optional<HWND> normalizeControlHwnd{};
  std::optional<HWND> resampleModeControlHwnd{};
  std::optional<HWND> monoToMonoControlHwnd{};
  std::optional<HWND> multiToMultiControlHwnd{};
  std::optional<HWND> preserveSampleRateControlHwnd{};
  bool sampleRateControlSetError{false};
  bool channelsControlSetError{false};
  bool normalisationOptionsEnabled{false};

  std::string sampleRateLastOption{};
  std::string channelsLastOption{};
  bool secondPassLastState{ false };
  bool monoToMonoLastState{ false };
  bool multiToMultiLastState{ false };
  bool preserveSampleRateLastState{ false };
  bool normalizeCheckboxLastState{ false };
};


};



#endif //RENDER_DIALOG_MODIFIER_HPP
