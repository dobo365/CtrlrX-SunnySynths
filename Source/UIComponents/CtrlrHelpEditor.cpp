/*
  ==============================================================================

    CtrlrHelpEditor.cpp
    Created: 12 May 2026 12:30:00pm
    Author:  Dominique Bontemps - dobo365 DB

  ==============================================================================
*/

#include "CtrlrHelpEditor.h"
#include "Misc/BarelyML - Sunny Synths/BarelyML - Sunny Synths.h"
#include "CtrlrManager/CtrlrManager.h"
#include <regex>
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrPanel/CtrlrPanelEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Static file to remember the currently opened/saved help file
static File currentHelpFile;

CtrlrHelpEditor::CtrlrHelpEditor(CtrlrManager &_owner)
    : owner(_owner)
{
	//File helpFile ("C:\\My stuff\\BarelyML\\BarelyML-main\\README.md");
	//String helpContent = helpFile.loadFileAsString();
    String helpContent = "## CtrlrX Help editor\n\nClear this text and start typing your text here using the *markdown* syntax.\n\nOtherwise, under the **File** menu, select **New** to start making a new Help file or **Open** to edit an existing one...";

    bmlText.addListener(this);
    addAndMakeVisible(bmlText);

    bmlText.setName(L"bmlText");
    bmlText.setFont(Font(owner.getFontManager().getDefaultMonoFontName(), 14, Font::plain));
    bmlText.setMultiLine(true);
    bmlText.setReturnKeyStartsNewLine(true);
    bmlText.setReadOnly(false);
    bmlText.setWantsKeyboardFocus(true);
    //bmlText.setText(helpContent);

    // Ensure background and text colours make the text visible
    bmlText.setColour(TextEditor::backgroundColourId, Colour(0xffffffff)); // white background
    //bmlText.setColour(TextEditor::textColourId, Colours::black);           // make text visible
	bmlText.setTextToShowWhenEmpty("Start typing your help text here...", Colours::grey);
    //bmlText.lookAndFeelChanged();

    // Display the mardown area
    addAndMakeVisible(bmlDisplay);
    bmlDisplay.setURLHandler(this);
    bmlDisplay.setMarkdownString(helpContent);

    setSize(600, 400);

    // Apply the selection/highlight rules immediately
    lookAndFeelChanged();

    // ==============================================================================
    // CRITICAL: Asynchronously inject text and force black colour
    // This executes just after JUCE window setup, preventing V4 theme overrides
    // ==============================================================================
    juce::MessageManager::callAsync([this, helpContent]()
        {
            bmlText.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
            bmlText.setColour(juce::TextEditor::textColourId, juce::Colours::black);
            bmlText.setText(helpContent);
            bmlText.repaint();
        });
}

CtrlrHelpEditor::~CtrlrHelpEditor()
{
    bmlText.removeListener(this);
}

void CtrlrHelpEditor::paint(Graphics& g)
{
}

void CtrlrHelpEditor::resized()
{
	int gap = 2;
    int h = getHeight();
    int v = getWidth() - 3*gap; // 3 gaps
    
    bmlText.setBounds(gap, gap, v/2, h - 2*gap);
    bmlDisplay.setBounds(v/2 + 2*gap, gap, v - v / 2, h - 2*gap);
}

void CtrlrHelpEditor::lookAndFeelChanged()
{
    // Call JUCE base method
    Component::lookAndFeelChanged();

    String lnfVersion = "V3"; // Default version
    bool versionFound = false;

    // Try to retrieve the LookAndFeel version from the active panel
    if (auto* activePanel = owner.getActivePanel())
    {
        if (CtrlrPanelEditor* editor = activePanel->getEditor())
        {
            lnfVersion = editor->getProperty(Ids::uiPanelLookAndFeel, "V3").toString();
            versionFound = true;
        }
    }

    // If there is no active panel or no editor, fallback to global CtrlrX preferences
    if (!versionFound)
    {
        lnfVersion = owner.getProperty(Ids::ctrlrColourScheme, "V3").toString();
    }

    // Apply the selection highlight color based on the LookAndFeel version
    if (lnfVersion.startsWithIgnoreCase("V4"))
    {
        // Try to retrieve the active LookAndFeel_V4 from the bmlText component
        if (auto* lfV4 = dynamic_cast<juce::LookAndFeel_V4*>(&bmlText.getLookAndFeel()))
        {
            auto schemeHighlightedFillColour = lfV4->getCurrentColourScheme().getUIColour(juce::LookAndFeel_V4::ColourScheme::highlightedFill);
            auto schemeHighlightedTextColour = lfV4->getCurrentColourScheme().getUIColour(juce::LookAndFeel_V4::ColourScheme::highlightedText);
            bmlText.setColour(juce::TextEditor::highlightColourId, schemeHighlightedFillColour);
            bmlText.setColour(juce::TextEditor::highlightedTextColourId, schemeHighlightedTextColour);
        }
        // Fallback to the global default JUCE LookAndFeel if dynamic_cast fails
        else if (auto* defaultLfV4 = dynamic_cast<juce::LookAndFeel_V4*>(&juce::LookAndFeel::getDefaultLookAndFeel()))
        {
            auto schemeHighlightedFillColour = defaultLfV4->getCurrentColourScheme().getUIColour(juce::LookAndFeel_V4::ColourScheme::highlightedFill);
            auto schemeHighlightedTextColour = defaultLfV4->getCurrentColourScheme().getUIColour(juce::LookAndFeel_V4::ColourScheme::highlightedText);
            bmlText.setColour(juce::TextEditor::highlightColourId, schemeHighlightedFillColour);
            bmlText.setColour(juce::TextEditor::highlightedTextColourId, schemeHighlightedTextColour);
        }
        // Safe fallback if JUCE is not running a V4 look and feel scheme
        else
        {
            bmlText.setColour(juce::TextEditor::highlightColourId, juce::Colour(0xFFD9D2E9));
            bmlText.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::black);
        }
    }
    else // "V3", "V2", "V1" or lower
    {
        // Fixed light purple / lavender color: 0xFFD9D2E9
        bmlText.setColour(juce::TextEditor::highlightColourId, juce::Colour(0xFFD9D2E9));
        bmlText.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::black);
    }

    // Force to strictly have black on white
    bmlText.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    bmlText.setColour(juce::TextEditor::textColourId, juce::Colours::black);

    bmlText.repaint();
}

StringArray CtrlrHelpEditor::getMenuBarNames()
{
    const char* const names[] = { "File", "Insert", nullptr };
    return StringArray(names);
}

PopupMenu CtrlrHelpEditor::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;
 
    if (topLevelMenuIndex == 0)
    {
        menu.addItem(1, "New");
        menu.addItem(2, "Open");
        menu.addItem(3, "Save");
        menu.addItem(4, "Save As");
    //    menu.addSeparator();
    //    menu.addItem(5, "Close");     // Not working so removing "Close"
    }
    if (topLevelMenuIndex == 1)         // See www.markdownguide.org/cheat-sheet
    {
        menu.addItem(1, "H1 Heading");
        menu.addItem(2, "H2 Heading");
        menu.addItem(3, "H3 Heading");
        menu.addItem(4, "H4 Heading");
        menu.addItem(5, "H5 Heading");
        menu.addSeparator();
        menu.addItem(9, "Table of Contents");
        menu.addSeparator();
        menu.addItem(10, "Bold");
        menu.addItem(11, "Italic");
        menu.addItem(12, "Strikethrough");
        menu.addSeparator();
        menu.addItem(20, "Ordered List");
        menu.addItem(21, "Unordered List");
        menu.addItem(22, "Inline Code");
        menu.addItem(23, "Code Block");
        menu.addItem(24, "Link");
        menu.addItem(25, "Image");
        menu.addItem(26, "Horizontal Rule");
        menu.addItem(27, "Blockquote");
        menu.addSeparator();
        menu.addItem(30, "Information");
        menu.addItem(31, "Hint");
        menu.addItem(32, "Important");
        menu.addItem(33, "Warning");
        menu.addItem(34, "Caution");
        menu.addItem(35, "Misc");
        menu.addItem(36, "Other");
    }   return (menu);
}

void CtrlrHelpEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == 0)
    {
        if (menuItemID == 1)
        {
            // Clear editor window
            bmlText.setText("");
            // clear remembered file when creating a new blank doc
            currentHelpFile = File();
        }
        else if (menuItemID == 2)
        {
            // Open file dialog to select a new help file, then load it in the bmlText text editor
            File defaultDir(owner.getProperty(Ids::ctrlrLastBrowsedFileDirectory));
            FileChooser fc(
                "Open CtrlrX help file",
                defaultDir,
                "*.md;*.txt",
                (bool)owner.getProperty(Ids::ctrlrNativeFileDialogs));

            if (fc.browseForFileToOpen())
            {
                File picked = fc.getResult();

                if (!picked.existsAsFile())
                {
                    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Open", "File not found: " + picked.getFullPathName());
                }
                else
                {
                    String content = picked.loadFileAsString();

                    if (content.isEmpty())
                    {
                        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Open", "File could not be read or is empty: " + picked.getFullPathName());
                    }
                    else
                    {
                        bmlText.setText(content);
                        bmlDisplay.setMarkdownString(content);

                        // Remember the opened file for Save and Save As operations
                        currentHelpFile = picked;
                        owner.setProperty(Ids::ctrlrLastBrowsedFileDirectory, picked.getParentDirectory().getFullPathName());
                    }
                }
            }
        }
        else if (menuItemID == 3)
        {
            // Save the current content of the editor to the original file, if it exists, or prompt for a location if it doesn't
            if (currentHelpFile.existsAsFile())
            {
                bool ok = currentHelpFile.replaceWithText(bmlText.getText());
                if (!ok)
                {
                    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Save", "Failed to save file: " + currentHelpFile.getFullPathName());
                }
                else
                {
                    owner.setProperty(Ids::ctrlrLastBrowsedFileDirectory, currentHelpFile.getParentDirectory().getFullPathName());
                }
            }
            else
            {
                // Fallback to Save As if we don't have a remembered file
                // (fallthrough to Save As behavior)
                File defaultDir(owner.getProperty(Ids::ctrlrLastBrowsedFileDirectory));
                FileChooser fc(
                    "Save CtrlrX help file",
                    defaultDir,
                    "*.md;*.txt",
                    (bool)owner.getProperty(Ids::ctrlrNativeFileDialogs));

                if (fc.browseForFileToSave(true))
                {
                    File saveFile = fc.getResult();
                    if (!saveFile.hasFileExtension(".md") && !saveFile.hasFileExtension(".txt"))
                        saveFile = saveFile.withFileExtension(".md");

                    bool ok = saveFile.replaceWithText(bmlText.getText());
                    if (!ok)
                        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Save", "Failed to save file: " + saveFile.getFullPathName());
                    else
                    {
                        currentHelpFile = saveFile;
                        owner.setProperty(Ids::ctrlrLastBrowsedFileDirectory, saveFile.getParentDirectory().getFullPathName());
                    }
                }
            }
        }
        else if (menuItemID == 4)
        {
            // Save as the current content of the editor to a new file, using a file save dialog to choose the location and name
            File defaultDir(owner.getProperty(Ids::ctrlrLastBrowsedFileDirectory));
            FileChooser fc(
                "Save CtrlrX help file",
                defaultDir,
                "*.md;*.txt",
                (bool)owner.getProperty(Ids::ctrlrNativeFileDialogs));

            if (fc.browseForFileToSave(true))
            {
                File saveFile = fc.getResult();
                if (!saveFile.hasFileExtension(".md") && !saveFile.hasFileExtension(".txt"))
                    saveFile = saveFile.withFileExtension(".md");

                bool ok = saveFile.replaceWithText(bmlText.getText());
                if (!ok)
                    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Save As", "Failed to save file: " + saveFile.getFullPathName());
                else
                {
                    currentHelpFile = saveFile;
                    owner.setProperty(Ids::ctrlrLastBrowsedFileDirectory, saveFile.getParentDirectory().getFullPathName());
                }
            }
        }
        /*
        else if (menuItemID == 5)  // Statement below is not working so removing "Close"
        {
            // Close window
            owner.getWindowManager().toggle(CtrlrManagerWindowManager::HelpEditor, false);
        }
        */
    }
    else if (topLevelMenuIndex == 1)
    {
        if (menuItemID == 1)
            wrapSelectedTextInTags("# ", ""); // H1 Heading
        else if (menuItemID == 2)
            wrapSelectedTextInTags("## ", ""); // H2 Heading
        else if (menuItemID == 3)
            wrapSelectedTextInTags("### ", ""); // H3 Heading
        else if (menuItemID == 4)
            wrapSelectedTextInTags("#### ", ""); // H4 Heading
        else if (menuItemID == 5)
            wrapSelectedTextInTags("##### ", ""); // H5 Heading

        else if (menuItemID == 9)
            generateTableOfContents();

        else if (menuItemID == 10)
            wrapSelectedTextInTags("**", "**"); // Bold
        else if (menuItemID == 11)
            wrapSelectedTextInTags("*", "*"); // Italic
        else if (menuItemID == 12)
            wrapSelectedTextInTags("~~", "~~"); // Strikethrough

        else if (menuItemID == 20)
            wrapSelectedTextInTags("1. ", ""); // Ordered List
        else if (menuItemID == 21)
            wrapSelectedTextInTags("- ", ""); // Unordered List
        else if (menuItemID == 22)
            wrapSelectedTextInTags("`", "`"); // Inline Code
        else if (menuItemID == 23)
        {
            // --- CODE BLOCK INSERTION USING NATIVE STRING OPERATIONS ---
            juce::Range<int> selectionRegion = bmlText.getHighlightedRegion();
            int start = selectionRegion.getStart();
            int end = selectionRegion.getEnd();
            int len = selectionRegion.getLength();

            if (len > 0)
            {
                bmlText.grabKeyboardFocus();

                // Extract the untouched selected text directly from the widget
                String selectedText = bmlText.getHighlightedText();
                String newText;

                // Toggle mode: If already wrapped, unwrap it smoothly
                if (selectedText.startsWith("```\n") && selectedText.endsWith("\n```"))
                {
                    newText = selectedText.substring(3, selectedText.length() - 4);
                    bmlText.insertTextAtCaret(newText);
                    // Restore selection cleanly over the unwrapped text
                    bmlText.setHighlightedRegion(Range<int>(start, start + newText.length()));
                }
                else
                {
                    // Standard wrapping: Create the multiline block structure
                    newText = juce::String("```\n") + selectedText + juce::String("\n```");
                    bmlText.insertTextAtCaret(newText);

                    // Set the final highlighted region to mathematically wrap the entire new block
                    // 4 characters for "```\n" at the start, and 4 characters for "\n```" at the end
                    bmlText.setHighlightedRegion(Range<int>(start, end + 8));
                }

                textEditorTextChanged(bmlText);
            }
            else
            {
                // When there is no selection, insert an empty code block template and position the caret in between
                const int caret = bmlText.getCaretPosition();
                bmlText.insertTextAtCaret("```\n\n```");
                bmlText.setCaretPosition(caret + 4);
                textEditorTextChanged(bmlText);
            }
        }
        else if (menuItemID == 24)
            wrapSelectedTextInTags("[", "](url)"); // Link
        else if (menuItemID == 25)
            wrapSelectedTextInTags("![", "](image-url)"); // Image
        else if (menuItemID == 26)
        {
            bmlText.grabKeyboardFocus();
            bmlText.insertTextAtCaret("\n---\n"); // Horizontal Rule
        }
        else if (menuItemID == 27)
            wrapSelectedTextInTags("> ", ""); // Blockquote

        else if (menuItemID >= 30 && menuItemID <= 39)
        {
            // 1. Determine the alert type string (default is OTHER)
            String typeStr = "OTHER";
            if (menuItemID == 30)      typeStr = "INFORMATION";
            else if (menuItemID == 31) typeStr = "HINT";
            else if (menuItemID == 32) typeStr = "IMPORTANT";
            else if (menuItemID == 33) typeStr = "WARNING";
            else if (menuItemID == 34) typeStr = "CAUTION";
            else if (menuItemID == 35) typeStr = "MISC";

            String openTag = "!!! " + typeStr + "\n";
            String closeTag = "\n!!!";

            juce::Range<int> selectionRegion = bmlText.getHighlightedRegion();
            int start = selectionRegion.getStart();
            int end = selectionRegion.getEnd();
            int len = selectionRegion.getLength();

            if (len > 0)
            {
                bmlText.grabKeyboardFocus();
                String selectedText = bmlText.getHighlightedText();
                String newText;

                // Toggle mode: If already wrapped in this exact tag, unwrap it smoothly
                if (selectedText.startsWith(openTag) && selectedText.endsWith(closeTag))
                {
                    newText = selectedText.substring(openTag.length(), selectedText.length() - closeTag.length());
                    bmlText.insertTextAtCaret(newText);
                    bmlText.setHighlightedRegion(Range<int>(start, start + newText.length()));
                }
                else
                {
                    // Standard wrapping: Apply the multiline alert block structure
                    newText = openTag + selectedText + closeTag;
                    bmlText.insertTextAtCaret(newText);
                    bmlText.setHighlightedRegion(Range<int>(start, end + openTag.length() + closeTag.length()));
                }

                textEditorTextChanged(bmlText);
            }
            else
            {
                // When there is no selection, insert an empty alert block template and position the caret in between
                const int caret = bmlText.getCaretPosition();
                bmlText.insertTextAtCaret(openTag + closeTag);
                bmlText.setCaretPosition(caret + openTag.length());
                textEditorTextChanged(bmlText);
            }
        }
    }
}

// TextEditor::Listener method
void CtrlrHelpEditor::textEditorTextChanged(TextEditor& editor)
{
    if (&editor == &bmlText)
    {
        // Simply fetch the raw content from the text editor widget
        String rawMarkdown = bmlText.getText();

        // Pass the raw data directly to the BarelyML display rendering engine
        // Let convertFromMarkdown and setMarkupString do their native optimized job
        bmlDisplay.setMarkdownString(rawMarkdown);
    }
}

void CtrlrHelpEditor::wrapSelectedTextInTags(const String& openTag, const String& closeTag)
{
    juce::Range<int> highlightRange = bmlText.getHighlightedRegion();
    int start = highlightRange.getStart();
    int len = highlightRange.getLength();

    // =================================================================
    // CASE 1: GENERAL TEXT SELECTION (Lists and Inline Styles)
    // =================================================================
    if (len > 0)
    {
        bmlText.grabKeyboardFocus();
        String selectedText = bmlText.getHighlightedText();
        String newText;

        StringArray lines;
        lines.addLines(selectedText);

        bool isList = (openTag == "- " || openTag == "1. " || openTag == "> ");

        if (isList)     // Handling of lists (Ordered, Unordered, Blockquotes) with Toggle functionality
        {
            // Check if all lines are already wrapped with the inline tag for Toggle mode
            bool allLinesHaveTag = true;
            int validLinesCount = 0;
            std::regex orderedListRegex(R"(^\d+\.\s)");

            for (auto line : lines)
            {
                if (line.trim().isEmpty()) continue;
                validLinesCount++;

                if (openTag == "1. ")
                {
                    if (!std::regex_search(line.toStdString(), orderedListRegex))
                        allLinesHaveTag = false;
                }
                else
                {
                    if (!line.startsWith(openTag))
                        allLinesHaveTag = false;
                }
            }

            if (validLinesCount == 0) allLinesHaveTag = false;

            int listIndex = 1;

            // Reconstruct the text line by line, applying or removing tags as needed
            for (int i = 0; i < lines.size(); ++i)
            {
                String line = lines[i];

                if (line.trim().isNotEmpty())
                {
                    if (allLinesHaveTag)
                    {
                        if (openTag == "1. ")
                        {
                            std::string stdLine = line.toStdString();
                            stdLine = std::regex_replace(stdLine, orderedListRegex, "");
                            newText += String(stdLine);
                        }
                        else
                        {
                            newText += line.substring(openTag.length());
                        }
                    }
                    else
                    {
                        if (openTag == "1. ")
                            newText += String(listIndex++) + ". " + line;
                        else
                            newText += openTag + line;
                    }
                }
                else
                {
                    newText += line;
                }

                if (i < lines.size() - 1) newText += "\n";
            }
        }
        else     // Handling of inline elements (Bold, Italic, Inline Code, Links) with Toggle functionality
        {
            bool isLinkOrImage = (openTag == "[" || openTag == "![");

			// Check if all lines are already wrapped with the inline tag for Toggle mode
            bool allLinesHaveInlineTag = true;
            int validLinesCount = 0;

            for (auto line : lines)
            {
                if (line.trim().isEmpty()) continue;
                validLinesCount++;

                bool lineHasTag = false;
                if (isLinkOrImage)
                    lineHasTag = line.startsWith(openTag) && line.contains("](");
                else
                    lineHasTag = line.startsWith(openTag) && line.endsWith(closeTag) && line.length() >= (openTag.length() + closeTag.length());

                if (!lineHasTag)
                    allLinesHaveInlineTag = false;
            }

            if (validLinesCount == 0) allLinesHaveInlineTag = false;

			// Reconstruct the text line by line, applying or removing tags as needed
            for (int i = 0; i < lines.size(); ++i)
            {
                String line = lines[i];

                if (line.trim().isNotEmpty())
                {
                    if (allLinesHaveInlineTag)
                    {
						// Toggle mode: Remove the inline tags while preserving the inner content. For links/images, only remove the outer brackets and keep the link text.
                        if (isLinkOrImage)
                        {
                            int firstCloseBracket = line.indexOfChar(']');
                            newText += line.substring(openTag.length(), firstCloseBracket);
                        }
                        else
                        {
                            newText += line.substring(openTag.length(), line.length() - closeTag.length());
                        }
                    }
                    else
                    {
						// If the line is already a list item (e.g., "1. My text"), the tag will be applied 
                        // AFTER the list number only if the user selected the text without the number.
                        newText += openTag + line + closeTag;
                    }
                }
                else
                {
					newText += line; // Preserve empty lines without adding tags
                }

                if (i < lines.size() - 1) newText += "\n";
            }
        }

		// Actual insert of the new text with tags applied or removed
        bmlText.insertTextAtCaret(newText);

        // Visual adjustment of the selection
        int newEnd = start + newText.length();
        bmlText.setHighlightedRegion(Range<int>(start, newEnd));

        textEditorTextChanged(bmlText);
        return;
    }

    // =================================================================
    // CASE 2: NO TEXT SELECTED (Caret position insertion)
    // =================================================================
    const int caret = bmlText.getCaretPosition();
    const String original = bmlText.getText();

    if (openTag == "```") //
    {
        // Special structural insert for an empty code block with rows layout split
        bmlText.insertTextAtCaret("```\n\n```");
        bmlText.setCaretPosition(caret + 4); // Place the caret right inside the empty code row
    }
    else if (openTag.endsWith(" ") || openTag.endsWith(". ") || openTag.endsWith("> "))
    {
        int lineStart = original.substring(0, caret).lastIndexOfChar('\n');
        lineStart = (lineStart < 0) ? 0 : lineStart + 1;
        bmlText.setCaretPosition(lineStart);
        bmlText.insertTextAtCaret(openTag);
        bmlText.setCaretPosition(lineStart + openTag.length());
    }
    else
    {
        // Inline code (`) falls here natively if no selection, creating "``" and centering caret
        bmlText.setCaretPosition(caret);
        bmlText.insertTextAtCaret(openTag + closeTag);
        bmlText.setCaretPosition(caret + openTag.length());
    }

    textEditorTextChanged(bmlText);
}

bool CtrlrHelpEditor::handleURL(juce::String url)
{
    DBG(">>> handleURL called with URL : " + url);

    // If the URL starts with "goto://", we navigate to the targeted heading instead of opening a web browser 
    if (url.startsWith("goto://"))
    {
        // Extract and decode the targeted heading text
        String targetHeading = URL::removeEscapeChars(url.substring(7)).trim();
        DBG(">>> Target structural marker extracted: " + targetHeading);

        // Separate the Markdown prefix from the actual text label
        bool isH3 = targetHeading.startsWith("### ");
        String pureHeadingLabel = targetHeading.substring(isH3 ? 4 : 3).trim();

        // --- STEP A: Sync left TextEditor cursor position (With safe substring searching) ---
        String fullText = bmlText.getText();

        // Find where the Table of Contents header itself starts
        int tocStartPos = fullText.indexOfIgnoreCase("## Table of contents");
        int searchStartPos = 0;

        if (tocStartPos >= 0)
        {
            // Find the next heading after the TOC to mark the real end of the TOC block
            int nextHeadingPos = fullText.indexOf(tocStartPos + 20, "\n## ");
            if (nextHeadingPos > tocStartPos)
                searchStartPos = nextHeadingPos;
            else
                searchStartPos = tocStartPos + 20; // Fallback if no other heading exists
        }

        // Search for our structural marker ONLY after the TOC section to prevent false matches
        int caretTargetPos = fullText.indexOf(searchStartPos, "\n" + targetHeading);

        if (caretTargetPos >= 0)
        {
            // Move the caret to position
            int finalCaretPos = caretTargetPos + 1;
            
            bmlText.grabKeyboardFocus();

            // Force a scroll to the very end of the file first
            bmlText.setCaretPosition(fullText.length());

            // Now snap back to our real heading line (JUCE will center it or put it near the top)
            bmlText.setCaretPosition(finalCaretPos);
            bmlText.moveCaretRight(false, false);
            bmlText.moveCaretLeft(false, false);
        }

        // --- STEP B: Scroll right graphic viewport canvas layout (Transportable Logic) ---
        const auto& documentBlocks = bmlDisplay.getBlocks();
        int matchCount = 0;

        for (int i = 0; i < documentBlocks.size(); ++i)
        {
            if (auto* currentBlock = documentBlocks[i])
            {
                String blockText = currentBlock->toString().trim();

                if (blockText.equalsIgnoreCase(pureHeadingLabel))
                {
                    matchCount++;

                    // If it's the first match, it's highly likely the one inside the TOC.
                    // We skip it and wait for the second match (the actual heading downstream).
                    if (matchCount == 1)
                    {
                        DBG(">>> Skipping first match (TOC row) for label: " + pureHeadingLabel);
                        continue;
                    }

                    DBG(">>> REAL MATCH FOUND on block " + String(i) + "! Scrolling to Y = " + String(currentBlock->getY()));

                    if (auto* scrollContainer = bmlDisplay.getViewport())
                    {
                        // Apply a 45 pixels top offset to let the heading breathe
                        int targetY = currentBlock->getY() - 45;
                        if (targetY < 0) targetY = 0;

                        scrollContainer->setViewPosition(0, targetY);
                    }

                    bmlDisplay.repaint();
                    return true;
                }
            }
        }

        // ULTIMATE FALLBACK: If there was no second match (e.g. the TOC row was the only one),
        // we execute the scroll on the first match anyway.
        if (matchCount == 1)
        {
            for (int i = 0; i < documentBlocks.size(); ++i)
            {
                if (auto* currentBlock = documentBlocks[i])
                {
                    if (currentBlock->toString().trim().equalsIgnoreCase(pureHeadingLabel))
                    {
                        if (auto* scrollContainer = bmlDisplay.getViewport())
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

    // If the URL starts with "copy://", we copy the text located after "copy://" to the clipboard instead of opening a web browser 
    if (url.startsWith("copy://"))
    {
        String encryptedCode = url.substring(7);
        String codeToCopy = URL::removeEscapeChars(encryptedCode);
        SystemClipboard::copyTextToClipboard(codeToCopy);
        return true;
    }

    return false; // Let JUCE handle regular web links (http://...)
}

void CtrlrHelpEditor::generateTableOfContents()
{
    String text = bmlText.getText();
    StringArray lines;
    lines.addLines(text);

    String tocMarkdown = "## Table of contents\n";
    bool hasHeadings = false;

    for (auto& line : lines)
    {
        String trimmed = line.trimStart();
        if (trimmed.startsWith("####")) continue;

        bool isH3 = trimmed.startsWith("### ");
        bool isH2 = !isH3 && trimmed.startsWith("## ");

        if (isH2 || isH3)
        {
            hasHeadings = true;
            String headingText = trimmed.substring(isH3 ? 4 : 3).trim();

            if (headingText.containsIgnoreCase("Table of contents"))
                continue;

            String fullMarker = (isH2 ? "## " : "### ") + headingText;

            if (isH2)
            {
                tocMarkdown << "* [" << headingText << "](goto://" << fullMarker << ")\n";
            }
            else
            {
                tocMarkdown << "  * [" << headingText << "](goto://" << fullMarker << ")\n";
            }
        }
    }

    if (hasHeadings)
    {
        const int startIndex = bmlText.getCaretPosition();
        bmlText.insertTextAtCaret(tocMarkdown);
        const int endIndex = startIndex + tocMarkdown.length();

        bmlText.grabKeyboardFocus();
        bmlText.setHighlightedRegion(juce::Range<int>(startIndex, endIndex));
        textEditorTextChanged(bmlText);
    }
}