/*
  ==============================================================================
 
 BarelyML - Sunny Synths.h
 Author:  Dominique Bontemps - Sunny Synths

 Different adaptations to extend the possibilities of the library (see below)

 Version: 0.8 17 June 2026
 Version: 0.7 10 June 2026
 Version: 0.6 06 June 2026
 Version: 0.5 01 June 2026
 Version: 0.4 22 May 2026
  
 ==============================================================================
 
 BarelyML.h
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

 BarelyML.h and BarelyML.cpp implement the BarelyML markup language, which
 supports the following syntax:
 
 Headings

 # Level 1 Heading
 ## Level 2 Heading
 ### Level 3 Heading
 #### Level 4 Heading
 ##### Level 5 Heading


 Bold and Italic; (new) Strikethrough

 *Italic Text*
 _Italic Text_
 **Bold Text**
 __Bold Text__
 ***Bold and Italic Text***
 ___Bold and Italic Text___
 ~~Strikethrough Text~~

 Unordered lists with hyphens. List items can contain links, inline code and other inline formatting.

 - Item 1
 - Item 2


 Ordered lists with numbers. List items can contain links, inline code and other inline formatting.

 1. Item 1
 2. Item 2


 Tables

 | Header 1      | Header 2          |
 | --- | --- | --- |
 | Regular cell  | {{image.svg?100}} |
 | Regular cell | Regular cell      |
 
 Cells can contain links, inline code, images or text.
 Line breaks within cell text: \\

 (new) column alignment with colons in the second row of the table under the header row:
 | :--- | :---: | ---: |

 Font Colour

 <c:red>Red Text</c>
 <c#FFFFFF>White Text</c>

 Colour names supported by default (CGA 16-colour palette with some extensions):
 black,blue,green,cyan,red,magenta,brown,lightgray,
 darkgray,lightblue,lightgreen,lightcyan,lightred,lightmagenta,yellow,white,
 orange, pink, darkyellow, purple, gray, linkcolour (by default set to blue)
 (the idea is that there will be the option to provide a custom colour definition object)

 (new) Code

 Inline code: `code`

 Code blocks with "```" as tag on a line before and a line after the actual code. 
 The content of the code block is rendered in a monospace font and with a GitHub-like gray background. 

 Images

 {{image-filename.jpg?200}}
 
 The number after the "?" is the maximum width (optional).
 
 
 Links
 
 [[https://mnsp.ch|My Website]]
 
 
 Admonitions (completely adapted)
 !!! TYPE
     This is the content of the admonition block.
     It can contain multiple paragraphs and other elements like lists, etc.
	 Closure tag is !!! on a new line.
	 The type can be one of the following:

 INFO: This is an info paragraph (blue).
 HINT: This is a hint paragraph (green).
 IMPORTANT: This is an important paragraph (red).
 CAUTION: This is a caution paragraph (yellow).
 WARNING: This is a warning paragraph (orange).
 (new) OTHER: This is an "other" paragraph (gray).
 (new) MISC: This is a miscellaneous paragraph (purple).

 (new) Horizontal rule
 ---

 (new) Blockquote
 > This is a blockquote.

 
 NOTE: The conversion methods FROM OTHER FORMATS TO BarelyML are incomplete,
       but work for most simple documents. If you have a use case that doesn't
       work yet, please let me know via GitHub or the JUCE forum and I'll try
       to make it work.
  
       The conversion methods FROM BarelyML TO OTHER FORMATS on the other hand
       are extremely minimal and only used in the demo application to keep the
       UI from doing weird stuff when switching the markdown language. For now
       I don't see any other use, so don't count on this becoming a feature.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

//==============================================================================
class BarelyMLDisplay  : public juce::Component
{
    public:
        BarelyMLDisplay();
        ~BarelyMLDisplay() override;
  
        // MARK: - juce::Component Methods
        void paint (juce::Graphics&) override;
        void resized() override;
  
        // MARK: - Parameters
        void setFont(juce::Font newFont) 
        { 
            this->font = newFont; 
        };
        void setMargin(int newMargin) 
        { 
            this->margin = newMargin; 
        };
        void setColours(juce::StringPairArray newColours) 
        { 
            this->colours = newColours; 
            setMarkupString(markupString); 
        };
        void setBGColour(juce::Colour newBg) 
        { 
            this->bg = newBg; 
            setMarkupString(markupString); 
        };
        void setTableColours(juce::Colour newTableBG, juce::Colour newTableBGHeader) 
        { 
            tableBG = newTableBG; 
            tableBGHeader = newTableBGHeader; 
            setMarkupString(markupString); 
        };
        void setTableMargins(int newTableMargin, int newGap) 
        { 
            tableMargin = newTableMargin; 
            tableGap = newGap; 
            setMarkupString(markupString); 
        };
        void setListIndents(int newIndentPerSpace, int newLabelGap) 
        {
            this->indentPerSpace = newIndentPerSpace;
            this->labelGap = newLabelGap;
            setMarkupString(markupString);
        };
        void setAdmonitionSizes(int newIconSize, int newAdMargin, int newAdLineWidth) 
        {
            this->iconsize = newIconSize;
            this->admargin = newAdMargin;
            this->adlinewidth = newAdLineWidth;
            setMarkupString(markupString);
        };

        // MARK: - Format Conversion (static methods)
        static juce::String convertFromMarkdown(juce::String md);
        static juce::String convertToMarkdown(juce::String bml);

        static juce::String convertFromDokuWiki(juce::String dw);
        static juce::String convertToDokuWiki(juce::String bml);
  
        static juce::String convertFromAsciiDoc(juce::String ad);
        static juce::String convertToAsciiDoc(juce::String bml);

        // MARK: - Content
        void setMarkupString(juce::String s);
        void setMarkdownString(juce::String md) { setMarkupString(convertFromMarkdown(md)); }
        void setDokuWikiString(juce::String dw) { setMarkupString(convertFromDokuWiki(dw)); }
        void setAsciiDocString(juce::String ad) { setMarkupString(convertFromAsciiDoc(ad)); }

        // MARK: - File Handling (for images)
        class FileSource 
        {
            public:
                virtual ~FileSource() {};
                virtual std::unique_ptr<juce::Drawable> getDrawableForFilename(juce::String filename) = 0;
        };
  
        void setFileSource(FileSource* fs) { fileSource = fs; }
  
        // MARK: - URL Handling (for custom link types)
        class URLHandler 
        {
            public:
                virtual ~URLHandler() {};
                virtual bool handleURL(juce::String url) = 0; // returns true if it handled the URL
        };
  
        void setURLHandler(URLHandler* uh) { urlHandler = uh; }
        void handleURL(juce::String url) 
        {
            // Check if URL handler exists and can handle our URL...
            if (!urlHandler || !urlHandler->handleURL(url)) 
            {
                // ...and if not, let JUCE's URL class handle it.
                juce::URL(url).launchInDefaultBrowser();
            }
        }

    private:

        // MARK: - Blocks
        class Block : public Component
        {
            public:
                Block ()  { colours = nullptr; defaultColour = juce::Colours::black; }
                // static utility methods
                static juce::Colour parseHexColourStatic(juce::String s, juce::Colour defaultColour);
                static bool containsLink(juce::String line);
                // Common functionalities for all blocks
                juce::String consumeLink(juce::String line, juce::String* link = nullptr);
                virtual void parseMarkup(const juce::StringArray& lines, juce::Font font) {};
                virtual float getHeightRequired(float width) = 0;
                void setColours(juce::StringPairArray* c) 
                { 
                    colours = c;
                    defaultColour = parseHexColour((*colours)["default"]);
                };
                virtual bool canExtendBeyondMargin() { return false; }; // for tables
                virtual bool containsLinkAt(juce::Point<int> localPoint) { return false; }
                virtual juce::String toString() const { return {}; }
                // Centralized function to render the GitHub-like inline code gray background
                static void drawInlineCodeBackground(juce::Graphics& g, float currentX, float runWidth, const juce::TextLayout::Line& line, float verticalPadding = 2.0f);

                // mouse handlers for clicking on links
                bool isHovered = false;
                void mouseDown(const juce::MouseEvent& event) override;
                void mouseUp(const juce::MouseEvent& event) override;
                //void mouseEnter(const juce::MouseEvent& event) override;
                void mouseExit(const juce::MouseEvent& event) override;
                void mouseMove(const juce::MouseEvent& event) override;

                void setBMLDisplay(BarelyMLDisplay* bd) { bmlDisplay = bd; }

            protected:
                juce::String link;
                juce::AttributedString parsePureText(const juce::StringArray& lines, juce::Font font, bool addNewline = true);
                juce::Colour defaultColour;
                juce::Colour currentColour;
                juce::StringPairArray* colours;
                juce::Colour parseHexColour(juce::String s);
                BarelyMLDisplay* bmlDisplay;

            private:
                juce::Point<float> mouseDownPosition;
        };
  
        class TextBlock  : public Block
        {
            public:
                void parseMarkup(const juce::StringArray& lines, juce::Font font) override;
                float getHeightRequired(float width) override;
                void paint(juce::Graphics&) override;
                bool containsLinkAt(juce::Point<int> localPoint) override;
                juce::String toString() const override { return attributedString.getText(); }
                // To capture the click on the Copy button for code blocks, we need to override mouseDown at the TextBlock level, not at the Block level, otherwise we would consume all clicks on all blocks and that would break links and other interactions. So we need to call the base class mouseDown from here when we don't detect a click on the Copy button.
				// Mouse move is used to change the mouse cursor to a pointing hand cursor when hovering over the copy code button, so we need to override it here as well to check for links in the TextBlock content.
                void mouseDown(const juce::MouseEvent& e) override;
                void mouseMove(const juce::MouseEvent& event) override;

            private:
                juce::AttributedString attributedString;
                bool isCodeBlockClosed = false; // Used to track code block closure state
        };
  
        class AdmonitionBlock  : public Block
        {
            public:
                static bool isAdmonitionLine(const juce::String& line);
                void setupAdmonitionStyle(const String& typeStr);
                void parseAdmonitionMarkup(const juce::String& line, juce::Font font, int newIconSize, int newMargin, int newLineWidth);
                float getHeightRequired(float width) override;
                void paint(juce::Graphics&) override;

            private:
                juce::AttributedString attributedString;
                enum ParagraphType { info, hint, important, caution, warning };
                ParagraphType type;
                int iconsize, margin, linewidth;
                bool isBlockquote = false;
                juce::Colour currentLineColor;
                juce::Colour currentBGColor;
        };
  
        class TableBlock : public Block
        {
            public:
                TableBlock ();
                static bool isTableLine(const juce::String& line);
                void parseMarkup(const juce::StringArray& lines, juce::Font font) override;
                float getWidthRequired();
                float getHeightRequired(float width) override;
                void resized() override;
                void setBGColours(juce::Colour bg, juce::Colour bgHeader) 
                {
                    table.bg = bg;
                    table.bgHeader = bgHeader;
                }
                void setMargins(int margin, int gap, int leftmargin) 
                {
                    table.cellmargin = margin;
                    table.cellgap = gap;
                    table.leftmargin = leftmargin;
                }
                bool canExtendBeyondMargin() override { return true; };
                void setFileSource(FileSource* fs) { fileSource = fs; };

            private:
                FileSource* fileSource;
	            //juce::Array<int> columnAlignments; // To store table cell alignments (0=left, 1=center, 2=right) initialized based on the header row
                typedef struct 
                {
                    juce::AttributedString s;
                    std::unique_ptr<juce::Drawable> drawable;
                    juce::String link;
                    bool  isHeader;
                    float width;
                    float height;
                } Cell;
                class InnerViewport : public juce::Viewport 
                {
                    public:
                        // Override the mouse event methods to forward them to the parent Viewport
                        void mouseDown(const juce::MouseEvent& e) override 
                        {
                            if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) 
                            {
                                juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
                                parent->mouseDown(ep);
                            }
                            juce::Viewport::mouseDown(e);
                        }
                        void mouseUp(const juce::MouseEvent& e) override 
                        {
                            if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) 
                            {
                                juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
                                parent->mouseUp(ep);
                            }
                            juce::Viewport::mouseUp(e);
                        }
                        void mouseDrag(const juce::MouseEvent& e) override 
                        {
                            if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) 
                            {
                                juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
                                parent->mouseDrag(ep);
                            }
                            juce::Viewport::mouseDrag(e);
                        }
                        // Override mouseWheelMove to forward events to the parent Viewport
                        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override 
                        {
                            juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>();
                            if (parent != nullptr) 
                            {
                                juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
                                parent->mouseWheelMove(ep, wheel);
                            }
                            juce::Viewport::mouseWheelMove(e, wheel);
                        }
                };
                class Table : public juce::Component 
                {
                    public:
                        void paint(juce::Graphics&) override;
                        juce::OwnedArray<juce::OwnedArray<Cell>> cells;
                        juce::Array<float> columnwidths;
                        juce::Array<float> rowheights;
                        juce::Colour bg, bgHeader;
                        int cellmargin, cellgap, leftmargin;
                        void mouseDown(const juce::MouseEvent& event) override;
                        void mouseUp(const juce::MouseEvent& event) override;
                        void mouseMove(const juce::MouseEvent& event) override;
                        void mouseExit(const juce::MouseEvent& event) override;
                        void setBMLDisplay(BarelyMLDisplay* bd) { bmlDisplay = bd; }

                    private:
                        juce::Point<float> mouseDownPosition;
                        BarelyMLDisplay* bmlDisplay;
                };
                InnerViewport viewport;
                Table table;
        };

        class ImageBlock : public Block
        {
            public:
                static bool isImageLine(const juce::String& line);
                void parseImageMarkup(const juce::String& line, FileSource* fileSource);
                float getHeightRequired(float width) override;
                void paint(juce::Graphics&) override;
                void resized() override;

            private:
                juce::AttributedString imageMissingMessage;
                std::unique_ptr<juce::Drawable> drawable;
                int maxWidth;
        };
  
        class ListItem : public Block
        {
            public:
                static bool isListItem(const juce::String& line);
                void parseItemMarkup(const juce::String& line, juce::Font font, int indentPerSpace, int gap);
                float getHeightRequired(float width) override;
                void paint(juce::Graphics&) override;
                bool containsLinkAt(juce::Point<int> localPoint);
                juce::String toString() const override { return attributedString.getText(); }
                // Mouse handlers for clicking on links
                void mouseMove(const juce::MouseEvent& event) override;
                void mouseExit(const juce::MouseEvent& event) override;
        private:
                juce::AttributedString attributedString;
                juce::AttributedString label;
                bool isTask, isTaskChecked;
                int indent;
                int gap;
        };

        public:
            // MARK: - Navigation Accesoires for Internal Links
            juce::OwnedArray<Block>& getBlocks() noexcept { return blocks; }
            juce::Viewport* getViewport() noexcept { return &viewport; }

        // MARK: - Private Variables
        juce::String markupString;            // current markup string
        juce::StringPairArray colours;        // colour palette
        juce::Colour bg;                      // background colour
        juce::Colour tableBG, tableBGHeader;  // table background colours
        int tableMargin, tableGap;            // table margins
        int indentPerSpace, labelGap;         // list item indents
        juce::Viewport  viewport;             // a viewport to scroll the content
        juce::Component content;              // a component with the content
        juce::OwnedArray<Block> blocks;       // representation of the document as blocks
        int margin;                           // content margin in pixels
        int iconsize;                         // admonition icon size in pixels
        int admargin;                         // admonition margin in pixels
        int adlinewidth;                      // admonition line width in pixels
        FileSource* fileSource;               // data source for image files, etc.
        URLHandler* urlHandler;               // URL handler for custom URLs
        juce::Font font;                      // default font for regular text
  
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarelyMLDisplay)
};
