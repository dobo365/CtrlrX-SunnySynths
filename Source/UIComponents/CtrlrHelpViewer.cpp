/*
  ==============================================================================

    CtrlrHelpViewer.cpp
    Created: 5 May 2026 12:30:00pm
    Author:  dobo365 DB
 
    2026-11-08 Adapted to support 3 displays: online, offline and properties

  ==============================================================================
*/

#include "CtrlrHelpViewer.h"
#include "Misc/BarelyML - Sunny Synths/BarelyML - Sunny Synths.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrWindowManagers/CtrlrChildWindowContainer.h"
#include <algorithm>
#include <memory>

using namespace juce;

namespace {
    // Helper to search Help directories and collect .md/.txt files
    static void discoverHelpFiles(const File& appDir, std::map<String, File>& outMap)
    {
        Array<File> candidates;
        candidates.add(appDir.getChildFile("Help/Properties"));
        candidates.add(appDir.getChildFile("help/Properties"));
        candidates.add(appDir.getChildFile("Resources/Help/Properties"));
        candidates.add(appDir.getChildFile("resources/help/Properties"));
        candidates.add(File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("CtrlrHelp/Properties"));

        StringArray extensions = { ".md", ".MD", ".txt", ".markdown" };

        for (auto d : candidates)
        {
            if (!d.exists() || !d.isDirectory()) continue;
            DirectoryIterator di(d, false, "*", File::findFiles);
            while (di.next())
            {
                File f = di.getFile();
                for (auto &ext : extensions)
                {
                    if (f.hasFileExtension(ext))
                    {
                        String key = f.getFileNameWithoutExtension();
                        outMap[key] = f;
                        break;
                    }
                }
            }
        }
    }
} // namespace

// URL handler that will call loadMarkdownFile when a link is clicked
struct HelpViewerURLHandler : public BarelyMLDisplay::URLHandler
{
    CtrlrHelpViewer* owner;

    HelpViewerURLHandler(CtrlrHelpViewer* o) : owner(o) {}

    bool handleURL(juce::String url) override
    {
        if (!owner) return false;

        // Secure & Isolated Internal Document Jump Protocol ---
        if (url.startsWith("goto://"))
        {
            // Extract the target marker (e.g., "## Tables" or "### Layouts")
            String targetMarker = URL::removeEscapeChars(url.substring(7)).trim();

            // Separate the Markdown prefix from the actual text label
            bool isH3 = targetMarker.startsWith("### ");
            String pureHeadingLabel = targetMarker.substring(isH3 ? 4 : 3).trim();

            // Fetch the block collection layers directly from the owner's display
            const auto& documentBlocks = owner->bmlDisplay.getBlocks();
            int matchCount = 0;

            for (int i = 0; i < documentBlocks.size(); ++i)
            {
                if (auto* currentBlock = documentBlocks[i])
                {
                    String blockText = currentBlock->toString().trim();

                    if (blockText.equalsIgnoreCase(pureHeadingLabel))
                    {
                        matchCount++;

                        // Skip the first match because it's the clickable row inside the TOC block
                        if (matchCount == 1)
                            continue;

                        // Second match found: This is our real targeted downstream section heading!
                        if (auto* scrollContainer = owner->bmlDisplay.getViewport())
                        {
                            // Apply a 45 pixels top offset margin to let the heading breathe comfortably
                            int targetY = currentBlock->getY() - 45;
                            if (targetY < 0) targetY = 0;

                            scrollContainer->setViewPosition(0, targetY);
                        }

                        owner->bmlDisplay.repaint();
                        return true; // Click event successfully consumed
                    }
                }
            }

            // If there was no second match (e.g., single unique heading), scroll to the first one
            if (matchCount == 1)
            {
                for (int i = 0; i < documentBlocks.size(); ++i)
                {
                    if (auto* currentBlock = documentBlocks[i])
                    {
                        if (currentBlock->toString().trim().equalsIgnoreCase(pureHeadingLabel))
                        {
                            if (auto* scrollContainer = owner->bmlDisplay.getViewport())
                            {
                                int targetY = currentBlock->getY() - 45;
                                if (targetY < 0) targetY = 0;
                                scrollContainer->setViewPosition(0, targetY);
                            }
                            break;
                        }
                    }
                }
            }

            return true;
        }

        // Support help://id (resolved through the property help files)
        if (url.startsWithIgnoreCase ("help://"))
        {
            String id = url.fromFirstOccurrenceOf ("help://", false, false);
            auto it = owner->helpFileMap.find (id);
            if (it != owner->helpFileMap.end())
            {
                CtrlrHelpViewer::HelpLocation loc;
                loc.kind  = CtrlrHelpViewer::HelpLocation::localFile;
                loc.value = it->second.getFullPathName();
                owner->navigateTo (loc, true);
                return true;
            }
            return false;
        }

        // Relative links of the offline manual
        if (owner->currentMode == CtrlrHelpViewer::offlineMode)
            return owner->handleOfflineLink (url);

        // Links of the online manual
        if (owner->currentMode == CtrlrHelpViewer::onlineMode)
            return owner->handleOnlineLink (url);
        
        // Support file://fullpath
        if (url.startsWithIgnoreCase ("file://"))
        {
            CtrlrHelpViewer::HelpLocation loc;
            loc.kind  = CtrlrHelpViewer::HelpLocation::localFile;
            loc.value = url.fromFirstOccurrenceOf ("file://", false, false);
            owner->navigateTo (loc, true);
            return true;
        }

        // Also support a direct id (no scheme)
        auto it = owner->helpFileMap.find (url);
        if (it != owner->helpFileMap.end())
        {
            CtrlrHelpViewer::HelpLocation loc;
            loc.kind  = CtrlrHelpViewer::HelpLocation::localFile;
            loc.value = it->second.getFullPathName();
            owner->navigateTo (loc, true);
            return true;
        }

        return false; // Let the fallback (open in browser) happen
    }
};

// Resolves the image paths of a markdown page relative to the folder of the page being displayed
struct HelpImageSource : public BarelyMLDisplay::FileSource
{
    CtrlrHelpViewer* owner;

    HelpImageSource (CtrlrHelpViewer* o) : owner (o) {}

    std::unique_ptr<Drawable> getDrawableForFilename (String filename) override
    {
        if (owner == nullptr)
            return nullptr;

        // Online mode: the images have been downloaded beforehand (see showOnlineMarkdown)
        if (owner->currentMode == CtrlrHelpViewer::onlineMode)
        {
            const String key = CtrlrHelpViewer::getImageRawUrl (owner->getCurrentOnlineUrl(), filename);
            auto it = owner->onlineImageCache.find (key);

            if (key.isEmpty() || it == owner->onlineImageCache.end() || it->second.getSize() == 0)
                return nullptr;

            return Drawable::createFromImageData (it->second.getData(), it->second.getSize());
        }

        if (owner->currentFile == File())
            return nullptr;

        // Images from the internet are only supported in online mode
        if (filename.containsIgnoreCase ("://"))
            return nullptr;

        const File imageFile = owner->currentFile.getParentDirectory()
                                    .getChildFile (URL::removeEscapeChars (filename).trim());

        if (! imageFile.existsAsFile() || ! imageFile.hasFileExtension ("png;jpg;jpeg;gif;svg"))
            return nullptr;

        return Drawable::createFromImageFile (imageFile);
    }
};

CtrlrHelpViewer::CtrlrHelpViewer(CtrlrManager &_owner)
    : owner(_owner)
{
    // Install URL handler so BarelyMLDisplay will call us on link clicks
    urlHandlerInstance.reset(new HelpViewerURLHandler(this));
    bmlDisplay.setURLHandler(urlHandlerInstance.get());
    imageSourceInstance.reset (new HelpImageSource (this));
    bmlDisplay.setFileSource (imageSourceInstance.get());
    bmlDisplay.setImageScale (1.5f); // Try 2.0f for even bigger images
    
    addAndMakeVisible(bmlDisplay);
    setSize(1000, 800);
}

CtrlrHelpViewer::~CtrlrHelpViewer()
{
    // deleteAndZero(bmlText);
    // urlHandlerInstance unique_ptr will be destroyed automatically
}

void CtrlrHelpViewer::paint(Graphics& g)
{
}

void CtrlrHelpViewer::resized()
{
    bmlDisplay.setBounds(getLocalBounds());
}

StringArray CtrlrHelpViewer::getMenuBarNames()
{
    StringArray names;

    names.add ("Navigate");

    // The "Properties" menu only exists in the Properties mode
    if (currentMode == propertiesMode)
        names.add ("Properties");

    return names;
}

PopupMenu CtrlrHelpViewer::getMenuForIndex (int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;

    if (menuName == "Properties")
    {
        const bool all = isShowingAllProperties();
        const bool onIndex = hasCurrentLocation() && history[historyPos].kind == HelpLocation::propertyIndex;

        menu.addItem (miPropsDocumented, "Documented properties only", true, onIndex && ! all);
        menu.addItem (miPropsAll,        "All properties",             true, onIndex && all);
    }
    else if (menuName == "Navigate")
    {
        menu.addItem (miBack,    "Back",    historyPos > 0);
        menu.addItem (miForward, "Forward", historyPos < history.size() - 1);
        menu.addSeparator();
        menu.addItem (miHome,    "Home");
        menu.addItem (miReload,  "Reload");

        if (currentMode == onlineMode)
        {
            menu.addSeparator();
            menu.addItem (miOpenInBrowser, "Open page in browser");
            menu.addItem (miCopyUrl,       "Copy page URL");
        }
        else if (currentMode == offlineMode)
        {
            menu.addSeparator();
           #if JUCE_MAC
            menu.addItem (miRevealFile, "Show in Finder", currentFile.existsAsFile());
           #else
            menu.addItem (miRevealFile, "Show in Explorer", currentFile.existsAsFile());
           #endif
        }
    }
    return (menu);
}

void CtrlrHelpViewer::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
    switch (menuItemID)
    {
        case miPropsDocumented:
        case miPropsAll:
        {
            HelpLocation loc;
            loc.kind  = HelpLocation::propertyIndex;
            loc.value = (menuItemID == miPropsAll) ? "all" : "documented";
            navigateTo (loc, true);
            break;
        }

        case miBack:     goBack();    break;
        case miForward:  goForward(); break;
        case miHome:     navigateTo (getHomeLocation(), true); break;

        case miReload:
            if (hasCurrentLocation())
            {
                history.getReference (historyPos).scrollY = getCurrentScrollY();

                if (history[historyPos].kind == HelpLocation::onlineUrl)
                {
                    onlineCache.erase (history[historyPos].value);   // Force a new download...
                    onlineImageCache.clear();                        // ...including the images (failed ones are retried)
                }

                display (history[historyPos]);   // Handy when you are editing the .md files
            }
            break;
            
        case miOpenInBrowser:
            if (hasCurrentLocation())
                URL (history[historyPos].value).launchInDefaultBrowser();
            break;

        case miCopyUrl:
            if (hasCurrentLocation())
                SystemClipboard::copyTextToClipboard (history[historyPos].value);
            break;

        case miRevealFile:
            currentFile.revealToUser();
            break;

        default:
            break;
    }
}

void CtrlrHelpViewer::loadMarkdownFile(const File helpFile)
{
    // Diagnostic checks
    if (!helpFile.existsAsFile())
    {
        bmlDisplay.setMarkdownString("Oups! No file found so I cannot display any help...");
    }
    else
    {
        String helpContent = helpFile.loadFileAsString();
        if (helpContent.isEmpty())
        {
            bmlDisplay.setMarkdownString("Oups! File could not be read or is empty so I cannot display any help...");
        }
        else
        {
            bmlDisplay.setMarkdownString(helpContent);
        }
    }
}

File CtrlrHelpViewer::getHelpBaseDir()
{
    // Same base as discoverHelpFiles()
    return File::getSpecialLocation (File::currentApplicationFile).getParentDirectory();
}

File CtrlrHelpViewer::findOfflineManualHome()
{
    const File appDir = getHelpBaseDir();
    Array<File> candidates;

    // Both cases are listed because Windows/macOS are case-insensitive but Linux is not
    candidates.add (appDir.getChildFile ("Help/User guide/README.md"));
    candidates.add (appDir.getChildFile ("help/User guide/README.md"));
    candidates.add (appDir.getChildFile ("Resources/Help/User guide/README.md"));
    candidates.add (appDir.getChildFile ("resources/help/User guide/README.md"));
    candidates.add (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("CtrlrHelp/User guide/README.md"));
    
    for (auto& f : candidates)
        if (f.existsAsFile())
            return f;

    return File();
}

bool CtrlrHelpViewer::isMarkdownFile (const File& f)
{
    return f.hasFileExtension ("md;markdown;txt");   // hasFileExtension is case-insensitive
}

// GitHub-like heading anchor: lowercase, keep letters/digits/'-'/'_', spaces become '-'
String CtrlrHelpViewer::slugify (const String& text)
{
    const String t = text.toLowerCase().trim();
    String slug;

    for (int i = 0; i < t.length(); ++i)
    {
        const juce_wchar c = t[i];

        if (CharacterFunctions::isLetterOrDigit (c) || c == '-' || c == '_')
            slug += String::charToString (c);
        else if (c == ' ')
            slug += "-";
    }
    return slug;
}

void CtrlrHelpViewer::showPropertiesHelp() { openMode (propertiesMode); }
void CtrlrHelpViewer::showOfflineHelp()    { openMode (offlineMode); }
void CtrlrHelpViewer::showOnlineHelp()     { openMode (onlineMode); }

void CtrlrHelpViewer::openMode (HelpMode newMode)
{
    currentMode = newMode;
    history.clear();
    historyPos = -1;
    currentFile = File();

    // The menu bar names depend on the mode: ask the container (the MenuBarModel) to rebuild it
    if (auto* container = getChildWindowContainer())
        container->menuItemsChanged();

    if (newMode == offlineMode && ! findOfflineManualHome().existsAsFile())
    {
        bmlDisplay.setMarkdownString ("# Offline help not found\n\n"
                                      "Expected file: `Help/User guide/README.md`\n\n"
                                      "Looked next to the application:\n\n`" + getHelpBaseDir().getFullPathName() + "`");
        return;
    }

    navigateTo (getHomeLocation(), true);
}

CtrlrHelpViewer::HelpLocation CtrlrHelpViewer::getHomeLocation() const
{
    HelpLocation home;

    switch (currentMode)
    {
        case offlineMode:
            home.kind  = HelpLocation::localFile;
            home.value = findOfflineManualHome().getFullPathName();
            break;

        case onlineMode:
            home.kind  = HelpLocation::onlineUrl;
            home.value = "https://github.com/damiensellier/CtrlrX/blob/master/Doc/manual/README.md";
            break;

        default:
            home.kind  = HelpLocation::propertyIndex;
            home.value = "documented";
            break;
    }
    return home;
}

int CtrlrHelpViewer::getCurrentScrollY()
{
    if (auto* vp = bmlDisplay.getViewport())
        return vp->getViewPositionY();
    return 0;
}

void CtrlrHelpViewer::navigateTo (HelpLocation location, bool addToHistory)
{
    // Remember where we were on the page we are leaving
    if (hasCurrentLocation())
        history.getReference (historyPos).scrollY = getCurrentScrollY();

    if (addToHistory)
    {
        history.removeRange (historyPos + 1, history.size() - (historyPos + 1)); // Drop the "forward" entries
        history.add (location);
        historyPos = history.size() - 1;
    }

    display (location);
}

void CtrlrHelpViewer::goBack()
{
    if (historyPos > 0)
    {
        history.getReference (historyPos).scrollY = getCurrentScrollY();
        --historyPos;
        display (history[historyPos]);
    }
}

void CtrlrHelpViewer::goForward()
{
    if (historyPos < history.size() - 1)
    {
        history.getReference (historyPos).scrollY = getCurrentScrollY();
        ++historyPos;
        display (history[historyPos]);
    }
}

void CtrlrHelpViewer::display (const HelpLocation& location)
{
    ++navigationId;
    pendingAnchor  = location.anchor;
    pendingScrollY = location.scrollY;

    switch (location.kind)
    {
        case HelpLocation::propertyIndex:
            showPropertiesIndex (location.value == "all");
            applyPendingScroll();
            break;

        case HelpLocation::localFile:
            loadLocalLocation (location);
            applyPendingScroll();
            break;

        case HelpLocation::onlineUrl:
            loadOnlineLocation (location);   // Applies the scroll itself when the download is done
            break;
    }
}

void CtrlrHelpViewer::loadLocalLocation (const HelpLocation& location)
{
    currentFile = File (location.value);
    loadMarkdownFile (currentFile);   // Existing function: shows an error page if the file is missing
}

void CtrlrHelpViewer::loadOnlineLocation (const HelpLocation& location)
{
    // Already downloaded during this session: display it immediately
    auto cached = onlineCache.find (location.value);
    if (cached != onlineCache.end())
    {
        showOnlineMarkdown (navigationId, location.value, cached->second);
        return;
    }

    bmlDisplay.setMarkdownString ("# Loading...\n\n`" + location.value + "`");

    Component::SafePointer<CtrlrHelpViewer> safeThis (this);
    const int requestId = navigationId;   // display() has just incremented it
    const String url    = location.value;
    const String rawUrl = toRawUrl (url);

    Thread::launch ([safeThis, requestId, url, rawUrl]()
    {
        String content;
        bool ok = false;
        int statusCode = 0;

        // JUCE 6.0.x signature (URL::InputStreamOptions only exists from 6.1)
        std::unique_ptr<InputStream> stream (URL (rawUrl).createInputStream (false, nullptr, nullptr, String(),
                                                                             8000, nullptr, &statusCode));
        if (stream != nullptr && statusCode == 200)
        {
            content = stream->readEntireStreamAsString();
            ok = content.isNotEmpty();
        }

        // Back to the message thread to touch the UI
        MessageManager::callAsync ([safeThis, requestId, url, content, ok]()
        {
            if (safeThis != nullptr)
                safeThis->displayOnlineContent (requestId, url, content, ok);
        });
    });
}

void CtrlrHelpViewer::showPropertiesIndex (bool showAllPropertiesMode)
{
    const File appDir = getHelpBaseDir();

    // Try several likely locations for CtrlrIDs.xml
    File idsFile = appDir.getChildFile("Help").getChildFile("Properties").getChildFile("CtrlrIDs.xml");
    if (!idsFile.existsAsFile())
        idsFile = appDir.getChildFile("Resources").getChildFile("XML").getChildFile("CtrlrIDs.xml");
    if (!idsFile.existsAsFile())
        idsFile = appDir.getChildFile("CtrlrIDs.xml");

    if (!idsFile.existsAsFile())
    {
        bmlDisplay.setMarkdownString("# Help\n\nNo CtrlrIDs.xml file found.");
        return;
    }

    XmlDocument myDocument(idsFile);
    std::unique_ptr<XmlElement> xmlRoot = myDocument.getDocumentElement();

    if (xmlRoot == nullptr)
    {
        bmlDisplay.setMarkdownString("# Help\n\nError parsing CtrlrIDs.xml: " + myDocument.getLastParseError());
        return;
    }

    // Discover help files and populate helpFileMap
    helpFileMap.clear();
    discoverHelpFiles(appDir, helpFileMap);

    String markdown;
    if (showAllPropertiesMode)
        markdown << "# All Properties\n\n";
    else
        markdown << "# Documented Properties\n\n";

    StringArray rawLines;
    rawLines.addLines(idsFile.loadFileAsString());
    String pendingHeader;
    bool headerHasItems = false;
    bool firstHeader = true;

    for (auto rawLine : rawLines)
    {
        String line = rawLine.trim();

        // Comment section header
        if (line.startsWith("<!--") && line.contains("-->"))
        {
            String header = line.fromFirstOccurrenceOf("<!--", false, false)
                .upToFirstOccurrenceOf("-->", false, false)
                .trim();
            if (header.isNotEmpty())
            {
                if (!firstHeader)
                    pendingHeader = "\n### " + header + "\n";
                else
                {
                    pendingHeader = "### " + header + "\n";
                    firstHeader = false;
                }
                headerHasItems = false;
            }
        }
        // CtrlrX property: use JUCE XmlElement to read its attributes safely
        else if (line.startsWith("<id"))
        {
            String cleanedLine = line.trimEnd();

            XmlDocument smallParser(cleanedLine);
            std::unique_ptr<XmlElement> propElement = smallParser.getDocumentElement();

            if (propElement != nullptr && propElement->hasTagName("id"))
            {
                String idname = propElement->getStringAttribute("name");
                String idtext = propElement->getStringAttribute("text");

                if (idname.isEmpty()) continue;

                bool hasHelp = (helpFileMap.find(idname) != helpFileMap.end());

                if (hasHelp || showAllPropertiesMode)
                {
                    // Print header if section contains valid items
                    if (pendingHeader.isNotEmpty() && !headerHasItems)
                    {
                        markdown << pendingHeader;
                        pendingHeader = String();
                        headerHasItems = true;
                    }

                    if (hasHelp)
                        markdown << "- [" << idname << "](help://" << idname << ") — " << (idtext.isNotEmpty() ? idtext : "No description") << "\n";
                    else
                        markdown << "- " << idname << " — " << (idtext.isNotEmpty() ? idtext : "No description") << "\n";
                }
            }
        }
    }
    bmlDisplay.setMarkdownString(markdown);
}

bool CtrlrHelpViewer::isShowingAllProperties() const
{
    return hasCurrentLocation()
        && history[historyPos].kind == HelpLocation::propertyIndex
        && history[historyPos].value == "all";
}

void CtrlrHelpViewer::applyPendingScroll()
{
    if (pendingScrollY >= 0)
    {
        if (auto* vp = bmlDisplay.getViewport())
            vp->setViewPosition (0, pendingScrollY);
    }
    else if (pendingAnchor.isNotEmpty())
    {
        scrollToAnchor (pendingAnchor);
    }
    else if (auto* vp = bmlDisplay.getViewport())
    {
        vp->setViewPosition (0, 0);
    }

    pendingAnchor = String();
    pendingScrollY = -1;
}

bool CtrlrHelpViewer::scrollToAnchor (const String& fragment)
{
    Viewport* vp = bmlDisplay.getViewport();
    if (vp == nullptr || fragment.isEmpty())
        return false;

    const String wanted = slugify (fragment);
    auto& blocks = bmlDisplay.getBlocks();
    int found = -1;

    // Keep the LAST match: a table of contents placed before the heading has the same text
    for (int i = 0; i < blocks.size(); ++i)
        if (auto* b = blocks[i])
            if (slugify (b->toString()) == wanted)
                found = i;

    if (found < 0)
        return false;

    vp->setViewPosition (0, jmax (0, blocks[found]->getY() - 45));
    return true;
}

bool CtrlrHelpViewer::handleOfflineLink (const String& url)
{
    // Internet links are never followed inside the viewer: open them in the system browser
    if (url.containsIgnoreCase ("://") || url.startsWithIgnoreCase ("mailto:"))
        return false;   // BarelyML falls back to URL(url).launchInDefaultBrowser()

    if (url.startsWithIgnoreCase ("www."))
    {
        URL ("https://" + url).launchInDefaultBrowser();
        return true;
    }

    const String linkPath = URL::removeEscapeChars (url.upToFirstOccurrenceOf ("#", false, false));
    const String anchor   = url.contains ("#") ? URL::removeEscapeChars (url.fromFirstOccurrenceOf ("#", false, false)) : String();

    Component::SafePointer<CtrlrHelpViewer> safeThis (this);

    // Anchor on the current page
    if (linkPath.isEmpty())
    {
        MessageManager::callAsync ([safeThis, anchor]()
        {
            if (safeThis != nullptr)
                safeThis->scrollToAnchor (anchor);
        });
        return true;
    }

    // getChildFile() resolves "./" and "../" for us
    const File target = currentFile.getParentDirectory().getChildFile (linkPath);

    if (! target.existsAsFile())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Help",
                                          "File not found:\n" + target.getFullPathName(), "OK");
        return true;
    }

    if (! isMarkdownFile (target))
    {
        target.startAsProcess();   // Image, PDF... opened by the default application
        return true;
    }

    // Defer: the clicked Block is destroyed by setMarkdownString(), so never do it inside its mouseUp()
    MessageManager::callAsync ([safeThis, target, anchor]()
    {
        if (safeThis == nullptr)
            return;

        HelpLocation loc;
        loc.kind   = HelpLocation::localFile;
        loc.value  = target.getFullPathName();
        loc.anchor = anchor;
        safeThis->navigateTo (loc, true);
    });
    return true;
}

// ---------------------------------------------------------------------------
// Online help
// ---------------------------------------------------------------------------

bool CtrlrHelpViewer::isInternalHelpUrl (const String& url)
{
    return url.startsWithIgnoreCase ("https://github.com/damiensellier/CtrlrX/");
}

// https://github.com/<user>/<repo>/blob/<branch>/<path>
//   -> https://raw.githubusercontent.com/<user>/<repo>/<branch>/<path>
String CtrlrHelpViewer::toRawUrl (const String& githubBlobUrl)
{
    const String githubPrefix = "https://github.com/";
    const int blobIndex = githubBlobUrl.indexOf ("/blob/");

    if (githubBlobUrl.startsWithIgnoreCase (githubPrefix) && blobIndex > 0)
    {
        return "https://raw.githubusercontent.com/"
               + githubBlobUrl.substring (githubPrefix.length(), blobIndex)
               + "/" + githubBlobUrl.substring (blobIndex + 6);
    }
    return githubBlobUrl;
}

// Resolves a (possibly relative) link against the URL of the page currently displayed
String CtrlrHelpViewer::resolveRelativeUrl (const String& baseUrl, const String& link)
{
    if (link.contains ("://") || link.startsWithIgnoreCase ("mailto:"))
        return link;   // Already absolute

    // Scheme-less external address, as in [Google](www.google.com)
    if (link.startsWithIgnoreCase ("www."))
        return "https://" + link;

    // Pure anchor: stays on the current page
    if (link.startsWith ("#"))
        return baseUrl.upToFirstOccurrenceOf ("#", false, false) + link;

    const String fragment = link.contains ("#") ? "#" + link.fromFirstOccurrenceOf ("#", false, false) : String();
    const String linkPath = link.upToFirstOccurrenceOf ("#", false, false);

    const String base = baseUrl.upToFirstOccurrenceOf ("#", false, false)
                               .upToFirstOccurrenceOf ("?", false, false);

    const int schemeEnd = base.indexOf ("://");
    if (schemeEnd < 0)
        return link;

    const int pathStart = base.indexOfChar (schemeEnd + 3, '/');
    if (pathStart < 0)
        return link;

    const String origin  = base.substring (0, pathStart);
    const String baseDir = base.substring (pathStart).upToLastOccurrenceOf ("/", true, false);
    const String rawPath = linkPath.startsWith ("/") ? linkPath : baseDir + linkPath;

    // Normalise "." and ".." segments
    StringArray segments;
    for (auto& part : StringArray::fromTokens (rawPath, "/", ""))
    {
        if (part == "..")
        {
            if (segments.size() > 0)
                segments.remove (segments.size() - 1);
        }
        else if (part != "." && part.isNotEmpty())
        {
            segments.add (part);
        }
    }

    return origin + "/" + segments.joinIntoString ("/") + fragment;
}

String CtrlrHelpViewer::getCurrentOnlineUrl() const
{
    if (hasCurrentLocation() && history[historyPos].kind == HelpLocation::onlineUrl)
        return history[historyPos].value;

    return String();
}

void CtrlrHelpViewer::displayOnlineContent (int requestId, const String& url, const String& markdown, bool success)
{
    // Keep what we downloaded even if the user has moved on in the meantime
    if (success)
        onlineCache[url] = markdown;

    // Ignore stale answers: another page was requested since
    if (requestId != navigationId)
        return;

    if (success)
    {
        showOnlineMarkdown (requestId, url, markdown);   // Downloads the images of the page if needed, then displays it
        return;
    }

    bmlDisplay.setMarkdownString ("# Online help unavailable\n\nCould not download:\n\n`" + url
                                  + "`\n\nCheck your internet connection, then use Navigate > Reload "
                                    "or Navigate > Open page in browser.");
    applyPendingScroll();
}

// Called by the URL handler for every link clicked while in online mode
bool CtrlrHelpViewer::handleOnlineLink (const String& url)
{
    DBG ("CtrlrHelpViewer::handleOnlineLink " + url);

    const String baseUrl  = getCurrentOnlineUrl();
    const String absolute = resolveRelativeUrl (baseUrl, url);
    const String target   = absolute.upToFirstOccurrenceOf ("#", false, false);
    const String anchor   = absolute.contains ("#") ? URL::removeEscapeChars (absolute.fromFirstOccurrenceOf ("#", false, false)) : String();

    const bool samePage     = (target == baseUrl.upToFirstOccurrenceOf ("#", false, false));
    const bool internalPage = isInternalHelpUrl (target) && target.endsWithIgnoreCase (".md");

    // Images, releases, external sites, other file types: system browser
    if (! samePage && ! internalPage)
    {
        URL (absolute).launchInDefaultBrowser();
        return true;
    }

    // Defer: the clicked Block is destroyed by setMarkdownString(), so never do it inside its mouseUp()
    Component::SafePointer<CtrlrHelpViewer> safeThis (this);
    MessageManager::callAsync ([safeThis, target, anchor, samePage]()
    {
        if (safeThis == nullptr)
            return;

        if (samePage)
        {
            safeThis->scrollToAnchor (anchor);
        }
        else
        {
            HelpLocation loc;
            loc.kind   = HelpLocation::onlineUrl;
            loc.value  = target;
            loc.anchor = anchor;
            safeThis->navigateTo (loc, true);
        }
    });
    return true;
}

// Returns the download URL of an image of an online page, or an empty string if it must not be downloaded.
// Used both when downloading and when displaying, so that both sides find the same cache key.
String CtrlrHelpViewer::getImageRawUrl (const String& pageUrl, const String& imageAddress)
{
    String address = imageAddress.trim();

    // Remove an optional markdown title:  path "title"
    const int quoteIndex = address.indexOfChar ('"');
    if (quoteIndex > 0)
        address = address.substring (0, quoteIndex).trim();

    // BarelyML hands over the address without its "?..." part ("?raw=true" is not needed anyway)
    address = address.upToFirstOccurrenceOf ("?", false, false)
                     .upToFirstOccurrenceOf ("#", false, false)
                     .replace (" ", "%20");

    if (address.isEmpty())
        return String();

    const String absolute = toRawUrl (resolveRelativeUrl (pageUrl, address));

    // Only HTTPS images hosted by GitHub are downloaded
    static const char* const allowedPrefixes[] = { "https://raw.githubusercontent.com/",
                                                   "https://github.com/",
                                                   "https://user-images.githubusercontent.com/" };

    for (auto* prefix : allowedPrefixes)
        if (absolute.startsWithIgnoreCase (prefix))
            return absolute;

    return String();
}

// Addresses of all the markdown images ![alt](address) of a page
StringArray CtrlrHelpViewer::extractImageAddresses (const String& markdown)
{
    StringArray addresses, lines;
    lines.addLines (markdown);

    for (auto& line : lines)
    {
        int pos = 0;

        while (true)
        {
            const int start = line.indexOf (pos, "![");
            if (start < 0) break;

            const int middle = line.indexOf (start + 2, "](");
            if (middle < 0) break;

            const int end = line.indexOf (middle + 2, ")");
            if (end < 0) break;

            addresses.addIfNotAlreadyThere (line.substring (middle + 2, end));
            pos = end + 1;
        }
    }
    return addresses;
}

// BarelyML loads the images synchronously while it parses a page: the images of the page are downloaded
// first (in a background thread) and the page is displayed once they are available.
void CtrlrHelpViewer::showOnlineMarkdown (int requestId, const String& url, const String& markdown)
{
    StringArray missing;

    for (auto& address : extractImageAddresses (markdown))
    {
        const String imageUrl = getImageRawUrl (url, address);

        if (imageUrl.isNotEmpty() && onlineImageCache.find (imageUrl) == onlineImageCache.end())
            missing.addIfNotAlreadyThere (imageUrl);
    }

    if (missing.isEmpty())
    {
        bmlDisplay.setMarkdownString (markdown);
        applyPendingScroll();
        return;
    }

    bmlDisplay.setMarkdownString ("# Loading...\n\n" + String (missing.size()) + " image(s) to download");

    Component::SafePointer<CtrlrHelpViewer> safeThis (this);

    Thread::launch ([safeThis, requestId, markdown, missing]()
    {
        std::vector<std::pair<String, MemoryBlock>> downloaded;

        for (auto& imageUrl : missing)
        {
            MemoryBlock data;
            int statusCode = 0;

            std::unique_ptr<InputStream> stream (URL (imageUrl).createInputStream (false, nullptr, nullptr, String(),
                                                                                   8000, nullptr, &statusCode));
            if (stream != nullptr && statusCode == 200)
                stream->readIntoMemoryBlock (data, 20 * 1024 * 1024);   // 20 MB maximum per image

            downloaded.emplace_back (imageUrl, std::move (data));       // An empty block means "download failed"
        }

        MessageManager::callAsync ([safeThis, requestId, markdown, downloaded]()
        {
            if (safeThis == nullptr)
                return;

            for (auto& item : downloaded)
                safeThis->onlineImageCache[item.first] = item.second;

            // Ignore stale answers: another page was requested since
            if (requestId == safeThis->navigationId && safeThis->currentMode == CtrlrHelpViewer::onlineMode)
            {
                safeThis->bmlDisplay.setMarkdownString (markdown);
                safeThis->applyPendingScroll();
            }
        });
    });
}
