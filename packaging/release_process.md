# Publishing a New Release

### A Note on Versioning
If you want to release a version which does not trigger the update notification for users, it is possible to just apply a non-numeric suffix to the current version. E.g, if `v1.1.0` is the currently released version, then publishing `v1.1.0b` will not notify users of `v1.1.0` that a new version is available. Users of versions prior to `v1.1.0` *will* receive the notification, but only if they have not previously been notified that either `v1.1.0` or `v1.1.0b` was available. 

Please refer to the `v1.1.0b` release for an example of how the various steps below should be handled when using version suffixes.

## Release Process

- Merge all feature branches in to `main`

- Close all associated Pull Requests and related Issues, and delete any branches no longer required.

- Check and update copyright years in `./COPYRIGHT.txt` and `./packaging/LICENSE.md` 

- If your changes introduce new files to the package, ensure you have updated `./packaging/install_list.xml` and `./packaging/uninstall_list.xml`

- Update version in `./CMakeLists.txt` 
  - This will be set as something like `set(EPS_VERSION_BASE 1.2.0)` (for v1.2.0) near the top of the file. Note that this variable must only be numeric, so do not apply any non-numeric suffixes here.

- Update version in `./vcpkg.json` 
  - Set the `"version"` field. E.g, `"version": "1.2.0",`. Again, do not apply any non-numeric suffixes here.

- Update `./CHANGELOG.md`
  - Most recent versions to top
  - Follow a similar style to existing entries
  - Ensure relevant Pull Requests and Issues are linked to. 

- Update `./packaging/README.md.in`
  - Version number in heading. E.g, `# EAR Production Suite (v1.1.0b release)`
  - `## Release Notes` section containing a summary of changes, the purpose of the release, and anything that should be pointed out to the user (e.g, change in minimum REAPER version, etc). Do not just copy and paste the change log as this is pulled in automatically later in the document.
  - `### Known Issues` section. This might stay as it is, or might require modifying depending on fixes within the release. Also point the user to the GitHub Issues too.

- Commit all, and push

- Tag the commit with the version
  - This is where you can use version suffixes if needs be. The software will pull version information from the git tag and show this in the plugins, setup application and REAPER menu. It only uses the numeric bits of this tag for version comparison (hence why suffixes do not trigger update notifications.)
  - The format for the tag is `v1.2.0-EPS`, `v1.1.0b-EPS`, `v0.8.0-EPS-beta`, etc. I'm not sure why we put "-EPS" in there but it's always been in there so we've just been staying consistent.
  - The tag message is generally just, for example, `v1.2.0 release`
  - e.g, `git tag -a v1.2.0-EPS -m "v1.2.0 release"` followed by `git push origin v1.2.0-EPS` to push the tag to the remote.

- Wait for CI to build from this tag
  - Ensure you use this tagged version for the release otherwise the software will be labelled incorrectly and the update checker will not have the correct version values for comparison!

- Download the MacOS "universal" artefact from the build and Sign it, Notorise it, and generate a DMG.
  - There is a Python script in `./packaging/codesign/codesign.py` to automate most of this as there are several bits to sign and put back together before generating a DMG.
  - There are example steps for running the script in `./packaging/codesign/example-steps.txt`
  - The naming convention for the DMG is, for example, `EPS_1_2_0_macos_universal.dmg`, `EPS_1_1_0b_macos_universal.dmg`, etc.

- Download the Linux and Windows (release) artefacts from the build
  - Rename them using the convention, e.g, `EPS_1_2_0_linux.zip`, `EPS_1_1_0b_linux.zip`, `EPS_1_2_0_windows.zip`, `EPS_1_1_0b_windows.zip`, etc.

- Download `pdfs` artefacts from the build
  - Extract `README.pdf` and `LICENSE.PDF` from this zip to somewhere local

- On GitHub, create a release from the tag
  - For the title, use something such as "EAR Production Suite v1.1.0b"
  - For the description, the "Release Notes" and "Known Issues" from `./packaging/README.md.in`, followed by the relevant section of `./CHANGELOG.md` for this release is normally sufficient.
  - Upload `README.pdf` and `LICENSE.PDF` as release assets
  - Upload the signed MacOS DMG as release asset
  - Also upload the correctly-named Windows zip and Linux zip as release assets

- Update website
  - The website is stored in the `gh-pages` branch of the repository. 
  - The data used to set link addresses and page text etc is stored in `./docs/version_info.json` - this should be the only file you need to change to update the website. This file is also requested and parsed by existing installations of the EPS for the update check functionality. 
  - The fields within this file are fairly self-explanatory and usage examples can be found in the commit history. A quick reference;
    - `"version"` - STRING; complete version, can include suffix. E.g, `"v1.2.0"`, `"v1.1.0b"`
    - `"version_major"`, `"version_minor"`, `"version_revision"` - INTEGER; breakdown of version numerals, used for version comparison for update check.
    - `"version_suffix"` - STRING; not used as part of version comparison for update check. Set as empty string (i.e, `""`) if not required!
    - `"version_date"` - STRING; nicely formatted date string for use on main page, e.g, `"20th March 2024"`
    - `"version_day"`, `"version_month"`, `"version_year"` - INTEGER; day, month and year. Year should be 4 figures.
    - `"download_macos_universal_url"`, `"download_windows_url"`, `"download_linux_url"` - STRING; download URLs of release assets, usually along the lines of `"https://github.com/ebu/ear-production-suite/releases/download/v1.1.0b-EPS/EPS_1_1_0b_macos_universal.dmg"`
    - `"readme_url"`, `"license_url"` - STRING; download URLs of release docs, usually along the lines of `"https://github.com/ebu/ear-production-suite/releases/download/v1.1.0b-EPS/LICENSE.pdf"`
    - `"license_type"`, `"license_type_long"` - STRING; license type in short form and long form to populate page text