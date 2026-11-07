#include "stdafx.h"
#include "CtrlrSettings.h"

CtrlrSettings::CtrlrSettings (CtrlrManager &_owner) : Component ("Global Properties"), owner(_owner)
{
    //addAndMakeVisible (propertyPanel = new PropertyPanel());  
    //propertyPanel->setName ("propertyPanel");
 
    auto& idManager = owner.getIDManager();
    ValueTree managerTree = owner.getManagerTree();

    tabbedComponent = std::make_unique<TabbedComponent>(TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(tabbedComponent.get());

    tabbedComponent->setTabBarDepth(managerTree.getProperty(Ids::ctrlrTabBarDepth));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));

    // Fetch sections in their precise XML declaration order
    auto registeredSections = idManager.getRegisteredSections();
 
    // Fallback if no explicit <sections> tag is declared in CtrlrIDs.xml
    if (registeredSections.empty())
    {
        registeredSections = { {"Global", "Global"}, {"MIDI", "MIDI"}, {"GUI", "GUI"}, {"Debug", "Debug"}, {"Paths", "Paths"} };
    }

    // Temporary buckets storing components associated with each section key
    std::map<String, Array<PropertyComponent*>> propertiesBySection;

    // Scan all settings inside the Manager Tree
    for (int i = 0; i < managerTree.getNumProperties(); ++i)
    {
        Identifier propName = managerTree.getPropertyName(i);

        // Skip obsolete configurations
        if (propName == Identifier("ctrlrShutdownDelay") || propName == Identifier("ctrlrUseEditorWrapper"))
            continue;

        // Apply Standalone application visibility conditions
        if (!JUCEApplication::isStandaloneApp())
        {
            if (propName == Identifier("ctrlrAutoSave") || propName == Identifier("ctrlrAutoSaveInterval") ||
                propName == Identifier("ctrlrLookAndFeel") || propName == Identifier("ctrlrColourScheme"))
                continue;
        }

        PropertyComponent* propComp = idManager.createComponentForProperty(propName, managerTree, nullptr);
        if (propComp != nullptr)
        {
            String sectionName = idManager.getSectionForXmlId(propName);
            if (sectionName.isEmpty())
                sectionName = "Global";

            propertiesBySection[sectionName].add(propComp);
        }
    }

    auto tabColor = getLookAndFeel().findColour(DocumentWindow::backgroundColourId);
    auto scrollbarThickness = managerTree.getProperty(Ids::ctrlrScrollbarThickness);

    // Construct tabs sequentially following the registered layout order
    for (const auto& sectionInfo : registeredSections)
    {
        const String& sectionId = sectionInfo.first;
        const String& sectionTitle = sectionInfo.second;
        Array<PropertyComponent*>& fields = propertiesBySection[sectionId];

        // Only instantiate the tab if there are components visible inside it
        if (fields.size() > 0)
        {
            auto panel = std::make_unique<PropertyPanel>();

            // Use addProperties to prevent font scaling glitches
            panel->addProperties(fields);
            panel->getViewport().setScrollBarThickness(scrollbarThickness);

            // Transfer ownership to the persistent class lifecycle container
            sectionPanels[sectionId] = std::move(panel);

            // Append the component to the layout view
            tabbedComponent->addTab(sectionTitle, tabColor, sectionPanels[sectionId].get(), false);
        }
    }

    // Set stable proportions for a side-by-side preference setup view
    setSize(700, 540);

    // Activating the listener here at the end of the constructor.
    // ÀAt this tage, the GUI has finished to sync with the managerTree.
    owner.getManagerTree().addListener(this);
}

// Added v5.6.36.5 to only react when a Preference has been modified
void CtrlrSettings::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    settingsChanged = true;
}

void CtrlrSettings::lookAndFeelChanged() // Added v5.6.36.5
{
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f));
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId));
    repaint();
}

void CtrlrSettings::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (DocumentWindow::backgroundColourId)); // Added v5.6.31

    // Added v5.6.36.5 @dobo365
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
        {
            // This is for v3 where the color is forced. Should add an If..then..else checking v4 LnF
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));   // Set to Light grey
        }
        else
        {
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId));
        }
    }

}

void CtrlrSettings::resized()
{
    //propertyPanel->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
    tabbedComponent->setBounds(0, 0, getWidth(), getHeight());
}

CtrlrSettings::~CtrlrSettings()
{
    //deleteAndZero (propertyPanel);
    
    owner.getManagerTree().removeListener(this);

    // Memory release is automated safely through standard std::unique_ptr stack mechanics
    if (settingsChanged)
    {
        if (JUCEApplication::isStandaloneApp())
        {
            // Show Ok/Cancel dialog to confirm restart
            const auto callbackRestart = juce::ModalCallbackFunction::create([this](int resultBox) {
                //            if (resultBox == 0){nullptr;} // for Cancel
                if (resultBox == 1) { restart(); } // for OK
                });
            juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon, "PREFERENCES", "A restart is required to apply your changes.", "Restart now", "Maybe later", nullptr, callbackRestart);
        }
        else {
            // For VST/AU instances
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "PREFERENCES", "A restart is required to apply your changes."); // Added v5.6.31
        }
    }
    else {
        // Added 5.6.36.5 Don't disturb the user when no changes...
        //AlertWindow::showMessageBox (AlertWindow::InfoIcon, "CtrlrX Preferences", "No property changed...");
    }
}

void CtrlrSettings::restart()
{
    // Check if multiple instances are allowed
    auto runningInstance = JUCEApplication::getInstance();
    bool multiInstance = runningInstance->JUCEApplication::moreThanOneInstanceAllowed();
        
    if (multiInstance) {
        Logger::writeToLog("Multiple instances allowed.");
    }
    else {
        Logger::writeToLog("Multiple instances not allowed.");
    }
    
    String executablePath = File::getSpecialLocation(File::currentExecutableFile).getFullPathName();
    File exeFile = File(executablePath);
    
    if (exeFile.exists())
    {
        Logger::writeToLog("Executable file found");
        
        int result = 1; // Assume failure initially
        
        auto osType = SystemStats::getOperatingSystemType();
        Logger::writeToLog("OS type: " + String(osType));
        
        String osName = SystemStats::getOperatingSystemName();
        Logger::writeToLog("OS Name: " + String(osName));
        
        if ((SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {  // To test whether any version of OSX is running
            // For OSX & macOS
            Logger::writeToLog("Launching on macOS using ChildProcess.");
            ChildProcess childProcess;
            
            
            // Attempt to start the process with ChildProcess
            if (childProcess.start(exeFile.getFullPathName())) {
                Logger::writeToLog("ChildProcess start() successful.");
                result = 0;
            } else {
                Logger::writeToLog("ChildProcess start() failed.");

                // If ChildProcess fails, try startAsProcess()
                Logger::writeToLog("Trying startAsProcess().");
                result = exeFile.startAsProcess();
                if (result == 0) {
                    Logger::writeToLog("startAsProcess() successful.");
                } else {
                    Logger::writeToLog("startAsProcess() failed.");
                }
            }
        }
        
        else if (SystemStats::getOperatingSystemType() == SystemStats::Windows){
            // For Windows
            Logger::writeToLog("Launching on Windows using exeFile.startAsProcess().");
            result = exeFile.startAsProcess();
        }
        
        else{
            // Handle unsupported operating systems
            Logger::writeToLog("Launching on other operating system.");
            std::cerr << "Launching on other operating system." << std::endl;
            result = 1;
        }
                
        if (result == 1){
            // Handle error (e.g., log error message)
            std::cerr << "Error launching executable: " << result << std::endl;
            Logger::writeToLog("Error launching executable: " + String(result));
        }
        else
        {
            Logger::writeToLog("Executable launched successfully.");
        }
    }
    else{
        // Handle error: executable file not found
        std::cerr << "Executable file not found." << std::endl;
        Logger::writeToLog("Executable file not found.");
    }
        
    Logger::writeToLog("Sending quit request to current application.");
    JUCEApplication::getInstance()->systemRequestedQuit();
    Logger::writeToLog("Quit request sent. Waiting for 500ms.");
    Thread::sleep(500);
}
