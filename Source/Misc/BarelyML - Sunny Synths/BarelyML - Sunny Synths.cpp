/*
 ==============================================================================
 
 BarelyML - Sunny Synths.cpp
 Created: 10 May 2026
 Author:  Dominique Bontemps - Sunny Synths

 Version: 0.8 17 June 2026
 - Changed inline code tagging to "`" and code blocks tagging with "```", which is the standard markdown syntax
 
 Version: 0.7 10 June 2026
 - Task lists showing icons instead of [x] text
 - Handles links in list items
 
 Version: 0.6 06 June 2026
 - Added horizontal rule drawing using standard markdown "---" tag
 - Blockquote marking using standard markdown "> " tag
 - Adapted Admonition block tagging as "!!! TYPE" and "!!!"
 - Possible Admonition block types are: HINT, INFO, CAUTION, WARNING, OTHER, MISC 
 - Completely rewritten Admonition blocks rendering that also covers blockquote
 - Modified H1 heading rendering to be more in line with GitHub's H1 rendering (with a line drawn below the text)

 Version: 0.5 01 June 2026
 - Supports inline code (using $$) and code blocks (using ££)
 - Code text is displayed in courier new font on a grey rectangle background
 - Ability to copy code block by pressing an icon
 
 Version: 0.4 22 May 2026
 All functions re-indented for easier reading
 Corrected usage of table background colours 
 Adapted to be closer to the Github Wiki layout
 - Added support for ~~ blocks to indicate a strikethrough text
 - Handle right/center/left alignment in tables columns
 - Links are not in bold; they get a pointing hand cursor when hovered
 - Tables are rendered as in Github Wiki (even rows have a very light background, grid in light grey, header in bold)
  
 ==============================================================================

 BarelyML.cpp
 Created: 5 Oct 2023
 Author:  Fritz Menzer
 Version: 0.3

 ==============================================================================
 Copyright (C) 2023-2024 Fritz Menzer

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of BarelyML and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ==============================================================================
*/

#include <JuceHeader.h>
#include "BarelyML - Sunny Synths.h"

using namespace juce;

//==============================================================================

// MARK: - Display

BarelyMLDisplay::BarelyMLDisplay()
{
  // default colour palette (CGA 16 colours with some extensions)
  colours.set("black",        "#000");
  colours.set("blue",         "#00A");
  colours.set("green",        "#0A0");
  colours.set("cyan",         "#0AA");
  colours.set("red",          "#A00");
  colours.set("magenta",      "#A0A");
  colours.set("brown",        "#A50");
  colours.set("lightgray",    "#AAA");
  colours.set("darkgray",     "#555");
  colours.set("lightblue",    "#55F");
  colours.set("lightgreen",   "#5F5");
  colours.set("lightcyan",    "#5FF");
  colours.set("lightred",     "#F55");
  colours.set("lightmagenta", "#F5F");
  colours.set("yellow",       "#FF5");
  colours.set("white",        "#FFF");
  colours.set("orange",       "#FA5");
  colours.set("pink",         "#F5F");
  colours.set("darkyellow",   "#AA0");
  colours.set("purple",       "#A0F");
  colours.set("gray",         "#777");
  colours.set("linkcolour",     "#0969da");     // Was #00A in original v0.3 version, alignment to GitHub's link colour, which is a bit lighter and looks good on both light and dark backgrounds.
  colours.set("strikecolour",   "#C00101");     // Default colour to identify a strikethrough text.
  colours.set("codecolour",     "0xFF24292F");  // Default colour to identify a code text.
  colours.set("tableHeaderBkgd", "#5FF");
  colours.set("tableEvenRowBkgd", "#0xFFF6F8FA");
  colours.set("tableGrid",      "#0xFFD0D7DE");

  // default font
  font = Font(15);
  
  // default background
  bg = Colours::white;

  // default table backgrounds
  tableBGHeader = Block::parseHexColourStatic(colours["lightcyan"], Colours::black);
  tableBG = Block::parseHexColourStatic(colours["lightgray"], Colours::black);

  // default table margins
  tableMargin = 10;
  tableGap = 2;
  
  // default list indents
  indentPerSpace = 15;
  labelGap = 30;
  
  // default content margin
  margin = 20;
  
  // default admonition margin and sizes
  iconsize = 20;
  admargin = 10;
  adlinewidth = 2;
  
  // default file source (none)
  fileSource = nullptr;

  addAndMakeVisible(viewport);
  viewport.setViewedComponent(&content, false); // we manage the content component
  viewport.setScrollBarsShown(true, false, false, false);  // scroll only vertically, and only when needed
  //viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::nonHover);     // Commented as this is for Juce 8, but we want to keep Juce 6 compatibility for now. We can uncomment this when we switch to Juce 8.
  viewport.setScrollOnDragEnabled(false);
}

BarelyMLDisplay::~BarelyMLDisplay() { }

void BarelyMLDisplay::paint (Graphics& g)
{
  g.fillAll(bg);   // clear the background
}

void BarelyMLDisplay::resized()
{
  // let's keep the relative vertical position
  double relativeScrollPosition = static_cast<double>(viewport.getViewPositionY()) / content.getHeight();
  // compute content height
  int h = margin;
  for (int i=0; i<blocks.size(); i++) {
    int bh;
    bh = blocks[i]->getHeightRequired(getWidth()-2*margin)+5;  // just to be on the safe side
    if (blocks[i]->canExtendBeyondMargin()) {
      blocks[i]->setBounds(0,h,getWidth(),bh);
    } else {
      blocks[i]->setBounds(margin,h,getWidth()-2*margin,bh+10);
    }
    h += bh;
  }
  // set new bounds
  viewport.setBounds(getLocalBounds());
  content.setBounds(0,0,getWidth(), h+margin);
  // set vertical scroll position
  int newScrollY = static_cast<int>(relativeScrollPosition * content.getHeight());
  viewport.setViewPosition(0, newScrollY);
}

void BarelyMLDisplay::setMarkupString(String s)
{
    markupString = s; // Store for later use (e.g., if colors changed)
    blocks.clear();
    StringArray lines;
    lines.addLines(s);

    int li = 0; // Line index switcher

    while (li < lines.size())
    {
        String line = lines[li];
        String cleanedLine = line.trim();

        // --- MULTILINE CODE BLOCK DETECTOR (```) ---
        if (cleanedLine.startsWith("```"))
        {
            StringArray codeBlockLines;
            li++; // Skip the opening '```' marker line

            // Check if the block actually finishes with a closing tag in the file
            bool isClosedByTag = false;

            // Gather all raw lines strictly until the closing marker is found
            while (li < lines.size())
            {
                if (lines[li].trim().startsWith("```"))
                {
                    isClosedByTag = true;       // May be needed to add a "if idClosedByTag then isClosedByTag = true"
                    break;
                }
                codeBlockLines.add(lines[li]);
                li++;
            }

            // Check if there is actual content inside the code arrays
            bool hasValidContent = false;
            for (int k = 0; k < codeBlockLines.size(); ++k)
            {
                if (codeBlockLines[k].isNotEmpty()) // Avoid rendering if empty or containing pure blank rows
                {
                    hasValidContent = true;
                    break;
                }
            }

            if (!codeBlockLines.isEmpty() && hasValidContent)
            {
                // Format code lines together inside a unified XML-like stream
                StringArray processedCodeStream;
                String accumulatedCode = "<blcode>" + codeBlockLines[0];

                for (int j = 1; j < codeBlockLines.size(); ++j)
                {
                    // Even if line is empty, <br> preserves the empty spacing row natively
                    accumulatedCode += "<br>" + codeBlockLines[j];
                }
                accumulatedCode += "</blcode>";

                // If the closing tag was typed, append an indicator tag
                if (isClosedByTag)
                {
                    accumulatedCode += "<closedcode></closedcode>";
                }

                processedCodeStream.add(accumulatedCode);

                // Instanciate a single persistent TextBlock for this code container
                TextBlock* b = new TextBlock();
                b->setBMLDisplay(this);
                b->setColours(&colours);
                b->parseMarkup(processedCodeStream, font);
                content.addAndMakeVisible(b);
                blocks.add(b);
            }

            if (li < lines.size()) // Skip the closing '```' marker line safely
                li++;

            continue;
        }

        // --- LIST ITEM DETECTOR (-, *, +) ---
        if (ListItem::isListItem(line))
        {
            ListItem* b = new ListItem;
            b->setBMLDisplay(this);
            b->setColours(&colours);
            if (Block::containsLink(line)) { line = b->consumeLink(line); }
            b->parseItemMarkup(line, font, indentPerSpace, labelGap);
            content.addAndMakeVisible(b);
            blocks.add(b);
            li++;
        }

        // --- BLOCKQUOTE DETECTOR (>) ---
        else if (cleanedLine.startsWith(">"))
        {
            StringArray quoteLines;

            // Absorb all consecutive lines starting with '>' to make a single block
            while (li < lines.size() && lines[li].trim().startsWith(">"))
            {
                String rawLine = lines[li].trimStart();
                if (rawLine.startsWith("> "))
                    quoteLines.add(rawLine.substring(2));
                else
                    quoteLines.add(rawLine.substring(1));

                li++;
            }

            if (!quoteLines.isEmpty())
            {
                AdmonitionBlock* b = new AdmonitionBlock;
                b->setBMLDisplay(this);
                b->setColours(&colours);

                String combinedText = quoteLines[0];
                for (int j = 1; j < quoteLines.size(); ++j)
                    combinedText += "<br>" + quoteLines[j];

                b->parseAdmonitionMarkup("BLOCKQUOTE: " + combinedText, font, iconsize, admargin, adlinewidth);

                content.addAndMakeVisible(b);
                blocks.add(b);
            }
        }

        // --- ADMONITION BLOCK DETECTOR (!!! TYPE ... !!!) ---
        else if (cleanedLine.startsWith("!!!") && cleanedLine.length() >= 3)
        {
            String blockType = cleanedLine.substring(3).trim();
            StringArray admonitionBodyLines;
            li++; // Skip the opening declaration row

            // Gather all content rows until the standalone closing tag is hit
            while (li < lines.size() && lines[li].trim() != "!!!")
            {
                admonitionBodyLines.add(lines[li]);
                li++;
            }

            if (!admonitionBodyLines.isEmpty())
            {
                AdmonitionBlock* b = new AdmonitionBlock;
                b->setBMLDisplay(this);
                b->setColours(&colours);

                String combinedText = admonitionBodyLines[0];
                for (int j = 1; j < admonitionBodyLines.size(); ++j)
                    combinedText += "<br>" + admonitionBodyLines[j];

                // If user just typed !!! without type yet, fallback safely to INFO styling
                if (blockType.isEmpty()) blockType = "OTHER";

                b->parseAdmonitionMarkup(blockType + ": " + combinedText, font, iconsize, admargin, adlinewidth);

                content.addAndMakeVisible(b);
                blocks.add(b);
            }

            if (li < lines.size()) // Skip the closing token row safely
                li++;

            continue;
        }

        // --- IMAGE BLOCK DETECTOR () ---
        else if (ImageBlock::isImageLine(line))
        {
            ImageBlock* b = new ImageBlock;
            b->setBMLDisplay(this);
            if (Block::containsLink(line)) { line = b->consumeLink(line); }
            b->parseImageMarkup(line, fileSource);
            content.addAndMakeVisible(b);
            blocks.add(b);
            li++;
        }

        // --- TABLE BLOCK DETECTOR ---
        else if (TableBlock::isTableLine(line))
        {
            TableBlock* b = new TableBlock;
            b->setBMLDisplay(this);
            b->setFileSource(fileSource);
            b->setColours(&colours);
            b->setBGColours(tableBG, tableBGHeader);
            b->setMargins(tableMargin, tableGap, margin);

            StringArray tlines;
            while (li < lines.size() && TableBlock::isTableLine(lines[li]))
            {
                tlines.add(lines[li]);
                li++;
            }
            b->parseMarkup(tlines, font);
            content.addAndMakeVisible(b);
            blocks.add(b);
        }

        // --- HORIZONTAL RULE DETECTOR ---
        else if (cleanedLine == "---" || cleanedLine.replace(" ", "") == "---")
        {
            TextBlock* b = new TextBlock();
            b->setBMLDisplay(this);
            b->setColours(&colours);

            // Pass a completely clean text token string, free of any markdown characters (* or _)
            StringArray hrContainer;
            hrContainer.add("HRRULE");
            b->parseMarkup(hrContainer, font);

            content.addAndMakeVisible(b);
            blocks.add(b);
            li++;
        }

        // --- GENERIC BLOCK DETECTOR ---
        else if (Block::containsLink(line))
        {
            TextBlock* b = new TextBlock();
            b->setBMLDisplay(this);
            b->setColours(&colours);
            line = b->consumeLink(line);
            b->parseMarkup(line, font);
            content.addAndMakeVisible(b);
            blocks.add(b);
            li++;
        }

        // --- STANDARD PLAIN TEXT PARAGRAPH ---
        else
        {
            StringArray blines;
            bool blockEnd = false;

            while (li < lines.size() && !blockEnd)
            {
                String currentLine = lines[li];

                // Security check: Never absorb a structured marker or a code block boundary inside standard text
                String horizontalStopCheck = currentLine.replace(" ", "");
                if (ListItem::isListItem(currentLine) ||
                    TableBlock::isTableLine(currentLine) ||
                    currentLine.trim().startsWith("!!!") ||
                    currentLine.trim().startsWith(">") ||
                    ImageBlock::isImageLine(currentLine) ||
                    Block::containsLink(currentLine) ||
                    currentLine.trim().startsWith("```") ||
                    horizontalStopCheck == "---")
                {
                    break;
                }

                blines.add(currentLine);
                blockEnd = currentLine.isEmpty();
                li++;

                if (li < lines.size())
                {
                    // End block definition if next line starts text structure
                    String nextLine = lines[li];
                    blockEnd &= nextLine.isNotEmpty();
                }
            }

            TextBlock* b = new TextBlock();
            b->setBMLDisplay(this);
            b->setColours(&colours);
            b->parseMarkup(blines, font);
            content.addAndMakeVisible(b);
            blocks.add(b);
        }
    }

    resized();
}

String BarelyMLDisplay::convertFromMarkdown(String md) 
{
    StringArray lines;
    lines.addLines(md);
    String bml;
    bool lastLineWasTable = false;
    
    for (int li=0; li<lines.size(); li++) 
    {
        String line = lines[li];
        
        // Replace unspported unordered list markers
        if (line.trimStart().startsWith("* ")) 
        {
            int idx = line.indexOf("* ");
            line = line.substring(0, idx) + "- " + line.substring(idx+2);
        }
        if (line.trimStart().startsWith("+ ")) 
        {
            int idx = line.indexOf("+ ");
            line = line.substring(0, idx) + "- " + line.substring(idx+2);
        }
        
        // Replace images
        while (line.contains("![") &&
               line.fromFirstOccurrenceOf("![", false, false).contains("](") &&
               line.fromLastOccurrenceOf("](", false, false).contains(")")) 
        {
            // Replace images
            int idx1 = line.indexOf("![");
            int idx2 = line.indexOf(idx1+2, "](");
            int idx3 = line.indexOf(idx2+2, ")");
            String address = line.substring(idx2+2, idx3);
            line = line.substring(0, idx1) + "{{" + address + "}}" + line.substring(idx3+2);
        }

        // Replace links with labels
        while (line.contains("[") &&
               line.fromFirstOccurrenceOf("[", false, false).contains("](") &&
               line.fromLastOccurrenceOf("](", false, false).contains(")")) 
        {
            // Replace links
            int idx1 = line.indexOf("[");
            int idx2 = line.indexOf(idx1+1, "](");
            int idx3 = line.indexOf(idx2+2, ")");
            String text = line.substring(idx1+1, idx2);
            String address = line.substring(idx2+2, idx3);
            line = line.substring(0, idx1) + "[[" + address + "|" + text + "]]" + line.substring(idx3+1);
        }
    
        // Replace links without labels
        while (line.contains("<") &&
               line.fromFirstOccurrenceOf("<", false, false).contains(">") && (
               line.fromFirstOccurrenceOf("<", false, false).startsWith("http://") ||
               line.fromFirstOccurrenceOf("<", false, false).startsWith("https://") ||
               line.fromFirstOccurrenceOf("<", false, false).startsWith("mailto:"))) 
        {
            // replace links
            int idx1 = line.indexOf("<");
            int idx2 = line.indexOf(idx1+1, ">");
            String address = line.substring(idx1+1, idx2);
            line = line.substring(0, idx1) + "[[" + address + "]]" + line.substring(idx2+1);
        }
    
        // Save back the modified line to the array for the table pass below
        lines.set(li, line);
    }

    // Structural markdown table preprocessing pass
    for (int li = 0; li < lines.size() - 1; li++)
    {
        juce::String currentLine = lines[li].trim();
        juce::String nextLine = lines[li + 1].trim();

        if (currentLine.startsWith("|") && nextLine.startsWith("|") && nextLine.contains("---"))
        {
            // Convert header row pipes into carets to flag it natively as a bold table header row
            currentLine = currentLine.replace("|", "^");
            lines.set(li, currentLine);
            // We keep the separator line intact here; TableBlock::parseMarkup will handle its final extraction
        }
    }

    // Single unified loop to build the final document string without text duplications
    bml = ""; // Clear bml to prevent any risk of double appending text blocks
    for (int li = 0; li < lines.size(); li++)
    {
        juce::String line = lines[li];

        if (line.trim().startsWith("^") || line.trim().startsWith("|"))
        {
            lastLineWasTable = true;
        }
        else
        {
            lastLineWasTable = false;
        }

        bml += line + (li < lines.size() - 1 ? "\n" : "");
    }

    // Replace strikeThrough markers
    bool openingStrikeFinal = true;
    while (bml.contains("~~"))
    {
        bml = bml.replaceSection(bml.indexOf("~~"), 2, openingStrikeFinal ? "<strike>" : "</strike>");
        openingStrikeFinal = !openingStrikeFinal;
    }

    // Replace ` markers for inline code
    bool openingGreenFinal = true;
    int backtickIdx = 0;

    while ((backtickIdx = bml.indexOf(backtickIdx, "`")) != -1)
    {
        // No action if 3 backticks (Code Block)
        if (bml.substring(backtickIdx).startsWith("```"))
        {
            backtickIdx += 3;
        }
        // Otherwise, it's a single isolated backtick (Inline Code)
        else
        {
            bml = bml.replaceSection(backtickIdx, 1, openingGreenFinal ? "<greenhighlight>" : "</greenhighlight>");
            openingGreenFinal = !openingGreenFinal;
            // The index does not need to be incremented manually here because the inserted tag
            // does not contain a backtick, the next indexOf will naturally move to the next occurrence.
        }
    }
    /*    while (bml.contains("$$"))
    {
        bml = bml.replaceSection(bml.indexOf("$$"), 2, openingGreenFinal ? "<greenhighlight>" : "</greenhighlight>");
        openingGreenFinal = !openingGreenFinal;
    }
*/
    // Replace bold and italic markers
    String tmpBoldMarker = "%%%BarelyML%%%Bold%%%";
    bml = bml.replace("**", tmpBoldMarker);
    bml = bml.replace("__", tmpBoldMarker);
    bml = bml.replace("*", "_");            // replace italic marker
    bml = bml.replace(tmpBoldMarker, "*");  // replace temporary bold marker

    return bml;
}

String BarelyMLDisplay::convertToMarkdown(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String md;
  
  bool isTable = false;
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    
    // replace table headers
    if (line.startsWith("^") && !isTable) {
      isTable = true;
      line = line.replace("^", "|");
      // count columns
      String tmp = line.substring(1);
      Array<int> colWidths;
      while (tmp.contains("|")) {
        colWidths.add(tmp.indexOf("|"));
        tmp = tmp.fromFirstOccurrenceOf("|", false, false);
      }
      if (!colWidths.isEmpty()) {
        line += "\n|";
        for (int i=0; i<colWidths.size(); i++) {
          int nhyphen = jmax(3,colWidths[i]-2);
          line += " ";
          while (nhyphen>0) {
            line += "-";
            nhyphen--;
          }
          line += " |";
        }
      }
    }
    if (line.startsWith("^") || line.startsWith("|")) {
      isTable = true;
    } else {
      isTable = false;
    }
    
    // replace links
    while (line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
      int idx1 = line.indexOf("[[");
      int idx2 = line.indexOf(idx1, "]]");
      String link = line.substring(idx1+2, idx2);
      if (link.contains("|")) {
        line = line.substring(0, idx1) + "[" + link.fromFirstOccurrenceOf("|", false, false) + "](" + link.upToFirstOccurrenceOf("|", false, false)  + ")" + line.substring(idx2+2);
      } else {
        line = line.substring(0, idx1) + "<" + link + ">" + line.substring(idx2+2);
      }
    }

    // add line
    md += line + (li<lines.size()-1?"\n":"");
  }
  
  // replace bold markers
  md = md.replace("*", "**");
  return md;
}

String BarelyMLDisplay::convertFromDokuWiki(String dw) {
  StringArray lines;
  lines.addLines(dw);
  String bml;
  Array<int> oLI = {1, 1, 1, 1, 1}; // ordered list indices up to 5 nesting levels supported
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // replace headings
    bool isHeading = false;
    if (line.startsWith("====== ")) { line = "# " + line.substring(7); isHeading = true; }
    if (line.startsWith("===== ")) { line = "## " + line.substring(6); isHeading = true; }
    if (line.startsWith("==== ")) { line = "### " + line.substring(5); isHeading = true; }
    if (line.startsWith("=== ")) { line = "#### " + line.substring(4); isHeading = true; }
    if (line.startsWith("== ")) { line = "##### " + line.substring(3); isHeading = true; }
    if (isHeading) { // if we've identified a heading, let's drop the trailing markup
      while (line.endsWith(" ") || line.endsWith("=")) {
        line = line.dropLastCharacters(1);
      }
    }
    // replace ordered list markers (up to 5 nesting levels);
    int oLLevel = 0;
    bool isOL = false;
    if (line.startsWith("  - ")) { line =     String(oLI[0]) + ". " + line.substring(4); oLLevel = 1; isOL = true; }
    if (line.startsWith("    - ")) { line = " " + String(oLI[1]) + ". " + line.substring(6); oLLevel = 2; isOL = true; }
    if (line.startsWith("      - ")) { line = "  " + String(oLI[2]) + ". " + line.substring(8); oLLevel = 3; isOL = true; }
    if (line.startsWith("        - ")) { line = "   " + String(oLI[3]) + ". " + line.substring(10); oLLevel = 4; isOL = true; }
    if (line.startsWith("          - ")) { line = "    " + String(oLI[4]) + ". " + line.substring(12); oLLevel = 5; isOL = true; }
    if (isOL) { // if we've identified an ordered list item, keep track of indices
      oLI.set(oLLevel-1, oLI[oLLevel-1] + 1); // increase counter at this level
    }
    // reset the indices for deeper levels
    for (int i = oLLevel; i < 5; i++) {
      oLI.set(i, 1);
    }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("  * ")) { line = "- " + line.substring(4); }
    if (line.startsWith("    * ")) { line = " - " + line.substring(6); }
    if (line.startsWith("      * ")) { line = "  - " + line.substring(8); }
    if (line.startsWith("        * ")) { line = "   - " + line.substring(10); }
    if (line.startsWith("          * ")) { line = "    - " + line.substring(12); }
    // add line
    bml += line + (li<lines.size()-1?"\n":"");
  }
  
  // save the URLs
  bml = bml.replace("[[http://", "[[http%%%BarelyML%%%URLSEPARATOR%%%");
  bml = bml.replace("[[https://", "[[https%%%BarelyML%%%URLSEPARATOR%%%");

  // replace bold and italic markers
  bml = bml.replace("**", "*");
  bml = bml.replace("//", "_");

  // restore the URLS
  bml = bml.replace("%%%BarelyML%%%URLSEPARATOR%%%", "://");
  
  // replace color markers (supporting a subset of the "color" plugin syntax)
  bml = bml.replace("<color #", "<c#");
  bml = bml.replace("<color " , "<c:");
  bml = bml.replace("</color>", "</c>");
  
  return bml;
}

String BarelyMLDisplay::convertToDokuWiki(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String dw;
  
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // replace bold and italic markers
    line = line.replace("*", "**");
    line = line.replace("_", "//");
    // replace headings
    if (line.startsWith("# ")) { line = "====== " + line.substring(2) + " ======"; }
    if (line.startsWith("## ")) { line = "===== " + line.substring(3) + " ====="; }
    if (line.startsWith("### ")) { line = "==== " + line.substring(4) + " ===="; }
    if (line.startsWith("#### ")) { line = "=== " + line.substring(5) + " ==="; }
    if (line.startsWith("##### ")) { line = "== " + line.substring(6) + " =="; }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("- ")) { line = "  * " + line.substring(2); }
    if (line.startsWith(" - ")) { line = "    * " + line.substring(3); }
    if (line.startsWith("  - ")) { line = "      * " + line.substring(4); }
    if (line.startsWith("   - ")) { line = "        * " + line.substring(5); }
    if (line.startsWith("    - ")) { line = "          * " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels)
    if (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) {
      int didx = line.indexOf(". ");
      if (line.startsWith("    ") && line.substring(4, didx).containsOnly("0123456789")) { line = "          - " + line.substring(didx+2); }
      if (line.startsWith("   ") && line.substring(3, didx).containsOnly("0123456789")) { line = "        - " + line.substring(didx+2); }
      if (line.startsWith("  ") && line.substring(2, didx).containsOnly("0123456789")) { line = "      - " + line.substring(didx+2); }
      if (line.startsWith(" ") && line.substring(1, didx).containsOnly("0123456789")) { line = "    - " + line.substring(didx+2); }
      if (                       line.substring(0, didx).containsOnly("0123456789")) { line = "  - " + line.substring(didx+2); }
    }
    // add line
    dw += line + (li<lines.size()-1?"\n":"");
  }

  // replace color markers (supporting a subset of the "color" plugin syntax)
  dw = dw.replace("<c#" , "<color #");
  dw = dw.replace("<c:" , "<color ");
  dw = dw.replace("</c>", "</color>");
  
  return dw;
}

String BarelyMLDisplay::convertFromAsciiDoc(String ad) {
  StringArray lines;
  lines.addLines(ad);
  String bml;
  Array<int> oLI = {1, 1, 1, 1, 1}; // ordered list indices up to 5 nesting levels supported
  bool isTable = false;
  int tableCols = 0;
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    bool skipLine = false;
    // skip lines in square brackets (these are used for features we don't support)
    if (line.startsWith("[") && line.endsWith("]")) { skipLine = true; }
    // skip table delimiters
    if (line.startsWith("|") && line.substring(1).containsOnly("=")) { skipLine = true; isTable = !isTable; tableCols = 0; }
    // handle table
    if (!skipLine && line.startsWith("|")) {
      if (tableCols == 0) {  // first line -> contains all columns (not guaranteed for remaining lines)
        // count columns
        String tmp = line;
        while (tmp.contains("|")) {
          tmp = tmp.fromFirstOccurrenceOf("|", false, false);
          tableCols++;
        }
        // check if next line is empty
        if (li+1<lines.size() && lines[li+1].isEmpty()) { // empty -> header
          // let's remove ^ characters first, otherwise there will be alignment issues
          line = line.replace("^","").replace("|", "^") + " ^";
        } else { // not empty -> regular table row
          line = line + " |";
        }
      } else {
        // when we're here this is the first line of a non-header table row
        int colsFound = 0;
        // count columns in this line
        String tmp = line;
        while (tmp.contains("|")) {
          tmp = tmp.fromFirstOccurrenceOf("|", false, false);
          colsFound++;
        }
        // accumulate lines until we've found enough columns
        while (colsFound < tableCols && li+1<lines.size() && lines[li+1].startsWith("|") && !lines[li+1].substring(1).containsOnly("=")) {
          String nextLine = lines[++li];
          line += nextLine;
          while (nextLine.contains("|")) {
            nextLine = nextLine.fromFirstOccurrenceOf("|", false, false);
            colsFound++;
          }
        }
        line += " |";
      }
    }
    // skip empty line inside table
    if (isTable && line.isEmpty()) { skipLine = true; }
    // replace headings
    if (line.startsWith("= ")) { line = "# " + line.substring(2); }
    if (line.startsWith("== ")) { line = "## " + line.substring(3); }
    if (line.startsWith("=== ")) { line = "### " + line.substring(4); }
    if (line.startsWith("==== ")) { line = "#### " + line.substring(5); }
    if (line.startsWith("===== ")) { line = "##### " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels);
    int oLLevel = 0;
    bool isOL = false;
    if (line.startsWith(". ")) { line =     String(oLI[0]) + ". " + line.substring(2); oLLevel = 1; isOL = true; }
    if (line.startsWith(".. ")) { line = " " + String(oLI[1]) + ". " + line.substring(3); oLLevel = 2; isOL = true; }
    if (line.startsWith("... ")) { line = "  " + String(oLI[2]) + ". " + line.substring(4); oLLevel = 3; isOL = true; }
    if (line.startsWith(".... ")) { line = "   " + String(oLI[3]) + ". " + line.substring(5); oLLevel = 4; isOL = true; }
    if (line.startsWith("..... ")) { line = "    " + String(oLI[4]) + ". " + line.substring(6); oLLevel = 5; isOL = true; }
    if (isOL) { // if we've identified an ordered list item, keep track of indices
      oLI.set(oLLevel-1, oLI[oLLevel-1] + 1); // increase counter at this level
    }
    // reset the indices for deeper levels
    for (int i = oLLevel; i < 5; i++) {
      oLI.set(i, 1);
    }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("* ")) { line = "- " + line.substring(2); }
    if (line.startsWith("** ")) { line = " - " + line.substring(3); }
    if (line.startsWith("*** ")) { line = "  - " + line.substring(4); }
    if (line.startsWith("**** ")) { line = "   - " + line.substring(5); }
    if (line.startsWith("***** ")) { line = "    - " + line.substring(6); }
    
    // replace admonitions (only NOTE and TIP, the other ones are identical)
    if (line.startsWith("NOTE: ")) { line = "INFO: " + line.substring(6); }
    if (line.startsWith("TIP: ")) { line = "HINT: " + line.substring(5); }

    // replace links
    StringArray linkSchemes = {"http://", "https://", "mailto:"};
    StringArray precedingChar = {" ", "\t"};
    for (int s=0; s<linkSchemes.size(); s++) {
      for (int p=0; p<precedingChar.size(); p++) {
        String target = precedingChar[p] + linkSchemes[s];
        while (line.contains(target) || line.startsWith(linkSchemes[s])) {
          int idx1 = line.indexOf(target)+1;
          if (idx1<0) { // if we don't find the target...
            idx1 = 0;   // ...that means the line starts with the link scheme
          }
          int idx2 = line.indexOf(idx1, " ");
          if (idx2<0) { idx2 = line.indexOf(idx1, "\t"); }
          if (idx2<0) { idx2 = line.length(); }
          // needed for cases like this: [JUCE Forum] (space in label)
          if (line.substring(idx1, idx2).contains("[")) {
            idx2 = jmax(idx2, line.indexOf(idx1, "]")+1);
          }
          String link = line.substring(idx1, idx2);
          if (link.contains("[") && link.endsWith("]")) {
            int lidx = link.indexOf("[");
            line = line.substring(0, idx1) + "[[" + link.substring(0, lidx) + "|" + link.substring(lidx+1) + "]" + line.substring(idx2);
          } else {
            line = line.substring(0, idx1) + "[[" + link + "]]" + line.substring(idx2);
          }
        }
      }
    }

    // add line
    if (!skipLine) {
      bml += line + (li<lines.size()-1?"\n":"");
    }
  }
  
  // replace bold and italic markers
  bml = bml.replace("**", "*");
  bml = bml.replace("__", "_");
  
  // replace color markers (which are actually style markers, so this is not perfectly accurate)
  while (bml.contains("]#") && bml.fromLastOccurrenceOf("]#", false, false).contains("#") && bml.upToLastOccurrenceOf("]#", false, false).contains("[")) {
    int idx2 = bml.lastIndexOf("]#");
    int idx1 = bml.substring(0, idx2).lastIndexOf("[");
    int idx3 = bml.indexOf(idx2+2, "#");
    bml = bml.substring(0, idx1) + "<c:" + bml.substring(idx1+1, idx2) + ">" + bml.substring(idx2+2,idx3) + "</c>" + bml.substring(idx3+1);
  }
  
  return bml;
}

String BarelyMLDisplay::convertToAsciiDoc(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String ad;
  
  bool isTable = false;
  
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // table
    if (line.startsWith("^") || line.startsWith("|")) {
      if (!isTable) { // this is the first line
        line = "|===\n" + line.replace("^", "|").upToLastOccurrenceOf("|", false, false) + "\n";
      } else {
        // drop the trailing | or ^ (note that we assume reasonable well-formedness here)
        line = line.trimEnd().dropLastCharacters(1);
      }
      isTable = true;
      if (li+1 >= lines.size() || !(lines[li+1].startsWith("|") || lines[li+1].startsWith("^"))) {
        // insert a table delimiter before the next line
        line += "\n|===";
        isTable = false;
      }
    }
    // replace headings
    if (line.startsWith("# ")) { line = "= " + line.substring(2); }
    if (line.startsWith("## ")) { line = "== " + line.substring(3); }
    if (line.startsWith("### ")) { line = "=== " + line.substring(4); }
    if (line.startsWith("#### ")) { line = "==== " + line.substring(5); }
    if (line.startsWith("##### ")) { line = "===== " + line.substring(6); }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("- ")) { line = "* " + line.substring(2); }
    if (line.startsWith(" - ")) { line = "** " + line.substring(3); }
    if (line.startsWith("  - ")) { line = "*** " + line.substring(4); }
    if (line.startsWith("   - ")) { line = "**** " + line.substring(5); }
    if (line.startsWith("    - ")) { line = "***** " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels)
    if (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) {
      int didx = line.indexOf(". ");
      if (line.startsWith("    ") && line.substring(4, didx).containsOnly("0123456789")) { line = "..... " + line.substring(didx+2); }
      if (line.startsWith("   ") && line.substring(3, didx).containsOnly("0123456789")) { line = ".... " + line.substring(didx+2); }
      if (line.startsWith("  ") && line.substring(2, didx).containsOnly("0123456789")) { line = "... " + line.substring(didx+2); }
      if (line.startsWith(" ") && line.substring(1, didx).containsOnly("0123456789")) { line = ".. " + line.substring(didx+2); }
      if (                       line.substring(0, didx).containsOnly("0123456789")) { line = ". " + line.substring(didx+2); }
    }
    
    // replace links
    while (line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
      int idx1 = line.indexOf("[[");
      int idx2 = line.indexOf(idx1, "]]");
      String link = line.substring(idx1+2, idx2);
      if (link.startsWith("http://") || link.startsWith("https://") || link.startsWith("mailto:")) {
        if (link.contains("|")) {
          line = line.substring(0, idx1) + link.upToFirstOccurrenceOf("|", false, false) + "[" + link.fromFirstOccurrenceOf("|", false, false) + "]" + line.substring(idx2+2);
        } else {
          line = line.substring(0, idx1) + link + line.substring(idx2+2);
        }
      }
    }
    
    // replace admonitions (only INFO and HINT, the other ones are identical)
    if (line.startsWith("INFO: ")) { line = "NOTE: " + line.substring(6); }
    if (line.startsWith("HINT: ")) { line = "TIP: " + line.substring(6); }

    // add line
    ad += line + (li<lines.size()-1?"\n":"");
  }
  
  // replace color markers (named colors only)
  while (ad.contains("<c:") && ad.fromFirstOccurrenceOf("<c:", false, false).contains(">") && ad.fromFirstOccurrenceOf("<c:", false, false).fromFirstOccurrenceOf(">", false, false).contains("</c>")) {
    int idx1 = ad.indexOf("<c:");
    int idx2 = ad.indexOf(idx1, ">");
    int idx3 = ad.indexOf(idx2, "</c>");
    ad = ad.substring(0, idx1) + "[" + ad.substring(idx1+3, idx2) + "]#" + ad.substring(idx2+1,idx3) + "#" + ad.substring(idx3+4);
  }
  
  return ad;
}


// MARK: - Block

Colour BarelyMLDisplay::Block::parseHexColourStatic(String s, Colour defaultColour) 
{
    if (s.startsWith("#")) 
    {
        s = s.substring(1);
        // If we have 3 or 4 characters, expand by duplicating characters
        if (s.length()==3 || s.length()==4) 
        {
            String expanded;

            for (int i=0; i<s.length(); i++) 
            {
                expanded += s[i];
                expanded += s[i];
            }
            s = expanded;
        }

        if (s.length()==6) 
        { 
            // also applies to duplicated 3 char string
            s = String("FF")+s;
        }
    }

    if (s.isEmpty()) 
    {
        return defaultColour;
    } 
    else 
    {
        return Colour::fromString(s);
    }
}

Colour BarelyMLDisplay::Block::parseHexColour(String s) 
{
    return parseHexColourStatic(s, defaultColour);
}

bool BarelyMLDisplay::Block::containsLink(String line) 
{
    return line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]");
}

String BarelyMLDisplay::Block::consumeLink(String line, String* link) 
{
    // Output either to Block's link or "link" argument
    if (!link) 
    { 
        link = &this->link; 
    }

    int idx1 = line.indexOf("[[");
    int idx2 = line.indexOf(idx1, "]]");

    if (idx1>=0 && idx2>idx1) 
    {
        *link = line.substring(idx1+2, idx2);
        if (link->contains("|")) 
        {
            String altText = link->fromFirstOccurrenceOf("|", false, false);
            *link = link->upToFirstOccurrenceOf("|", false, false);

            // If there's an image in the link...
            if (altText.contains("{{") && altText.fromFirstOccurrenceOf("{{", false, false).contains("}}")) 
            {
                // ...return only the altText...
                return line.substring(0, idx1) + altText + line.substring(idx2+2);
            } 
            else 
            {
                // ...otherwise, return highlighted text.
                //return line.substring(0, idx1) + "<c:linkcolour>*" + altText + "*</c>" + line.substring(idx2 + 2);  //Original code causing the links to be displayed in bold by the *
                return line.substring(0, idx1) + "<c:linkcolour>" + altText + "</c>" + line.substring(idx2 + 2);      //New code. The links will be displayed in the same style as the surrounding text, but with the link color
            }
        } 
        else 
        {
            //return line.substring(0, idx1) + "<c:linkcolour>*" + *link + "*</c>" + line.substring(idx2 + 2);      //Original code causing the links to be displayed in bold by the *
            return line.substring(0, idx1) + "<c:linkcolour>" + *link + "</c>" + line.substring(idx2 + 2);          //New code. The links will be displayed in the same style as the surrounding text, but with the link color
        }
    } 
    else 
    {
        *link = "";
        return line;
    }
}

void BarelyMLDisplay::Block::mouseDown(const MouseEvent& event) 
{
    mouseDownPosition = event.position;     // keep track of position
}

void BarelyMLDisplay::Block::mouseUp(const MouseEvent& event) 
{
    if (link.isNotEmpty()) 
    {
        // if we have a link...
        float distance = event.position.getDistanceFrom(mouseDownPosition);
        if (distance < 20) 
        {                  
            // ...and we're not scrolling...
            jassert(bmlDisplay);
            bmlDisplay->handleURL(link);        // ...let bmlDisplay handle URL.
        }
    }
}

void BarelyMLDisplay::Block::mouseMove(const juce::MouseEvent& event)
{
    // Si ce bloc ne contient pas de lien, on ne fait rien
    if (link.isEmpty())
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        return;
    }

    // On délègue la vérification de collision géométrique à la fonction virtuelle
    if (containsLinkAt(event.getPosition()))
    {
        if (!isHovered)
        {
            isHovered = true;
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            repaint();
        }
    }
    else
    {
        if (isHovered)
        {
            isHovered = false;
            setMouseCursor(juce::MouseCursor::NormalCursor);
            repaint();
        }
    }
}

void BarelyMLDisplay::Block::mouseExit(const juce::MouseEvent& event)
{
    // Sécurité : si la souris quitte complètement le bloc, on réinitialise tout
    if (isHovered)
    {
        isHovered = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

AttributedString BarelyMLDisplay::Block::parsePureText(const StringArray& lines, Font font, bool addNewline)
{
    AttributedString attributedString;
    String currentLine;
    currentColour = defaultColour;
    Justification cellJustification = Justification::left;

    // Store current font height to prevent font shrinking when switching to inline code and back
    const float paragraphBaseHeight = font.getHeight();

  bool bold = false;
  bool italic = false;
  bool strikeThrough = false;
  bool isInlineCode = false;
  bool localGreenHighlight = false;
  bool isMultilineCodeBlock = false;

  for (auto line : lines)
  {
    line = line.replace("\\\\", "\n");
    if (line.startsWith("##### "))
    {
      attributedString.append(parsePureText(line.substring(6), font.boldened().withHeight(font.getHeight()*1.1f),false));
    }
    else if (line.startsWith("#### "))
    {
      attributedString.append(parsePureText(line.substring(5), font.boldened().withHeight(font.getHeight()*1.25f),false));
    }
    else if (line.startsWith("### "))
    {
      attributedString.append(parsePureText(line.substring(4), font.boldened().withHeight(font.getHeight()*1.42f),false));
    }
    else if (line.startsWith("## "))
    {
      attributedString.append(parsePureText(line.substring(3), font.boldened().withHeight(font.getHeight()*1.7f),false));
    }
    else if (line.startsWith("# "))
    {
      attributedString.append(parsePureText(line.substring(2), font.boldened().withHeight(font.getHeight()*2.1f),false));
    }
    else
    {
        while (line.isNotEmpty()) 
        {
        bool needsNewFont = false;
        // Find the first token to interpret
        int bidx = line.indexOf("*");   // Toggle for bold text
        int iidx = line.indexOf("_");   // Toggle for italic text
        int tidx = line.indexOf("<");   // Start of a tag
        Colour nextColour = currentColour;

        if (bidx > -1 && (bidx < iidx || iidx == -1) && (bidx < tidx || tidx == -1)) 
        {
            // if the next token is toggling the bold state...
            // ...first add everything up to the token...
            attributedString.append(line.substring(0, bidx), font, currentColour);
            line = line.substring(bidx+1); // ...then drop up to and including the token...
            bold = !bold;                  // ...toggle the bold status...
            needsNewFont = true;           // ...and request new font.
        } 
        else if (iidx > -1 && (iidx < tidx || tidx == -1))
        {
            // if the next token is toggling the italic state...
            // ...first add everything up to the token...
            attributedString.append(line.substring(0, iidx), font, currentColour);
            line = line.substring(iidx+1); // ...then drop up to and including the token...
            italic = !italic;              // ...toggle the italic status...
            needsNewFont = true;           // ...and request new font.
        } 
        else if (tidx > -1) 
        {
            // if the next token is a tag, first figure out if it is a recognized tag...
            String tag;
            bool tagRecognized = false;

            // find tag end
            int tidx2 = line.indexOf(tidx, ">");
            if (tidx2>tidx) 
            {
                tag = line.substring(tidx+1, tidx2).trim(); // Added trim to suppress spaces aorund the keyword
            }

            if (tag.startsWith("c#")) 
            {
                // hex colour tag
                nextColour = parseHexColour(tag.substring(1));
                tagRecognized = true;
                needsNewFont = true;           // ...to force 
            } 
            else if (tag.startsWith("c:")) 
            {
                // named colour tag
                String name = tag.substring(2);
                if (colours != nullptr && colours->containsKey(name)) 
                {
                    nextColour = parseHexColour((*colours)[name]);
                }
                tagRecognized = true;
                needsNewFont = true;           // ...and request new font.
            }
            else if (tag.startsWith("/c")) 
            {
                // end of colour tag
                nextColour = defaultColour;
                tagRecognized = true;
                needsNewFont = true;           // ...and request new font.
            }
            else if (tag == "strike")
            {
                strikeThrough = true;
                nextColour = parseHexColour((*colours)["strikecolour"]);    // Set text to the strike colour
                tagRecognized = true;
                needsNewFont = true;
            }
            else if (tag == "/strike")
            {
                strikeThrough = false;
                nextColour = defaultColour; // Go back to current colour
                tagRecognized = true;
                needsNewFont = true;
            }
            // Handle the HTML line-break tag for unified multiline blocks ---
            else if (tag == "br")
            {
                // 1. Append the text found right before the <br> tag into the layout
                attributedString.append(line.substring(0, tidx), font, currentColour);

                // 2. Inject a physical carriage return character to trigger JUCE line-break mechanics
                attributedString.append("\n", font, currentColour);

                // 3. Clear the processed segment from the current line string using the tag variable size
                // We add 2 to include the opening '<' and closing '>' characters
                line = line.substring(tidx + tag.length() + 2);

                // 4. Reset index counters to safely reboot character scanning from the new row start position
                tidx = -1;
                tagRecognized = true;
                continue;
            }
            else if (tag == "greenhighlight")
            {
                localGreenHighlight = true;
                nextColour = parseHexColour((*colours)["green"]);
                tagRecognized = true;
                needsNewFont = true;
            }
            else if (tag == "/greenhighlight")
            {
                localGreenHighlight = false;
                nextColour = defaultColour; // Go back to current colour
                tagRecognized = true;
                needsNewFont = true;
            }
            else if (tag == "blcode")
            {
                isMultilineCodeBlock = true;
                nextColour = parseHexColour((*colours)["darkgray"]); // Github look
                tagRecognized = true;
                needsNewFont = true;
            }
            else if (tag == "/blcode")
            {
                isMultilineCodeBlock = false;
                nextColour = defaultColour;
                tagRecognized = true;
                needsNewFont = true;
            }
            else if (tag == "closedcode" || tag == "/closedcode")
            {
                // Fix: Instead of touching a member variable, we append a unique custom property 
                // or tag name into the attributed string metadata, or just a dummy marker text.
                // Let's inject a hidden specific color or text token to flag the closure.
                attributedString.append("##CLOSED##", font, juce::Colours::transparentBlack);
                tagRecognized = true;
            }
            else if (tag == "left")
            { 
                cellJustification = Justification::left; 
                tagRecognized = true; 
            }
            else if (tag == "center") 
            { 
                cellJustification = Justification::centred;
                tagRecognized = true; 
            } 
            else if (tag == "right") 
            { 
                cellJustification = Justification::right; 
                tagRecognized = true; 
            }

            if (tagRecognized) 
            {
                // Extract the text prefix before the tag BEFORE modifying anything
                String textPrefix = line.substring(0, tidx);

                if (tag == "greenhighlight")
                {
                    // Insert a clean standard text space right before entering the code block
                    attributedString.append(textPrefix + " ", font, currentColour);

                    // Insert an isolated styled space inside the code bounds to allow background overlap
                    attributedString.append(" ", font, nextColour);
                }
                else if (tag == "/greenhighlight")
                {
                    // Append text and close the tag layout sequence smoothly
                    attributedString.append(textPrefix, font, currentColour);
                    attributedString.append(" ", font, currentColour);
                }
                else
                {
                    // Standard fallback for other general tags
                    attributedString.append(textPrefix, font, currentColour);
                }
                // ...then drop up to and including the tag.
                line = line.substring(tidx2 + 1);

            }
            else
            {
                // --- UNRECOGNIZED TAG OR HTML TAG ---
                // If a closing '>' was found, safely consume the entire segment as plain text
                if (tidx2 > tidx)
                {
                    attributedString.append(line.substring(0, tidx2 + 1), font, currentColour);
                    line = line.substring(tidx2 + 1);
                }
                else
                {
                    // If '<' has no closing '>' yet (e.g., user is currently typing),
                    // consume only the '<' character to strictly avoid any infinite loop risk.
                    attributedString.append(line.substring(0, tidx + 1), font, currentColour);
                    line = line.substring(tidx + 1);
                }
            }
        }
        else 
        {
            // if no token was found -> add the remaining text...
            attributedString.append(line, font, currentColour);
            // ...and clear the line.
            line.clear();
        }

        currentColour = nextColour;

        if (needsNewFont) {
            float currentHeight = font.getHeight(); 
            font = font.withStyle(Font::plain);
            if (bold) 
            { 
                font = font.boldened(); 
            }
            if (italic) 
            { 
                font = font.italicised(); 
            }

            // Apply Courier New font family for inline and code blocks
            if (localGreenHighlight || isMultilineCodeBlock || currentColour == parseHexColour((*colours)["darkgray"]))
            {
                font = juce::Font(juce::Font::getDefaultMonospacedFontName(), paragraphBaseHeight * 0.85f, font.getStyleFlags());
            }
            else
            {
                // Force rollback to the standard system sans-serif font 
                // whenever code tags are closed or inactive.
                font = juce::Font(juce::Font::getDefaultSansSerifFontName(), paragraphBaseHeight, font.getStyleFlags());
            }
        }
      }
    }
    
    if (addNewline) 
    {
        attributedString.append(" \n", font, defaultColour);
    }
  }

  attributedString.setJustification(cellJustification);
  return attributedString;
}

void BarelyMLDisplay::Block::drawInlineCodeBackground(juce::Graphics& g, float currentX, float runWidth, const juce::TextLayout::Line& line, float verticalPadding)
{
    float lineHeight = line.getLineBounds().getHeight();
    float rectX = currentX - 1.0f;
    float rectY = (line.lineOrigin.y - line.ascent) - verticalPadding;
    float rectW = runWidth;
    float rectH = lineHeight + (verticalPadding * 2.0f);

    // Draw the subtle light grey background rectangle behind the text fragment
    g.setColour(juce::Colours::lightgrey.withAlpha(0.4f));
    g.fillRect(juce::Rectangle<float>(rectX, rectY, rectW, rectH));
	// Wanted to have rounded corners but they are not visible due to lack of height on single-line code fragments)
    // g.fillRoundedRectangle(rectX, rectY, rectW, rectH, 3.0f);
}


// MARK: - Text Block

bool BarelyMLDisplay::TextBlock::containsLinkAt(juce::Point<int> localPoint)
{
    // 1. Récupérer la couleur des liens pour identifier le segment textuel
    juce::Colour linkColor = juce::Colours::blue;
    if (colours != nullptr && colours->containsKey("linkcolour"))
        linkColor = parseHexColour((*colours)["linkcolour"]);

    // 2. Créer un TextLayout pour obtenir les coordonnées physiques des caractères
    juce::TextLayout layout;
    layout.createLayout(attributedString, getWidth());

    // 3. Parcourir les lignes du texte dessiné
    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        auto& line = layout.getLine(i);
 
        // On garde une trace de l'avancement en X au sein de la ligne
        float currentX = line.lineOrigin.x;
        float lineHeight = line.getLineBounds().getHeight();

        // Parcourir chaque segment (Run) de la ligne
        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            // Calculer la largeur totale de ce fragment en sommant ses glyphes
            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
            {
                runWidth += run->glyphs.getReference(k).width;
            }

            // Si ce fragment possède la couleur du lien
            if (run->colour == linkColor)
            {
                // On crée le rectangle de collision exact pour les mots du lien
                juce::Rectangle<float> runBounds(currentX,
                    line.lineOrigin.y - line.getLineBounds().getHeight(),
                    runWidth,
                    lineHeight);

                // Si la souris se trouve à l'intérieur de cette boîte, collision !
                if (runBounds.contains(localPoint.toFloat()))
                    return true;
            }

            // On décale notre curseur X pour le prochain fragment (Run) de la ligne
            currentX += runWidth;
        }
    }
    return false;
}

void BarelyMLDisplay::TextBlock::parseMarkup(const StringArray& lines, Font font)
{
    attributedString = parsePureText(lines, font);

    // Read the code block custom closure token from the parsed layout text string
    if (attributedString.getText().contains("##CLOSED##"))
    {
        isCodeBlockClosed = true; // Variable is perfectly accessible here!

        // Clean up: rebuild the string to remove the hidden structural token text
        String cleanText = attributedString.getText().replace("##CLOSED##", "");

        // Save current attributes, clear, and re-apply without the token text
        juce::AttributedString newString;
        newString.append(cleanText, font, parseHexColour((*colours)["darkgray"]));

        // Re-assign the cleaned version
        attributedString = newString;
    }
    else
    {
        isCodeBlockClosed = false;
    }

    // Check if it's a code block
    juce::Colour codeColor = parseHexColour((*colours)["darkgray"]);

    for (int idx = 0; idx < attributedString.getNumAttributes(); ++idx)
    {
        if (attributedString.getAttribute(idx).colour == codeColor)
        {
            // Direct adjustment: executed ONLY if we are inside a code block
            int finalHeight = juce::roundToInt(getHeightRequired(static_cast<float>(getWidth())));
            setSize(getWidth(), finalHeight);
            break; // Exit the loop immediately since the block is identified
        }
    }
}

float BarelyMLDisplay::TextBlock::getHeightRequired(float width) 
{
	// If this block is an internal horizontal rule separator, consider 4px before, 4 px for the line, and 5px after = 12px total
    if (attributedString.getText().contains("HRRULE"))
    {
        return 12.0f;
    }

    TextLayout layout;
    layout.createLayout(attributedString, width);

    // Check if this specific layout represents a markdown code container
    bool isCodeContainer = false;
    juce::Colour codeColor = parseHexColour((*colours)["darkgray"]);

    for (int idx = 0; idx < attributedString.getNumAttributes(); ++idx)
    {
        if (attributedString.getAttribute(idx).colour == codeColor)
        {
            isCodeContainer = true;
            break;
        }
    }

    // Apply basic padding, and add 16px extra bottom margin ONLY if the closing tag is present
    float padding = 0.0f;

    if (isCodeContainer)
    {
        padding = isCodeBlockClosed ? 24.0f : 24.0f;    // Was 28 and 12
    }
    else if (attributedString.getNumAttributes() > 0)
    {
        float firstRunFontHeight = attributedString.getAttribute(0).font.getHeight();
        float baseFontHeight = (bmlDisplay != nullptr) ? bmlDisplay->font.getHeight() : 15.0f;

        if (firstRunFontHeight >= (baseFontHeight * 2.0f))
        {
            // 4px between text and line + 1px line + 12px margin before next paragraph
            padding = 9.0f;
        }
    }

    return layout.getHeight() + padding;
    // Explicitly append 12 pixels to have a default intiial height with equal distance above and under the copy code button
    //return layout.getHeight() + (isCodeContainer ? 12.0f : 0.0f);
}

void BarelyMLDisplay::TextBlock::mouseDown(const juce::MouseEvent& e)
{
    bool isCodeBlock = false;
    juce::Colour codeBlockMarkerColor = parseHexColour((*colours)["darkgray"]);

    // Identify if this instance is a valid code segment
    for (int idx = 0; idx < attributedString.getNumAttributes(); ++idx)
    {
        auto attr = attributedString.getAttribute(idx);
        if (attr.colour == codeBlockMarkerColor)
        {
            isCodeBlock = true;
            break;
        }
    }

    if (isCodeBlock)
    {
        int btnX = getWidth() - 42;
        int btnY = 10;

        // HITBOX: Must encapsulate the 40x40 rectangle drawn at line 1376
        juce::Rectangle<int> buttonHitbox(btnX, btnY, 40, 40);

        // Check if user click coordinate falls inside the bounds of the Copy button box
        if (buttonHitbox.contains(e.x, e.y))
        {
            // Convert AttributedString storage content to pure plain text string
            juce::String cleanTextToCopy = attributedString.getText();

            // Clean up internal custom XML markup tags if any remain inside the string payload
            cleanTextToCopy = cleanTextToCopy.replace("<blcode>", "").replace("</blcode>", "");

            // Ship native sequence array data straight into OS Desktop Clipboard infrastructure
            juce::SystemClipboard::copyTextToClipboard(cleanTextToCopy.trim());

            return; // Consume the event so it doesn't trigger underlying elements
        }
    }

    // Call baseline component fallback interaction to ensure window focus still flows natively
    Block::mouseDown(e);
}

void BarelyMLDisplay::TextBlock::mouseMove(const juce::MouseEvent& event)
{
    bool isCodeBlockContainer = false;
    juce::Colour codeBlockMarkerColor = parseHexColour((*colours)["darkgray"]);

    // Check if this specific TextBlock is a code block
    for (int idx = 0; idx < attributedString.getNumAttributes(); ++idx)
    {
        auto attr = attributedString.getAttribute(idx);
        if (attr.colour == codeBlockMarkerColor)
        {
            isCodeBlockContainer = true;
            break;
        }
    }

    if (isCodeBlockContainer)
    {
        // TARGET ANCHORS: Matching your paint() layouts at line 1371
        int btnX = getWidth() - 42;
        int btnY = 10;

        // HITBOX: Exact 40x40 area matching the button rectangle bounds
        juce::Rectangle<int> buttonHitbox(btnX, btnY, 32, 32);

        if (buttonHitbox.contains(event.getPosition()))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return; // Exit early to avoid standard link checks from overriding the cursor
        }
    }

    // Fallback to original layout tracking: check if it hovers a text hyperlink
    if (containsLinkAt(event.getPosition()))
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
    else
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void BarelyMLDisplay::TextBlock::paint(juce::Graphics& g)
{
    // --- RENDER HORIZONTAL BREAK RULE ---
    if (attributedString.getText().contains("HRRULE"))
    {
        // Use standard light gray or customized table grid color from your palette
        juce::Colour ruleColor = juce::Colour(216, 222, 228); // GitHub standard #D0D7DE
        if (colours != nullptr && colours->containsKey("tableGrid"))
            ruleColor = parseHexColourStatic((*colours)["tableGrid"], ruleColor);

        float totalWidth = static_cast<float>(getWidth());

        g.setColour(ruleColor);
        // Draw a solid 2px thick crisp horizontal line perfectly centered vertically (at Y=4.0f)
        g.fillRect(0.0f, 4.0f, totalWidth, 4.0f);
        return; // Bypass standard text rendering safely
    }

    // Opaque colors (GitHub Wiki palette)
    juce::Colour codeBackgroundColor(246, 248, 250); // Clean solid light grey
    juce::Colour codeBorderColor(216, 222, 228);     // Defined solid border grey
    juce::Colour iconContourColor(87, 96, 106);       // Darker grey for sharp icon lines

	// Margins for code block content padding (only applied if we detect code block attributes)
    float leftMargin = 0.0f;
    float topMargin = 0.0f;

    /* --- DETECT MULTILINE CODE BLOCK VIA ATTRIBUTED STRING RUNS --- */
    bool hasCodeBlockColor = false;
    juce::Colour codeColor = parseHexColour((*colours)["darkgray"]);

    // Scan the accumulated attributes to check if this block is a code container
    for (int idx = 0; idx < attributedString.getNumAttributes(); ++idx)
    {
        // In JUCE, getAttribute returns by value or structural reference, access with '.'
        auto attr = attributedString.getAttribute(idx);
        if (attr.colour == codeColor)
        {
            hasCodeBlockColor = true;
            break;
        }
    }

    // If a code element is detected, paint the global full-width GitHub style background box first
    if (hasCodeBlockColor)
    {
        // Use local component size dimensions directly to guarantee fit
        float totalWidth = static_cast<float>(getWidth());
        float totalHeight = static_cast<float>(getHeight());

        // Draw smooth rounded rectangle code background (GitHub Wiki standard look)
        g.setColour(codeBackgroundColor);
        g.fillRoundedRectangle(0.0f, 0.0f, totalWidth, totalHeight, 6.0f);

        // Outer border accentuation
        g.setColour(codeBorderColor);
        g.drawRoundedRectangle(0.0f, 0.0f, totalWidth - 1.0f, totalHeight - 1.0f, 6.0f, 1.0f);

        // Vectorized Double-Sheet "Copy to Clipboard" icon in the upper-right corner
		// This coordinate system is relative to the TextBlock component
		// btnX and btnY are the top-left corner of the button's hitbox
        int btnX = totalWidth - 42;
        int btnY = 10;

        // Button hitbox
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(static_cast<float>(btnX), static_cast<float>(btnY), 32.0f, 32.0f, 6.0f, 1.0f);

        // Back sheet
        g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
        g.setColour(iconContourColor);
        g.drawRoundedRectangle(static_cast<float>(btnX + 8), static_cast<float>(btnY + 14), 10.0f, 10.0f, 1.0f, 1.5f);
        // Front sheet (overlapping foreground)
        g.setColour(codeBackgroundColor);
        g.fillRoundedRectangle(static_cast<float>(btnX + 12), static_cast<float>(btnY + 8), 12.0f, 12.0f, 1.0f);
        //g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
        g.setColour(iconContourColor);
        g.drawRoundedRectangle(static_cast<float>(btnX + 14), static_cast<float>(btnY + 8), 10.0f, 10.0f, 1.0f, 1.5f);
    }

    /* Handling of strikethrough text */

    juce::Colour strikeColor = juce::Colour(0xFFC00101); // Default color in case of...
    if (colours != nullptr && colours->containsKey("strikecolour"))
    {
        strikeColor = parseHexColour((*colours)["strikecolour"]);
    }

    // Get paragraph geometry
    juce::TextLayout layout;
    layout.createLayout(attributedString, static_cast<float>(getWidth()) - (hasCodeBlockColor ? 60.0f : 0.0f));

    // Scan all text lines
    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        const auto& line = layout.getLine(i);

        // Base coordinate positions
        float currentX = line.lineOrigin.x;
        float adjustedY = line.lineOrigin.y;
        float lineHeight = line.getLineBounds().getHeight();

        // If we are rendering inside a global code block container, push the line start point to the right
        if (hasCodeBlockColor)
        {
            leftMargin = 18.0f;
            topMargin = 18.0f;
            currentX += leftMargin;  // 18 pixels left margin padding
            adjustedY += topMargin; // 18 pixels top margin padding
        }

        // Scan line
        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            // Calculate the length of each segment between glyphs
            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
            {
                runWidth += run->glyphs.getReference(k).width;
            }

            // Check if the segment is in the strikeThrough colour
            if (run->colour == strikeColor)
            {
                // Set the strikeThrough line colour accordingly
                g.setColour(run->colour);

                // Calculate the suitable Y position (baseline minus ~25% of the line height)
                // to cross the text in the middle of high and low caps letters
                float yPosition = line.lineOrigin.y - (lineHeight * 0.25f);

                float xStart = currentX;
                float xEnd = currentX + runWidth;

                // Draw the line across the identified range
                g.drawLine(xStart, yPosition, xEnd, yPosition, 1.0f);
            }

            // Inline Code Box Layer Rendering (`)

            juce::Colour greenColor = parseHexColour((*colours)["green"]);

            if (run->colour == greenColor)
            {
                // Call the shared rendering system
                drawInlineCodeBackground(g, currentX, runWidth, line, 2.0f);

                // Override text token color to Dark Gray for GitHub standard styling
                run->colour = parseHexColour((*colours)["darkgray"]);
            }
    
            // To check next line segment
            currentX += runWidth;
        }
    }
    
    // Translate the text if it is a code block
    g.saveState();
    if (hasCodeBlockColor)
    {
        // Push the entire text block layout matrix down and right inside the gray container box bounds
        g.addTransform(juce::AffineTransform::translation(leftMargin, topMargin));
    }

    // Draw the text layers flawlessly using native layout routines
    layout.draw(g, juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight())).toFloat());
    g.restoreState();

    // --- GITHUB STYLE H1 HORIZONTAL RULE EXTENSION ---
    // Check if the original markup line was an H1 heading
    if (attributedString.getText().isNotEmpty() && attributedString.getNumAttributes() > 0)
    {
        float firstRunFontHeight = attributedString.getAttribute(0).font.getHeight();
        float baseFontHeight = (bmlDisplay != nullptr) ? bmlDisplay->font.getHeight() : 15.0f;

        if (firstRunFontHeight >= (baseFontHeight * 2.0f))
        {
            // Recalculate layout height to find where the text actually ends
            juce::TextLayout textLayout;
            textLayout.createLayout(attributedString, static_cast<float>(getWidth()));

            // Position the line 4 pixels above the baseline of the text characters
            float lineY = textLayout.getHeight() - 4.0f;
            //float lineY = textLayout.getHeight();
            float lineWidth = static_cast<float>(getWidth());

            // Medium-light gray color, slightly darker than lightgrey for better visibility
            juce::Colour separatorColor = juce::Colour::fromRGB(200, 205, 212);

            g.setColour(separatorColor);
            g.fillRect(0.0f, lineY, lineWidth, 1.0f); // 1px thick crisp horizontal line
        }
    }
}


// MARK: - Admonition Block

bool BarelyMLDisplay::AdmonitionBlock::isAdmonitionLine(const String& line)
{
    String trimmed = line.trimStart();

    // Check for the starting markers of blockquotes or macro blocks
    if (trimmed.startsWith(">") || trimmed.startsWith("!!!"))
        return true;

    return line.startsWith("INFO: ") || line.startsWith("HINT: ") || line.startsWith("IMPORTANT: ") || line.startsWith("CAUTION: ") || line.startsWith("WARNING: ") || line.startsWith("OTHER: ") || line.startsWith("MISC: ");
}

void BarelyMLDisplay::AdmonitionBlock::setupAdmonitionStyle(const String& typeStr)
{
    String t = typeStr.trim().toUpperCase();

    // 1. Structural dimensions overrides matching GitHub layout guidelines
    this->iconsize = 0;   // Remove icons entirely to clean up the modern wiki style
    this->linewidth = 4;  // GitHub & Confluence standard 4px thick vertical accent bar
    this->margin = 18;    // Text indentation spacing from the left accent bar

    // 2. Dynamic Color Palette Mapping matching your exact layout hex criteria
    if (t == "INFO" || t == "INFORMATION") {
        currentLineColor = juce::Colour(166, 202, 236); // #A6CAEC
        currentBGColor = juce::Colour(234, 242, 250); // #EAF2FA
    }
    else if (t == "HINT") {
        currentLineColor = juce::Colour(180, 229, 162); // #B4E5A2
        currentBGColor = juce::Colour(233, 248, 228); // #E9F8E4
    }
    else if (t == "CAUTION") {
        currentLineColor = juce::Colour(244, 238, 0);   // #F4EE00
        currentBGColor = juce::Colour(255, 255, 213); // #FFFFD5
    }
    else if (t == "WARNING") {
        currentLineColor = juce::Colour(255, 192, 0);   // #FFC000
        currentBGColor = juce::Colour(253, 241, 219); // #FDF1DB
    }
    else if (t == "IMPORTANT") {
        currentLineColor = juce::Colour(255, 117, 115); // #FF7575
        currentBGColor = juce::Colour(252, 237, 228); // #FCEDE4
    }
    else if (t == "MISC") {
        currentLineColor = juce::Colour(229, 158, 221); // #E59EDD
        currentBGColor = juce::Colour(250, 234, 248); // #FAEAF8
    }
    else if (t == "OTHER") {
        currentLineColor = juce::Colour(216, 222, 228); // GitHub light gray line (#D0D7DE)
        currentBGColor = juce::Colour(242, 242, 242); // #F2F2F2
    }
    else if (t == "BLOCKQUOTE") {
        currentLineColor = juce::Colour(216, 222, 228); // GitHub light gray line (#D0D7DE)
        currentBGColor = juce::Colours::transparentBlack; // Clean transparent citation background
    }
    else {
        // Fallback safety style configuration
        currentLineColor = juce::Colours::grey;
        currentBGColor = juce::Colours::transparentBlack;
    }
}

void BarelyMLDisplay::AdmonitionBlock::parseAdmonitionMarkup(const String& line, Font font, int iconsize, int margin, int linewidth)
{
    String typeToken = line.upToFirstOccurrenceOf(": ", false, false).trim();
    String contentText = line.fromFirstOccurrenceOf(": ", false, false);

    // Build the clean attributed string layout payload support
    StringArray textLines;
    textLines.add(contentText);
    attributedString = parsePureText(textLines, font);

    // Trigger palette colors mapping setup calculations
    setupAdmonitionStyle(typeToken);

    // Enforce size bounds immediately to lock dimensions layout footprint
    int exactHeight = juce::roundToInt(getHeightRequired(static_cast<float>(getWidth())));
    setSize(getWidth(), exactHeight);
}

float BarelyMLDisplay::AdmonitionBlock::getHeightRequired(float width)
{
    TextLayout layout;

    // Deduct only ONE left vertical line bar width and padding margin to prevent character squeezing
    float availableWidth = width - (float)margin - (float)linewidth;
    layout.createLayout(attributedString, availableWidth);

    // Add 8 pixels padding total buffer (4px top + 4px bottom) to let the container breathe
    return layout.getHeight() + 8.0f;
}

void BarelyMLDisplay::AdmonitionBlock::paint(juce::Graphics& g)
{
    float totalHeight = static_cast<float>(getHeight());
    float totalWidth = static_cast<float>(getWidth());

    // Draw solid background canvas fill area if not completely transparent
    if (currentBGColor != juce::Colours::transparentBlack)
    {
        g.setColour(currentBGColor);
        //float barW = static_cast<float>(linewidth);
        //g.fillRect(0.0f, 0.0f, totalWidth - barW, totalHeight);
        g.fillRect(0.0f, 0.0f, totalWidth, totalHeight);
    }

    // Draw the vertical accent line bar covering 100% of height footprint for continuity
    g.setColour(currentLineColor);
    g.fillRect(0.0f, 0.0f, static_cast<float>(linewidth), totalHeight);

    // Setup text layout rendering engine dimensions budget
    juce::TextLayout layout;
    float textWidth = totalWidth - (float)margin - (float)linewidth;
    layout.createLayout(attributedString, textWidth);

    // Calculate precise dynamic vertical padding to perfectly center the text inside the row height
    float textActualHeight = layout.getHeight();
    float verticalMarginOffset = (totalHeight - textActualHeight) * 0.5f;

    // Safety clamp to ensure offset never drops below 0 pixels
    if (verticalMarginOffset < 0.0f)
        verticalMarginOffset = 0.0f;

    g.saveState();

    // Shift matrix drawing point past the left accent line and padding spacing margins, adding 4px vertical padding
    float xOffset = static_cast<float>(linewidth + margin);
    g.addTransform(juce::AffineTransform::translation(xOffset, verticalMarginOffset));

    // Dual-pass layer processing for inline code tags highlighting
    juce::Colour greenColor = parseHexColour((*colours)["green"]);
    juce::Colour darkGrayText = parseHexColour((*colours)["darkgray"]);

    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        const auto& line = layout.getLine(i);
        float currentX = line.lineOrigin.x;

        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
                runWidth += run->glyphs.getReference(k).width;

            if (run->colour == greenColor)
            {
                // Call centralized static background drawing function
                Block::drawInlineCodeBackground(g, currentX, runWidth, line, 2.0f);

                // Rollback layout character ink token color inside code bounds to draw in Dark Gray
                run->colour = darkGrayText;
            }
            else
            {
                // Force all generic text component rows inside the block to paint in Dark Gray
                run->colour = darkGrayText;
            }
            currentX += runWidth;
        }
    }

    // Render the text layout flawlessly over the painted backgrounds
    layout.draw(g, juce::Rectangle<float>(0.0f, 0.0f, textWidth, totalHeight));

    g.restoreState();
}

// MARK: - Table Block

BarelyMLDisplay::TableBlock::TableBlock() 
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&table, false); // we manage the content component
    viewport.setScrollBarsShown(true, false, false, true); // scroll only vertically, and only when needed
    //viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::nonHover);     // Commented as this is for Juce 8, but we want to keep Juce 6 compatibility for now. We can uncomment this when we switch to Juce 8.
    viewport.setScrollOnDragEnabled(false);
}

bool BarelyMLDisplay::TableBlock::isTableLine(const String& line) 
{
    return line.startsWith("^") || line.startsWith("|");
}

void BarelyMLDisplay::TableBlock::parseMarkup(const StringArray& lines, Font font)
{
    // Create a local copy to allow structural modifications (removing the alignment row)
    StringArray localLines = lines;

    // Detect and extract the table markdown alignment line
    // Using Array<int> instead of Array<Justification> to comply with JUCE 6.1.5 constructor constraints
    Array<int> columnAlignments;
    int alignmentLineIndex = -1;

    for (int i = 0; i < localLines.size(); ++i)
    {
        String line = localLines[i];

        // Check if the line matches a converted alignment row.
        // It must contain delimiters (^ or |) and its cells must only consist 
        // of hyphens, colons, and whitespace characters.
        if (line.containsAnyOf("^|"))
        {
            bool isAlignmentLine = true;
            StringArray cellsInLine;

            // Simulate cell splitting to inspect contents
            String tempLine = line;
            while (tempLine.containsAnyOf("^|"))
            {
                tempLine = tempLine.substring(1);
                int nextDelim = tempLine.indexOfAnyOf("^|");
                if (nextDelim >= 0)
                {
                    String cellContent = tempLine.substring(0, nextDelim);

                    // Fix delimiter if we have broken apart a link inside the alignment check
                    if (cellContent.contains("[[") && !cellContent.contains("]]") && tempLine.contains("]]"))
                    {
                        int linkEnd = tempLine.indexOf("]]");
                        int idx1 = tempLine.indexOf(linkEnd, "|");
                        int idx2 = tempLine.indexOf(linkEnd, "^");
                        nextDelim = linkEnd;
                        if (idx1 >= 0 && (idx2 < 0 || idx1 < idx2)) { nextDelim = idx1; }
                        if (idx2 >= 0 && (idx1 < 0 || idx2 < idx1)) { nextDelim = idx2; }
                        cellContent = tempLine.substring(0, nextDelim);
                    }

                    cellsInLine.add(cellContent.trim());
                    tempLine = tempLine.substring(nextDelim);
                }
                else
                {
                    break;
                }
            }

            // A valid alignment line must not be empty, and all cells must match 
            // the markdown alignment patterns (e.g., :---:, ---:, :---)
            if (cellsInLine.size() > 0)
            {
                for (auto cell : cellsInLine)
                {
                    // If a cell contains characters other than colons or hyphens, it's a regular data row
                    if (cell.retainCharacters(":-").isNotEmpty() && cell.retainCharacters(":-").trim() == cell)
                    {
                        // Valid alignment syntax, keep checking next cells
                    }
                    else
                    {
                        isAlignmentLine = false;
                        break;
                    }
                }
            }
            else
            {
                isAlignmentLine = false;
            }

            // If it is confirmed to be the alignment line, parse and store the justifications flags
            if (isAlignmentLine)
            {
                alignmentLineIndex = i;
                for (auto cell : cellsInLine)
                {
                    bool left = cell.startsWith(":");
                    bool right = cell.endsWith(":");

                    if (left && right) {
                        columnAlignments.add(Justification::centred);
                    }
                    else if (right) {
                        columnAlignments.add(Justification::right);
                    }
                    else {
                        columnAlignments.add(Justification::left); // Default or :---
                    }
                }
                break; // Alignment target found, stop searching through lines
            }
        }
    }

    // Remove the technical alignment line to prevent it from rendering as a blank row
    if (alignmentLineIndex >= 0) {
        localLines.remove(alignmentLineIndex);
    }

    // Read and process the actual data cells
    table.cells.clear();
    table.setBMLDisplay(bmlDisplay);

    for (auto line : localLines) {
        OwnedArray<Cell>* row = new OwnedArray<Cell>();
        int columnIndex = 0; // Track current column index to match stored alignment

        while (line.containsAnyOf("^|")) {
            bool isHeader = line.startsWith("^");
            line = line.substring(1); // remove left delimiter
            int nextDelimiter = line.indexOfAnyOf("^|"); // find right delimiter
            if (nextDelimiter >= 0) {
                String rawString = line.substring(0, nextDelimiter);

                // fix delimiter if we have broken apart a link
                if (rawString.contains("[[") && !rawString.contains("]]") && line.contains("]]"))
                {
                    int linkEnd = line.indexOf("]]");
                    int idx1 = line.indexOf(linkEnd, "|");
                    int idx2 = line.indexOf(linkEnd, "^");
                    nextDelimiter = linkEnd;
                    if (idx1 >= 0 && (idx2 < 0 || idx1 < idx2)) { nextDelimiter = idx1; }
                    if (idx2 >= 0 && (idx1 < 0 || idx2 < idx1)) { nextDelimiter = idx2; }
                    rawString = line.substring(0, nextDelimiter);
                }
                line = line.substring(nextDelimiter); // drop everything up to right delimiter

                // Apply the parsed column alignment (falls back to Left if out of bounds)
                Justification cellJustification = Justification::left;
                if (columnIndex < columnAlignments.size()) {
                    cellJustification = Justification(columnAlignments[columnIndex]);
                }

                String trimmed = rawString.trim();
                String cellLink;
                if (trimmed.contains("[[") && trimmed.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
                    trimmed = consumeLink(trimmed, &cellLink);
                }

                // check for image
                int width = -1;
                std::unique_ptr<Drawable> drawable;
                if (trimmed.startsWith("{{") && trimmed.endsWith("}}")) {
                    String filename = trimmed.fromFirstOccurrenceOf("{{", false, false).upToFirstOccurrenceOf("}}", false, false);
                    if (filename.contains("?")) {
                        width = filename.fromFirstOccurrenceOf("?", false, false).getIntValue();
                        filename = filename.upToFirstOccurrenceOf("?", false, false);
                    }
                    if (fileSource) {
                        drawable = fileSource->getDrawableForFilename(filename);
                        if (!drawable) {
                            trimmed += String(" File not found.");
                        }
                    }
                    else {
                        trimmed += String(" No file source.");
                    }
                }

                AttributedString attributedString = parsePureText(trimmed, isHeader ? font.boldened() : font);

                // Set the determined justification onto the AttributedString
                attributedString.setJustification(cellJustification);

                TextLayout layout;
                float availableWidth = (width > 0) ? (float)width : 1.0e7f;
                layout.createLayout(attributedString, availableWidth);

                if (width > 0 && drawable && drawable->getDrawableBounds().getWidth() > 0.f) {
                    float w = drawable->getDrawableBounds().getWidth();
                    float h = drawable->getDrawableBounds().getHeight();
                    row->add(new Cell{ attributedString, std::move(drawable), cellLink, isHeader, (float)width, width * h / w });
                }
                else {
                    // Use layout.getWidth() safely as it handles alignment internal boundaries
                    row->add(new Cell{ attributedString, std::move(drawable), cellLink, isHeader, layout.getWidth(), layout.getHeight() });
                }

                columnIndex++;
            }
        }
        table.cells.add(row);
    }

    // Compute height and width of rows
    table.columnwidths.clear();
    for (int i = 0; i < table.cells.size(); i++) {
        OwnedArray<Cell>* row = table.cells[i];
        for (int j = 0; j < row->size(); j++) {
            if (j < table.columnwidths.size()) {
                table.columnwidths.set(j, jmax(table.columnwidths[j], (*row)[j]->width));
            }
            else {
                table.columnwidths.set(j, (*row)[j]->width);
            }
        }
    }

    table.rowheights.clear();
    for (int i = 0; i < table.cells.size(); i++) {
        OwnedArray<Cell>* row = table.cells[i];
        float rowheight = 0;
        for (int j = 0; j < row->size(); j++) {
            rowheight = jmax(rowheight, (*row)[j]->height);
        }
        table.rowheights.set(i, rowheight);
    }
    table.setBounds(0, 0, getWidthRequired() + table.leftmargin + table.cellgap + 2,
        getHeightRequired(0.f) + 2);
}

float BarelyMLDisplay::TableBlock::getWidthRequired() 
{
    float width = 0;
    for (int i=0; i<table.columnwidths.size(); i++) 
    {
        width += table.columnwidths[i] + 2 * table.cellmargin + table.cellgap;
    }
    return width - table.cellgap;
}

float BarelyMLDisplay::TableBlock::getHeightRequired(float width) 
{
    // NOTE: We're ignoring width - the idea is that tables can be scrolled horizontally if necessary
    float height = 0;
    
    for (int i=0; i<table.rowheights.size(); i++) 
    {
        height += table.rowheights[i] + 2 * table.cellmargin + table.cellgap;
    }
    return height-table.cellgap;
}

void BarelyMLDisplay::TableBlock::resized() 
{
    viewport.setBounds(getLocalBounds());
}

void BarelyMLDisplay::TableBlock::Table::paint(juce::Graphics& g)
{
    // Retrieve the target colors dynamically from the display palette strings
    juce::Colour strikeColor = juce::Colour(0xFFC00101); // Default fallback for strikethrough text
    juce::Colour gridColor = juce::Colour(0xFFD0D7DE); // Default fallback for table grid (Github standard)
    juce::Colour EvenRowsBkgdColor = juce::Colour(0xFFF6F8FA); // Default fallback for table even rows (Github standard)

    if (bmlDisplay != nullptr)
    {
        if (bmlDisplay->colours.containsKey("strikecolour"))
            strikeColor = Block::parseHexColourStatic(bmlDisplay->colours["strikecolour"], strikeColor);

        // Read your custom table header color definition from the palette dictionary
        if (bmlDisplay->colours.containsKey("tableGrid"))
            gridColor = Block::parseHexColourStatic(bmlDisplay->colours["tableGrid"], gridColor);

        // Read your custom alternating table row color definition from the palette dictionary
        if (bmlDisplay->colours.containsKey("tableEvenRowBkgd"))
            EvenRowsBkgdColor = Block::parseHexColourStatic(bmlDisplay->colours["tableEvenRowBkgd"], EvenRowsBkgdColor);
    }

    float y = 0.f; // Y coordinate of the current row's top-left corner

    for (int i = 0; i < cells.size(); i++)
    {
        float x = leftmargin; // X coordinate of the current cell's top-left corner
        OwnedArray<Cell>* row = cells[i]; // Get the current row

        for (int j = 0; j < row->size(); j++)
        {
            Cell* c = (*row)[j]; // Get the current cell

            // Set the background color explicitly before filling the rectangle area
            if (c->isHeader || i == 0)
            {
                g.setColour(juce::Colours::transparentBlack);   // Github standard: header is transparent
            }
            else
            {
                // Alternating row background: tableBG for odd rows, transparent for even rows
                if (i % 2 != 0)
                {
                    g.setColour(EvenRowsBkgdColor);
                }
                else
                {
                    g.setColour(juce::Colours::transparentBlack);
                }
            }

            // Fill the cell background and draw the grid

            // Cast coordinates into strict integers to align perfectly with the screen pixel grid
            int xInt = juce::roundToInt(x);
            int yInt = juce::roundToInt(y);
            int wInt = juce::roundToInt(x + columnwidths[j] + 2 * cellmargin + cellgap) - xInt;
            int hInt = juce::roundToInt(y + rowheights[i] + 2 * cellmargin + cellgap) - yInt;

            // Fill the cell background area cleanly up to the borders
            g.fillRect(xInt, yInt, wInt, hInt);            

            // Render Cell Outline Borders Layer (GitHub light gray gridlines)
            g.setColour(gridColor);

            // Draw only Top and Left edges for every cell to prevent overlapping lines
            g.fillRect(xInt, yInt, wInt, 1); // Top border line
            g.fillRect(xInt, yInt, 1, hInt); // Left border line

            // Draw Right edge ONLY for the last column to close the outer table frame
            if (j == row->size() - 1)
            {
                //g.drawLine((float)(xInt + wInt), (float)yInt, (float)(xInt + wInt), (float)(yInt + hInt), 1.0f);
                g.fillRect(xInt + wInt - 1, yInt, 1, hInt);
            }

            // Draw Bottom edge ONLY for the last row to close the outer table frame
            if (i == cells.size() - 1)
            {
                //g.drawLine((float)xInt, (float)(yInt + hInt), (float)(xInt + wInt), (float)(yInt + hInt), 1.0f);
                g.fillRect(xInt, yInt + hInt - 1, wInt, 1);
            }

            // Define the bounding area for the cell content
            Rectangle<float> destArea = Rectangle<float>(x + cellmargin, y + cellmargin, columnwidths[j], rowheights[i]);

            if (c->drawable)
            {
                c->drawable->drawWithin(g, destArea, RectanglePlacement::centred, 1.0f);
            }
            else
            {
                // 1. Instantiate the precise geometry layout matrix for the table cell text
                juce::TextLayout layout;
                float availableWidth = columnwidths[j];
                layout.createLayout(c->s, jmax(1.0f, availableWidth));

                // Fetch the alignment metadata from the cell's AttributedString configuration
                juce::Justification cellJustification = c->s.getJustification();

                // Dynamically fetch green color definition for inline code scanning
                juce::Colour greenColor = juce::Colours::green;
                if (bmlDisplay != nullptr && bmlDisplay->colours.containsKey("green"))
                    greenColor = Block::parseHexColourStatic(bmlDisplay->colours["green"], greenColor);

                // --- FIRST PASS: Render inline code background boxes BEFORE drawing characters ---
                for (int l = 0; l < layout.getNumLines(); ++l)
                {
                    const auto& line = layout.getLine(l);

                    // --- MANUAL JUCE 6 ALIGNMENT CALCULATOR ---
                    float alignmentOffset = 0.0f;
                    float lineWidth = line.getLineBounds().getWidth();

                    if (cellJustification.testFlags(juce::Justification::centred))
                        alignmentOffset = (availableWidth - lineWidth) * 0.5f;
                    else if (cellJustification.testFlags(juce::Justification::right))
                        alignmentOffset = availableWidth - lineWidth;

                    // Reposition X reference coordinate relative to the cell inner bounds
                    float currentX = line.lineOrigin.x + x + cellmargin + alignmentOffset;

                    for (int r = 0; r < line.runs.size(); ++r)
                    {
                        auto* run = line.runs.getUnchecked(r);
                        if (run == nullptr) continue;

                        float runWidth = 0.0f;
                        for (int k = 0; k < run->glyphs.size(); ++k)
                            runWidth += run->glyphs.getReference(k).width;

                        if (run->colour == greenColor)
                        {
                            // Adjust the line layout coordinates system to align with cell absolute Y viewport
                            juce::TextLayout::Line translatedLine = line;
                            translatedLine.lineOrigin.y += y + cellmargin;

                            // Call centralized static background drawing system
                            Block::drawInlineCodeBackground(g, currentX, runWidth, translatedLine, 2.0f);

                            // Style text layout color token matching darkgray palette standard
                            run->colour = Block::parseHexColourStatic(bmlDisplay->colours["darkgray"], juce::Colours::darkgrey);
                        }
                        currentX += runWidth;
                    }
                }

                // 2. Render the final text characters matrix onto the freshly painted grey rectangles
                // FIX: layout.draw guarantees layout alignment and color mutations apply properly!
                layout.draw(g, destArea);

                // --- SECOND PASS: Post-paint engine layer to overlay structural strikethrough lines ---
                for (int l = 0; l < layout.getNumLines(); ++l)
                {
                    const auto& line = layout.getLine(l);

                    float alignmentOffset = 0.0f;
                    float lineWidth = line.getLineBounds().getWidth();

                    if (cellJustification.testFlags(juce::Justification::centred))
                        alignmentOffset = (availableWidth - lineWidth) * 0.5f;
                    else if (cellJustification.testFlags(juce::Justification::right))
                        alignmentOffset = availableWidth - lineWidth;

                    float currentX = line.lineOrigin.x + x + cellmargin + alignmentOffset;
                    float lineHeight = line.getLineBounds().getHeight();

                    for (int r = 0; r < line.runs.size(); ++r)
                    {
                        auto* run = line.runs.getUnchecked(r);
                        if (run == nullptr) continue;

                        float runWidth = 0.0f;
                        for (int k = 0; k < run->glyphs.size(); ++k)
                            runWidth += run->glyphs.getReference(k).width;

                        if (run->colour == strikeColor)
                        {
                            g.setColour(run->colour);

                            // Calculate the accurate Y drawing coordinate by offsetting layout text baseline
                            float yPosition = y + cellmargin + line.lineOrigin.y - (lineHeight * 0.25f);

                            // Draw the line segment horizontally in the middle of the characters
                            g.drawLine(currentX, yPosition, currentX + runWidth, yPosition, 1.0f);
                        }
                        currentX += runWidth;
                    }
                }
            }
            // Move coordinates rightwards for the next column cell
            x += columnwidths[j] + 2 * cellmargin + cellgap;
        }

        // Move coordinates downwards for the next row cell
        y += rowheights[i] + 2 * cellmargin + cellgap;
    }
}

void BarelyMLDisplay::TableBlock::Table::mouseDown(const MouseEvent& event)
{
    mouseDownPosition = event.position;     // keep track of position
}

void BarelyMLDisplay::TableBlock::Table::mouseUp(const MouseEvent& event) 
{
    String link;

    // Find link for mouseDownPosition
    float mdy = mouseDownPosition.y;
    float mdx = mouseDownPosition.x;
    float y = 0.f;                    // Y coordinate of row's top edge

    for (int i=0; i<cells.size(); i++) 
    {
        if (mdy>=y && mdy<y+rowheights[i] + 2 * cellmargin) 
        {
            float x = leftmargin;         // X coordinate of cell's left edge
            for (int j=0; j<cells[i]->size(); j++) 
            {
                if (mdx>=x && mdx<x+columnwidths[j] + 2 * cellmargin) 
                {
                    link = (*cells[i])[j]->link;
                }
                // move one cell to the right
                x += columnwidths[j] + 2 * cellmargin + cellgap;
            }
        }
        // move to next row
        y += rowheights[i] + 2 * cellmargin + cellgap;
    }

    // If we have a link...
    if (link.isNotEmpty()) 
    {          
        float distance = event.position.getDistanceFrom(mouseDownPosition);
        if (distance < 20) 
        {            // ...and we're not scrolling...
            jassert(bmlDisplay);
            bmlDisplay->handleURL(link);  // ...let bmlDisplay handle URL.
        }
    }
}

void BarelyMLDisplay::TableBlock::Table::mouseMove(const juce::MouseEvent& event)
{
    // 1. Retrieve the target link color from the parent display's palette
    juce::Colour linkColor = juce::Colours::blue; // Safe fallback color
    if (bmlDisplay != nullptr && bmlDisplay->colours.containsKey("linkcolour"))
    {
        linkColor = Block::parseHexColourStatic(bmlDisplay->colours["linkcolour"], linkColor);
    }

    juce::Point<int> localPoint = event.getPosition();
    float y = 0.f; // Y coordinate tracker for table rows

    // 2. Iterate through all rows and cells to detect geometric collision with a link
    for (int i = 0; i < cells.size(); i++)
    {
        float x = leftmargin; // X coordinate tracker for table columns
        OwnedArray<Cell>* row = cells[i];

        for (int j = 0; j < row->size(); j++)
        {
            Cell* c = (*row)[j];

            // Define the bounding box area of the current cell text
            Rectangle<float> destArea = Rectangle<float>(x + cellmargin, y + cellmargin, columnwidths[j], rowheights[i]);

            // Run the check only if the mouse is physically inside this cell and it contains text
            if (destArea.contains(localPoint.toFloat()) && !c->drawable)
            {
                TextLayout layout;
                // Match the exact text bounding box used during the paint operation (minus margins)
                float availableWidth = columnwidths[j];
                layout.createLayout(c->s, jmax(1.0f, availableWidth));

                Justification cellJustification = c->s.getJustification();

                for (int l = 0; l < layout.getNumLines(); ++l)
                {
                    const auto& line = layout.getLine(l);

                    // --- MANUAL JUCE 6 ALIGNMENT CALCULATOR ---
                    float alignmentOffset = 0.0f;
                    float lineWidth = line.getLineBounds().getWidth();

                    if (cellJustification.testFlags(juce::Justification::centred))
                    {
                        alignmentOffset = (availableWidth - lineWidth) * 0.5f;
                    }
                    else if (cellJustification.testFlags(juce::Justification::right))
                    {
                        alignmentOffset = availableWidth - lineWidth;
                    }

                    float currentX = line.lineOrigin.x + x + cellmargin + alignmentOffset;
                    float lineHeight = line.getLineBounds().getHeight();

                    for (int r = 0; r < line.runs.size(); ++r)
                    {
                        auto* run = line.runs.getUnchecked(r);
                        if (run == nullptr) continue;

                        // Calculate the precise width of the text fragment based on its glyphs
                        float runWidth = 0.0f;
                        for (int k = 0; k < run->glyphs.size(); ++k)
                        {
                            runWidth += run->glyphs.getReference(k).width;
                        }

                        // Check if the current segment matches the link color
                        if (run->colour == linkColor)
                        {
                            // Calculate the accurate collision rectangle of the link words
                            // using lineOrigin.y and subtracting the line ascent for precise vertical collision
                            juce::Rectangle<float> runBounds(currentX,
                                y + cellmargin + line.lineOrigin.y - line.ascent,
                                runWidth,
                                lineHeight);

                            // If the mouse cursor hits the link range, trigger the hand cursor and exit
                            if (runBounds.contains(localPoint.toFloat()))
                            {
                                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                                return;
                            }
                        }
                        currentX += runWidth;
                    }
                }
            }
            // Offset X to move to the next column cell
            x += columnwidths[j] + 2 * cellmargin + cellgap;
        }
        // Offset Y to move to the next row item
        y += rowheights[i] + 2 * cellmargin + cellgap;
    }

    // 3. Fallback to normal arrow cursor if no link is currently hovered inside the table
    setMouseCursor(juce::MouseCursor::NormalCursor);
}


void BarelyMLDisplay::TableBlock::Table::mouseExit(const juce::MouseEvent& event)
{
    // Reset to the standard arrow cursor when the mouse leaves the table bounds completely
    setMouseCursor(juce::MouseCursor::NormalCursor);
}


// MARK: - Image Block
bool BarelyMLDisplay::ImageBlock::isImageLine(const String& line) 
{
    return (line.startsWith("{{") && line.trim().endsWith("}}")) || // either just an image...
         (line.startsWith("[[") && line.trim().endsWith("]]") &&  // ...or a link around...
          line.contains("{{") && line.fromFirstOccurrenceOf("{{", false, false).contains("}}")); // ...an image.
}

void BarelyMLDisplay::ImageBlock::parseImageMarkup(const String& line, FileSource* fileSource) 
{
    String filename = line.fromFirstOccurrenceOf("{{", false, false).upToFirstOccurrenceOf("}}", false, false);

    if (filename.contains("?")) 
    {
        maxWidth = filename.fromFirstOccurrenceOf("?", false, false).getIntValue();
        filename = filename.upToFirstOccurrenceOf("?", false, false);
    }
    else
    {
        maxWidth = -1;
    }

    if (fileSource) 
    {
        drawable = fileSource->getDrawableForFilename(filename);
    }
    else
    {
        imageMissingMessage.append("no file source. ", Font(14), defaultColour);
    }

    if (!drawable) 
    {
        imageMissingMessage.append(filename + " not found.", Font(14), defaultColour);
    }
}

float BarelyMLDisplay::ImageBlock::getHeightRequired(float width) 
{
    if (drawable && drawable->getDrawableBounds().getWidth()>0.f) 
    {
        float w = drawable->getDrawableBounds().getWidth();
        float h = drawable->getDrawableBounds().getHeight();

        if (maxWidth>0) 
        {
            return jmin((float)maxWidth,width)*h/w;
        } 
        else
        {
            return width*h/w;
        }
    }
    else
    {
        return 20.f;
    }
}

void BarelyMLDisplay::ImageBlock::paint(juce::Graphics& g) 
{
    if (drawable) 
    {
        float w = getWidth();

        if (maxWidth>0) 
        {
            w = jmin((float)maxWidth,w);
        }
        drawable->drawWithin(g, Rectangle<float>(0, 0, w, getHeight()), RectanglePlacement::centred, 1.0f);
    } 
    else 
    {
        g.setColour(defaultColour);
        g.drawRect(getLocalBounds());
        g.drawLine(0, 0, getWidth(), getHeight());
        g.drawLine(getWidth(), 0, 0, getHeight());
        imageMissingMessage.draw(g, getLocalBounds().reduced(5,5).toFloat());
    }
}

void BarelyMLDisplay::ImageBlock::resized() 
{
}


// MARK: - List Item

bool BarelyMLDisplay::ListItem::isListItem(const String& line) 
{
    return (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) || (line.indexOf("- ")>=0 && !line.substring(0, line.indexOf("- ")).containsNonWhitespaceChars());
}

void BarelyMLDisplay::ListItem::parseItemMarkup(const String& line, Font font, int indentPerSpace, int gap) 
{
    this->gap  = gap;
    label.clear();

    // --- First check for tasks. GITHUB TASK LIST DETECTION ---
    String checkTask = line.trimStart();
    if (checkTask.startsWith("- ") || checkTask.startsWith("* "))
        checkTask = checkTask.substring(2).trimStart();

    // Reset task variables
    this->isTask = false;
    this->isTaskChecked = false;

    // Create a local modifiable copy
    String cleanedLine = line;

    if (checkTask.startsWith("[ ]"))
    {
        this->isTask = true;
        this->isTaskChecked = false;
        cleanedLine = line.replace("[ ]", "").replace("  ", " ");
    }
    else if (checkTask.startsWith("[x]") || checkTask.startsWith("[X]"))
    {
        this->isTask = true;
        this->isTaskChecked = true;
        cleanedLine = line.replace("[x]", "").replace("[X]", "").replace("  ", " ");
    }

    int dotidx = cleanedLine.indexOf(". ");                      // find dot+space in line
    String beforedot = cleanedLine.substring(0, dotidx);         // find out if before the dot...
    String lbl = beforedot.trimStart();                   // ...there's only whitespace...

    if (dotidx>0 && lbl.containsOnly("0123456789"))      // ...and at least one number.
    {
        label.append(lbl+".", font, defaultColour);          // create label
        // parse item text (everything after the dot)
        attributedString = parsePureText(cleanedLine.substring(dotidx+2).trimStart(), font);
        // use number of whitespace characters to determine indent
        indent = indentPerSpace * (beforedot.length()-lbl.length());
    } 
    else    // otherwise try unordered list: 
    {                                              
        int hyphenidx = cleanedLine.indexOf("- ");                 // find hyphen+space in line
        String beforehyphen = cleanedLine.substring(0, hyphenidx); // find out if before the hyphen...
        
        if (!beforehyphen.containsNonWhitespaceChars())     // ...there's only whitespace.
        {   
            // parse item text (everything after the hyphen)
            attributedString = parsePureText(cleanedLine.substring(hyphenidx+2).trimStart(), font);
            // use number of whitespace characters to determine indent
            indent = indentPerSpace * beforehyphen.length();
            // create label TODO: have bullet character depend on indent
            label.append(CharPointer_UTF8("•"), font, defaultColour);
        } 
        else 
        {  // if everything fails, interpret as regular text without label
            indent = 0;
            attributedString = parsePureText(cleanedLine, font);
        }
    }
}

float BarelyMLDisplay::ListItem::getHeightRequired(float width) 
{
    TextLayout layout;
    layout.createLayout(attributedString, width-indent-gap);
    return layout.getHeight();
}

void BarelyMLDisplay::ListItem::paint(juce::Graphics& g)
{
    float totalHeight = static_cast<float>(getHeight());

    // --- GITHUB TASK LIST SKIN CONTROL ---
    // If it's a task, we dynamically calculate and inject the checkbox geometry
    // and adjust the indentation variables so the rest of your layout aligns perfectly.
    int localIndent = indent;
    int localGap = gap;

    if (isTask)
    {
        float boxSize = 13.0f;
        float boxX = 2.0f; // Align cleanly with the list layout margins
        float boxY = (totalHeight - boxSize) * 0.5f; // Perfect vertical centering
        float cornerSize = 2.5f;

        if (isTaskChecked)
        {
            // Task Completed: Filled gray box container with white checkmark glyph
            g.setColour(juce::Colour(200, 205, 210));
            g.fillRoundedRectangle(boxX, boxY, boxSize, boxSize, cornerSize);

            g.setColour(juce::Colours::white);
            auto checkmarkPath = juce::Path();
            checkmarkPath.startNewSubPath(boxX + 3.0f, boxY + boxSize * 0.5f + 0.5f);
            checkmarkPath.lineTo(boxX + boxSize * 0.45f, boxY + boxSize - 3.5f);
            checkmarkPath.lineTo(boxX + boxSize - 3.0f, boxY + 3.5f);
            g.strokePath(checkmarkPath, juce::PathStrokeType(1.8f));
        }
        else
        {
            // Task Pending: Rounded clean empty border layout frame
            g.setColour(juce::Colour(210, 215, 220));
            g.drawRoundedRectangle(boxX + 0.5f, boxY + 0.5f, boxSize - 1.0f, boxSize - 1.0f, cornerSize, 1.2f);
        }

        // Override local layout offsets to leave a clean 22px gutter margin for the text
        localIndent = 22;
        localGap = 0;
    }
    else
    {
        // 1. Render the list item bullet or number label natively
        label.draw(g, getLocalBounds().withTrimmedLeft(indent).toFloat());
    }

    // 2. Safely extract target text rendering colors configuration
    juce::Colour strikeColor = juce::Colours::red;
    if (colours != nullptr && colours->containsKey("strikecolour"))
        strikeColor = parseHexColour((*colours)["strikecolour"]);

    juce::Colour greenColor = juce::Colours::green;
    if (colours != nullptr && colours->containsKey("green"))
        greenColor = parseHexColour((*colours)["green"]);

    // 3. Instantiate the precise geometry layout matrix accounting for bullet constraints
    juce::TextLayout layout;
    layout.createLayout(attributedString, getWidth() - indent - gap);

    // 4. FIRST SCAN LOOP: Render inline code background boxes BEFORE drawing the text characters
    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        const auto& line = layout.getLine(i);
        float currentX = line.lineOrigin.x + indent + gap;

        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
                runWidth += run->glyphs.getReference(k).width;

            if (run->colour == greenColor)
            {
                // Call centralized static background drawing function
                drawInlineCodeBackground(g, currentX, runWidth, line, 2.0f);

                // Override layout text token color to Dark Gray for professional contrast reading
                run->colour = parseHexColour((*colours)["darkgray"]);
            }
            currentX += runWidth;
        }
    }

    // 5. Render the final text characters matrix 
    // Create the destination rectangle bounding box for text drawing
    juce::Rectangle<float> textBounds = getLocalBounds().withTrimmedLeft(indent + gap).toFloat();

    if (isTask)
    {
        // Shift the text drawing area 8 pixels down to align perfectly with the first checkbox
        textBounds.translate(0.0f, 8.0f);
    }

    layout.draw(g, textBounds);

    // 6. SECOND SCAN LOOP: Post-paint engine layer to overlay structural strikethrough lines
    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        const auto& line = layout.getLine(i);
        float currentX = line.lineOrigin.x + indent + gap;
        float lineHeight = line.getLineBounds().getHeight();

        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
                runWidth += run->glyphs.getReference(k).width;

            if (run->colour == strikeColor)
            {
                g.setColour(run->colour);
                float yPosition = line.lineOrigin.y - (lineHeight * 0.25f);
                g.drawLine(currentX, yPosition, currentX + runWidth, yPosition, 1.0f);
            }
            currentX += runWidth;
        }
    }
}

bool BarelyMLDisplay::ListItem::containsLinkAt(juce::Point<int> localPoint)
{
    juce::Colour linkColor = juce::Colours::blue;
    if (colours != nullptr && colours->containsKey("linkcolour"))
        linkColor = parseHexColour((*colours)["linkcolour"]);

    juce::TextLayout layout;
    layout.createLayout(attributedString, static_cast<float>(getWidth()));
    float listItemVisualOffset = static_cast<float>(this->indent + this->gap);

    for (int i = 0; i < layout.getNumLines(); ++i)
    {
        auto& line = layout.getLine(i);
        float currentX = line.lineOrigin.x + listItemVisualOffset;
        float lineHeight = line.getLineBounds().getHeight();

        for (int j = 0; j < line.runs.size(); ++j)
        {
            auto* run = line.runs.getUnchecked(j);
            if (run == nullptr) continue;

            float runWidth = 0.0f;
            for (int k = 0; k < run->glyphs.size(); ++k)
                runWidth += run->glyphs.getReference(k).width;

            if (run->colour == linkColor)
            {
                juce::Rectangle<float> runBounds(currentX,
                    line.lineOrigin.y - line.getLineBounds().getHeight(),
                    runWidth,
                    lineHeight);

                if (runBounds.contains(localPoint.toFloat()))
                    return true;
            }
            currentX += runWidth;
        }
    }
    return false;
}

void BarelyMLDisplay::ListItem::mouseMove(const juce::MouseEvent& event)
{
    if (link.isEmpty())
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        return;
    }

    if (containsLinkAt(event.getPosition()))
    {
        if (!isHovered)
        {
            isHovered = true;
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            repaint();
        }
    }
    else
    {
        if (isHovered)
        {
            isHovered = false;
            setMouseCursor(juce::MouseCursor::NormalCursor);
            repaint();
        }
    }
}

void BarelyMLDisplay::ListItem::mouseExit(const juce::MouseEvent& event)
{
    if (isHovered)
    {
        isHovered = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}