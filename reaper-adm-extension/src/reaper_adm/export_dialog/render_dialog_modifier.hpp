#ifndef RENDER_DIALOG_MODIFIER_HPP
#define RENDER_DIALOG_MODIFIER_HPP
#ifndef _WIN32
#include <WDL/swell/swell-types.h>
#else
#include <windows.h>
#endif
#include <exportaction_admsourcescontainer.h>
#include <optional>
#include <string>

namespace admplug {
class ReaperAPI;
class RenderDialogModifier {

public:
  explicit RenderDialogModifier(std::shared_ptr<ReaperAPI> api) : reaperApi{std::move(api)} {}
  void startPreparingRenderControls(HWND hwndDlg);
  std::string takeOverRenderControls(
      HWND hwndDlg, std::shared_ptr<AdmExportHandler> const &admExportHandler);
  BOOL prepareRenderControl_pass1(
      HWND hwnd, std::shared_ptr<AdmExportHandler> const &adm_export_handler);
  BOOL prepareRenderControl_pass2(HWND hwnd, LPARAM lParam);
  void restoreControls();

private:
  enum class Control {
    UNKNOWN,
    TEXT, // Note: On OSX this can be editable text or a label. Mirrored this
          // behaviour for Windows.
    BUTTON, // Note: This includes radios and checkboxes.
    COMBOBOX,
    EDITABLECOMBO
  };
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
    bool secondPassLastState{false};
    bool monoToMonoLastState{false};
    bool multiToMultiLastState{false};
    bool preserveSampleRateLastState{false};
    bool normalizeCheckboxLastState{false};
  };

  ControlsState controls;
  std::shared_ptr<ReaperAPI> reaperApi;

      Control
      getControlType(HWND hwnd);
  HWND getComboBoxEdit(HWND hwnd);
  std::string getComboBoxItemText(HWND hwnd, int idx = 0);
  std::string getWindowText(HWND hwnd);
  LRESULT selectInComboBox(HWND hwnd, std::string text);
  bool getCheckboxState(HWND hwnd);
  void setCheckboxState(HWND hwnd, bool state);

  bool startedPrepareDialogControls{false};
};
} // namespace admplug

#endif // RENDER_DIALOG_MODIFIER_HPP
