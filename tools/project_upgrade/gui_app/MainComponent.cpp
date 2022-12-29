#include "MainComponent.h"
#include "../upgrade.h"
#include <algorithm>
#include <fstream>
#include <components/version_label.hpp>
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

MainComponent::MainComponent()
{
    addAndMakeVisible(header);
    configureVersionLabel(versionLabel);
    addAndMakeVisible(versionLabel);
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (Graphics& g)
{
    g.fillAll (EarColours::Background);
    auto area = getLocalBounds();

    auto topHalf = area.removeFromTop(getHeight() / 2);
    topHalf.removeFromTop(30); // margin
    g.setColour (EarColours::Heading);
    g.setFont (EarFontsSingleton::instance().HeroHeading);
    g.drawText ("Project Upgrade Utility", topHalf.removeFromTop(topHalf.getHeight() / 2), Justification::centredBottom, true);
    g.setColour (EarColours::Label);
    g.setFont (EarFontsSingleton::instance().Label);
    topHalf.removeFromTop(10); // margin
    g.drawText ("Updates REAPER projects to use the latest version of the EAR Production Suite", topHalf, Justification::centredTop, true);

    area.removeFromBottom(50); // version label space
    g.setColour (EarColours::Label);
    g.setFont (EarFontsSingleton::instance().Description.italicised());
    if(processing) {
        g.drawText("Please wait. Processing...", area, Justification::centred, true);
    } else {
        g.drawText("Drag & Drop REAPER project files (.RPP) here...", area, Justification::centred, true);
    }
}

void MainComponent::resized()
{
    header.setBounds(getLocalBounds().removeFromTop(50));
    versionLabel.setBounds(getLocalBounds().removeFromBottom(30));
}

namespace {
  bool hasProjectFileExtension(juce::String const& fileName) {
    return fileName.endsWithIgnoreCase(".rpp") || fileName.endsWithIgnoreCase(".rpp-bak");
  }

  juce::String getOutputFileName(juce::String const& fileName) {
    auto dotPos = fileName.lastIndexOf(".");
    juce::String outFilename;
    int suffixNum = 1;
    do {
      outFilename = fileName.substring(0, dotPos);
      outFilename += "-conv";
      if(suffixNum > 1) {
        outFilename += "(";
        outFilename += suffixNum;
        outFilename += ")";
      }
      suffixNum++;
      outFilename += fileName.substring(dotPos);
    } while(juce::File(outFilename).exists());
    return outFilename;
  }

  struct ConversionStats {
    int fileCount;        // Number of input files;
    int notAProjectFile;  // Count of files which dont have correct extension
    int success;          // Count of files successfully converted
    int failure;          // Count of files failed conversion
    int noChangesNeeded;  // Count of files not requiring conversion
    int lastFileChanges;  // Number of changes in the last successfully converted file
  };

  ConversionStats doConversion(const StringArray& files) {
    ConversionStats stats = {};
    stats.fileCount = files.size();
    for(auto& file : files) {
      if(hasProjectFileExtension(file)) {
        bool cleanupRequired = false;
        auto outFilename = getOutputFileName(file);
        auto ifs = std::ifstream{file.toStdString().c_str()};
        auto ofs = std::ofstream{outFilename.toStdString().c_str()};
        if(ifs && ofs) {
          stats.lastFileChanges = upgrade::upgrade(ifs, ofs);
          if(stats.lastFileChanges == 0) {
            stats.noChangesNeeded++;
            cleanupRequired = true;
          } else {
            stats.success++;
          }
        } else {
          stats.failure++;
          cleanupRequired = true;
        }
        if(cleanupRequired) {
          // Close file stream and delete output file
          if(ofs.is_open()) ofs.close();
          juce::File(outFilename).deleteFile();
        }
      } else {
        stats.notAProjectFile++;
      }
    }
    return stats;
  }

  juce::String getConversionReport(ConversionStats const& stats) {
    juce::String msg;

    if(stats.fileCount == 1) {
      if(stats.success > 0) {
        msg = "File upgraded successfully.\n";
        msg += juce::String(stats.lastFileChanges);
        msg += " changes made.\n     (saved with '-conv' appended to filename).";
      } else if(stats.noChangesNeeded > 0) {
        msg = "File did not require upgrade.";
      } else if(stats.failure > 0) {
        msg = "File upgrade failed!";
      }

    } else {
      msg = juce::String(stats.success);

      if(stats.success == 0) {
        msg += " files upgraded successfully.";
      } else if(stats.success == 1) {
        msg += " file upgraded successfully\n     (saved with '-conv' appended to filename).";
      } else {
        msg += " files upgraded successfully\n     (saved with '-conv' appended to filenames).";
      }

      if(stats.noChangesNeeded > 0) {
        msg += "\n\n";
        msg += juce::String(stats.noChangesNeeded);
        if(stats.noChangesNeeded == 1) {
          msg += " file did not require upgrade.";
        } else {
          msg += " files did not require upgrade.";
        }
      }

      if(stats.notAProjectFile > 0) {
        msg += "\n\n";
        msg += juce::String(stats.notAProjectFile);
        if(stats.notAProjectFile == 1) {
          msg += " file was not processed as it did not have a .RPP or .RPP-BAK extension.";
        } else {
          msg += " files were not processed as they did not have a .RPP or .RPP-BAK extension.";
        }
      }

      if(stats.failure > 0) {
        msg += "\n\nUpgrade failed on ";
        msg += juce::String(stats.failure);
        if(stats.failure == 1) {
          msg += " file!";
        } else {
          msg += " files!";
        }
      }
    }
    return msg;
  }
}

bool MainComponent::isInterestedInFileDrag(const StringArray & files)
{
  return std::any_of(files.begin(), files.end(), hasProjectFileExtension);
}

void MainComponent::filesDropped(const StringArray & files, int x, int y)
{
    processing = true;
    repaint();

    auto stats = doConversion(files);
    auto msg = getConversionReport(stats);

    processing = false;
    repaint();

    NativeMessageBox::showMessageBox(
        stats.failure > 0? AlertWindow::AlertIconType::WarningIcon : AlertWindow::AlertIconType::InfoIcon,
        "Project Upgrade Results", msg, this);
}
