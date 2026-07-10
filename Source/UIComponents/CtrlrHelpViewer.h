/*
  ==============================================================================

    CtrlrHelpViewer.h
    Created: 5 May 2026 12:29:44pm
    Author:  dobo365 DB

  ==============================================================================
*/
#ifndef __JUCER_HEADER_CTRLRHELPVIEWER__
#define __JUCER_HEADER_CTRLRHELPVIEWER__

//[Headers]     -- You can add your own extra header files here --
#include "CtrlrMacros.h"
#include "CtrlrLog.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrWindowManagers/CtrlrManagerWindowManager.h"
#include "Misc/BarelyML - Sunny Synths/BarelyML - Sunny Synths.h"
#include <map>
#include <memory>
class CtrlrManager;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class CtrlrHelpViewer : public CtrlrChildWindowContent
{
public:
    //==============================================================================
    CtrlrHelpViewer(CtrlrManager& _owner);
    ~CtrlrHelpViewer();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
 
    enum ToolbarItems
    {
        _none,
        clearHelp
    };
    String getContentName()     { return ("Help viewer"); }
    uint8 getType()             { return (CtrlrManagerWindowManager::HelpViewer); }
    StringArray getMenuBarNames();
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);
    void loadMarkdownFile(const File helpFile);
    // Allow the custom URL handler to read private variables (bmlDisplay)
    friend struct HelpViewerURLHandler;
    //[/UserMethods]

    void paint(Graphics& g);
    void resized();

    //==============================================================================
 
    // Map of CtrlrIDs <-> files discovered in Help directories (populated when building TOC)
    std::map<String, File> helpFileMap;

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    CtrlrManager &owner;
    BarelyMLDisplay bmlDisplay; // The BarelyML component
    Viewport viewPort;

    // Keep URL handler alive while this viewer exists
    std::unique_ptr<BarelyMLDisplay::URLHandler> urlHandlerInstance;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    CtrlrHelpViewer(const CtrlrHelpViewer&);
    const CtrlrHelpViewer& operator= (const CtrlrHelpViewer&);
};


#endif   // __JUCER_HEADER_CTRLRHELPVIEWER__
