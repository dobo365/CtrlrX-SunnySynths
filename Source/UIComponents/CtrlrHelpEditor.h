/*
  ==============================================================================

    CtrlrHelpEditor.h
    Created: 1 May 2026 12:29:44pm
    Author:  dobo365 DB

  ==============================================================================
*/
#ifndef __JUCER_HEADER_CTRLRHELPEDITOR__
#define __JUCER_HEADER_CTRLRHELPEDITOR__

//[Headers]     -- You can add your own extra header files here --
#include "CtrlrMacros.h"
#include "CtrlrLog.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContent.h"
#include "CtrlrWindowManagers/CtrlrManagerWindowManager.h"
#include "Misc/BarelyML - Sunny Synths/BarelyML - Sunny Synths.h"
class CtrlrManager;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class CtrlrHelpEditor : public CtrlrChildWindowContent,
                        public TextEditor::Listener,
                        public BarelyMLDisplay::URLHandler
{
public:
    //==============================================================================
    CtrlrHelpEditor(CtrlrManager& _owner);
    ~CtrlrHelpEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
 
    String getContentName()     { return ("Help editor"); }
    uint8 getType()             { return (CtrlrManagerWindowManager::HelpEditor); }
    StringArray getMenuBarNames();
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);

	// Juce 6 functions for TextEditor::Listener
    void textEditorTextChanged (TextEditor& editor) override;
    void textEditorReturnKeyPressed(TextEditor&) override {}
    void textEditorEscapeKeyPressed(TextEditor&) override {}
    void textEditorFocusLost(TextEditor&) override {}

    bool handleURL(juce::String url) override;
    
    void wrapSelectedTextInTags (const String& openTag, const String& closeTag);
    //[/UserMethods]

    void paint(Graphics& g);
    void resized();
    void lookAndFeelChanged();

    //==============================================================================

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    CtrlrManager &owner;
    BarelyMLDisplay bmlDisplay; // The BarelyML component
    TextEditor bmlText;
    void generateTableOfContents();
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    CtrlrHelpEditor(const CtrlrHelpEditor&);
    const CtrlrHelpEditor& operator= (const CtrlrHelpEditor&);
};

#endif   // __JUCER_HEADER_CTRLRHELPEDITOR__
