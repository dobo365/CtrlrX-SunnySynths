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

    enum HelpMode { propertiesMode, offlineMode, onlineMode };

    // One place the viewer can display (and come back to with Back/Forward)
    struct HelpLocation
    {
        enum Kind { propertyIndex, localFile, onlineUrl };

        Kind   kind = propertyIndex;
        String value;        // "documented" | "all" (propertyIndex), full path (localFile), GitHub URL (onlineUrl)
        String anchor;       // Optional heading anchor to scroll to
        int    scrollY = -1; // Scroll position saved when leaving the page
    };

    String getContentName()     { return ("Help viewer"); }
    uint8 getType()             { return (CtrlrManagerWindowManager::HelpViewer); }
    StringArray getMenuBarNames();
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);
    void loadMarkdownFile(const File helpFile);
    
    void showPropertiesHelp();
    void showOfflineHelp();
    void showOnlineHelp();

    void navigateTo (HelpLocation location, bool addToHistory);
    bool handleOfflineLink (const String& url);
    bool handleOnlineLink (const String& url);
    
    // Allow the custom URL handler to read private variables (bmlDisplay)
    friend struct HelpViewerURLHandler;
    friend struct HelpImageSource;
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
    std::unique_ptr<BarelyMLDisplay::FileSource> imageSourceInstance;
    
    enum MenuIds
    {
        miBack = 100, miForward, miHome, miReload, miOpenInBrowser, miCopyUrl, miRevealFile,
        miPropsDocumented = 200, miPropsAll
    };

    HelpMode currentMode = propertiesMode;
    Array<HelpLocation> history;
    int historyPos = -1;
    File currentFile;              // Local file currently displayed (offline / property help pages)
    String pendingAnchor;          // Applied once the content is displayed
    int pendingScrollY = -1;
    int navigationId = 0;          // Used later to discard stale online downloads
    std::map<String, String> onlineCache;   // GitHub page URL -> downloaded markdown (current session only)
    std::map<String, MemoryBlock> onlineImageCache;   // Raw URL of an image -> downloaded data (empty block = download failed)

    static String getImageRawUrl (const String& pageUrl, const String& imageAddress);
    static StringArray extractImageAddresses (const String& markdown);
    void showOnlineMarkdown (int requestId, const String& url, const String& markdown);
    
    static File getHelpBaseDir();
    static File findOfflineManualHome();
    static bool isMarkdownFile (const File& f);
    static String slugify (const String& text);
    static bool isInternalHelpUrl (const String& url);
    static String toRawUrl (const String& githubBlobUrl);
    static String resolveRelativeUrl (const String& baseUrl, const String& link);
    
    void openMode (HelpMode newMode);
    HelpLocation getHomeLocation() const;
    void display (const HelpLocation& location);
    void goBack();
    void goForward();
    int getCurrentScrollY();
    void applyPendingScroll();
    bool scrollToAnchor (const String& fragment);
    bool isShowingAllProperties() const;

    void showPropertiesIndex (bool showAllProperties);
    void loadLocalLocation (const HelpLocation& location);
    void loadOnlineLocation (const HelpLocation& location);
    void displayOnlineContent (int requestId, const String& url, const String& markdown, bool success);
    String getCurrentOnlineUrl() const;
    
    bool hasCurrentLocation() const { return historyPos >= 0 && historyPos < history.size(); }

    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    CtrlrHelpViewer(const CtrlrHelpViewer&);
    const CtrlrHelpViewer& operator= (const CtrlrHelpViewer&);
};


#endif   // __JUCER_HEADER_CTRLRHELPVIEWER__
