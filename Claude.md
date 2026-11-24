# CtrlrX - Sunny Synths (JUCE 6.0.8 / macOS)

## Project Architecture & Toolchain
- **Framework:** JUCE 6.0.8 (managed via `.jucer` project file, NOT CMake)
- **Target OS / IDE:** macOS 14.8.7, Xcode 15.4
- **Formats:** VST3, AU (Component), Standalone

## Critical Projucer Rules
- **NEVER manually edit `Builds/MacOSX/*.xcodeproj/project.pbxproj`.** Projucer overwrites manual Xcode edits.
- **Adding/Removing Source Files:** Whenever you create, rename, or delete `.cpp` or `.h` files inside `Source/`, you MUST register them in the `.jucer` XML under `<MAINGROUP>` (assigning unique 9-character alphanumeric IDs) AND regenerate the Xcode project using Projucer CLI:
  `"/Applications/Projucer.app/Contents/MacOS/Projucer" --resave "CtrlrX - Sunny Synths [Without AAX] - Apple Silicon.jucer"`
  *(If Projucer is not at that exact path, locate `Projucer.app` or ask the user to hit Cmd+S in Projucer before building.)*

## Build & Verification Commands
- **Build VST3 (Debug) via Terminal:**
  `xcodebuild -project "Builds/MacOSX/CtrlrX - Sunny Synths.xcodeproj" -scheme "CtrlrX - Sunny Synths - VST3" -configuration Debug build`
  - **Build Shared Code (Release):**
  `xcodebuild -project "Builds/MacOSX/CtrlrX - Sunny Synths.xcodeproj" -scheme "CtrlrX - Sunny Synths - Shared Code" -configuration Release build`
  - **Build Standalone Plugin (Release):**
  `xcodebuild -project "Builds/MacOSX/CtrlrX - Sunny Synths.xcodeproj" -scheme "CtrlrX - Sunny Synths - Standalone Plugin" -configuration Release build`
- **Build All Targets (Release):**
  `xcodebuild -project "Builds/MacOSX/CtrlrX - Sunny Synths.xcodeproj" -scheme "CtrlrX - Sunny Synths - All" -configuration Release build`
- Always run a build check after modifying C++ headers or DSP logic to verify zero compiler or linker errors.

## JUCE 6 & Real-Time Audio Guidelines
- Keep `processBlock()` strictly real-time safe: no memory allocations (`new`, `malloc`, `std::vector::push_back`), no locks/mutexes, no file I/O, and no `MessageManager` calls on the audio thread.
- Use `juce::AudioProcessorValueTreeState` (APVTS) and atomic parameter pointers for thread-safe UI-to-DSP communication.
- Stick strictly to **JUCE 6.1.5 APIs** (do not use JUCE 7 or JUCE 8 classes such as `juce::WebBrowserComponent` options or modern `ParameterAttachment` overloads introduced after 6.1.5).

