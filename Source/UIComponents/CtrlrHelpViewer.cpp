/*
  ==============================================================================

    CtrlrHelpViewer.cpp
    Created: 5 May 2026 12:30:00pm
    Author:  dobo365 DB

  ==============================================================================
*/

#include "CtrlrHelpViewer.h"
#include "Misc/BarelyML - Sunny Synths/BarelyML - Sunny Synths.h"
#include "CtrlrManager/CtrlrManager.h"
#include <algorithm>
#include <memory>

using namespace juce;

namespace {
    // Helper to search Help directories and collect .md/.txt files
    static void discoverHelpFiles(const File& appDir, std::map<String, File>& outMap)
    {
        Array<File> candidates;
        candidates.add(appDir.getChildFile("help"));
        candidates.add(appDir.getChildFile("Help"));
        candidates.add(appDir.getChildFile("resources").getChildFile("help"));
        candidates.add(appDir.getChildFile("Resources").getChildFile("help"));
        candidates.add(File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("CtrlrHelp"));

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


        // Support help://id
        if (url.startsWithIgnoreCase("help://"))
        {
            String id = url.fromFirstOccurrenceOf("help://", false, false);
            auto it = owner->helpFileMap.find(id);
            if (it != owner->helpFileMap.end())
            {
                owner->loadMarkdownFile(it->second);
                return true;
            }
            return false;
        }

        // Support file://fullpath
        if (url.startsWithIgnoreCase("file://"))
        {
            String path = url.fromFirstOccurrenceOf("file://", false, false);
            File f(path);
            owner->loadMarkdownFile(f);
            return true;
        }

        // also support direct id (no scheme)
        auto it = owner->helpFileMap.find(url);
        if (it != owner->helpFileMap.end())
        {
            owner->loadMarkdownFile(it->second);
            return true;
        }

        return false; // let fallback (open in browser) happen
    }
};

CtrlrHelpViewer::CtrlrHelpViewer(CtrlrManager &_owner)
    : owner(_owner)
{
    // Install URL handler so BarelyMLDisplay will call us on link clicks
    urlHandlerInstance.reset(new HelpViewerURLHandler(this));
    bmlDisplay.setURLHandler(urlHandlerInstance.get());

    addAndMakeVisible(bmlDisplay);
    setSize(600, 800);
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
    const char* const names[] = { "View", nullptr };
    return StringArray(names);
}

PopupMenu CtrlrHelpViewer::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        menu.addItem(1, "Current help content");
        menu.addItem(2, "All properties");
    }
    return (menu);
}

void CtrlrHelpViewer::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == 1 || menuItemID == 2)
    {
        const bool showAllPropertiesMode = (menuItemID == 2);

        File appDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory();

        // Try several likely locations for CtrlrIDs.xml
        File idsFile = appDir.getChildFile("Resources").getChildFile("XML").getChildFile("CtrlrIDs.xml");
        if (!idsFile.existsAsFile())
            idsFile = appDir.getChildFile("Help").getChildFile("CtrlrIDs.xml");
        if (!idsFile.existsAsFile())
            idsFile = appDir.getChildFile("CtrlrIDs.xml");

        if (!idsFile.existsAsFile())
        {
            bmlDisplay.setMarkdownString("# Help\n\nNo CtrlrIDs.xml file found.");
            return;
        }

        // --- JUCE XML PARSER INITIALIZATION (JUCE 6.1.5+) ---
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
            markdown << "# Current help content\n\n";

        // Scan for headers and properties. Compatible with JUCE 6.1.5 ---
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
            // CtrlrX property. Use JUCE XmlElement to locate its attributes safely
            else if (line.startsWith("<id"))
            {
                // Clean the line for \r, \n et invisible spaces at end of line to be safe on Mac and Windows
                String cleanedLine = line.trimEnd();
                
                // Isolate the element text to parse it safely via JUCE XML Engine
                XmlDocument smallParser(cleanedLine);
                std::unique_ptr<XmlElement> propElement = smallParser.getDocumentElement();

                if (propElement != nullptr && propElement->hasTagName("id"))
                {
                    // JUCE 6.1.5 native extractors - totally immune to tabs/spaces layout bugs
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
                            pendingHeader = String(); // Clear standby
                            headerHasItems = true;
                        }

                        if (hasHelp)
                        {
                            markdown << "- [" << idname << "](help://" << idname << ") — " << (idtext.isNotEmpty() ? idtext : "No description") << "\n";
                        }
                        else
                        {
                            markdown << "- " << idname << " — " << (idtext.isNotEmpty() ? idtext : "No description") << "\n";
                        }
                    }
                }
            }
        }
        bmlDisplay.setMarkdownString(markdown);
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
