/*
  ==============================================================================

    CtrlrPanelFindProperty.cpp
    Created: 4 Jan 2021 4:49:27pm
    Author:  atom

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CtrlrPanelFindProperty.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelComponentProperties.h"

//==============================================================================
CtrlrPanelFindProperty::CtrlrPanelFindProperty(CtrlrPanelEditor &_owner, CtrlrPanelComponentProperties *_props)
: owner(_owner), props(_props)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
	filterInput.reset(new TextEditor("Filter Input"));
	filterInput->setFont(filterInput->getFont().withHeight(15.0));
	filterInput->addListener(this);
	filterButton.reset(gui::createDrawableButton("Filter", BIN2STR(search_svg)));
	addAndMakeVisible(filterInput.get());
	addAndMakeVisible(filterButton.get());
}

CtrlrPanelFindProperty::~CtrlrPanelFindProperty()
{
}

void CtrlrPanelFindProperty::paint (juce::Graphics& g)
{
	g.fillAll(Colours::white);
}

void CtrlrPanelFindProperty::resized()
{
	int buttonWidth = 24;
	int buttonHeight = 24;

	filterButton->setBounds(0, 0, buttonWidth, buttonHeight);
	filterInput->setBounds(buttonWidth, 0, getWidth() - buttonWidth, buttonHeight);
}

void CtrlrPanelFindProperty::textEditorReturnKeyPressed (TextEditor &e)
{
	if (props)
	{
		const String modifiedFilter = props->setFilter(e.getText());
		e.setText(modifiedFilter);
	}
}

void CtrlrPanelFindProperty::textEditorEscapeKeyPressed (TextEditor &e)
{
	if (props)
	{
		props->setFilter("");
		e.setText("");
	}
}
