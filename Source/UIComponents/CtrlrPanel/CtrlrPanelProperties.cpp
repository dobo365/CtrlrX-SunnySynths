#include "stdafx.h"
#include "CtrlrPanelEditor.h"
#include "CtrlrPanelUtilities.h"
#include "CtrlrLog.h"
#include "CtrlrPanelComponentProperties.h"
#include "CtrlrPanelResourceEditor.h"


#include "CtrlrPanelProperties.h"

CtrlrPanelProperties::CtrlrPanelProperties (CtrlrPanelEditor &_owner)
    : Component (L"Properties"),
      owner(_owner),
      tabbedComponent (0)
{
    // Added v5.6.36.4 @dobo365
    addAndMakeVisible(propLabel = new Label("propLabel", "PROPERTIES"));
    propLabel->setFont(Font(14, Font::bold));
    propLabel->setJustificationType(Justification::centred);
    propLabel->setColour(Label::backgroundColourId, Colour(0xffcccccc));
    propLabel->setColour(Label::textColourId, Colours::black);

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtRight));
    
    tabbedComponent->setTabBarDepth (owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth));
    tabbedComponent->setCurrentTabIndex (-1);
    tabbedComponent->setOutline (1);
    
	CtrlrPanelComponentProperties *props = new CtrlrPanelComponentProperties (owner);
        
	tabbedComponent->addTab ("General",
                             getLookAndFeel().findColour(TabbedComponent::backgroundColourId), // Updated v5.6.31
                             props,
                             true
                             );
    
    tabbedComponent->addTab ("Resources",
                             getLookAndFeel().findColour(TabbedComponent::backgroundColourId), // Updated v5.6.31
                             new CtrlrPanelResourceEditor(owner),
                             true
                             );
    
	tabbedComponent->addTab ("Utility",
                             getLookAndFeel().findColour(TabbedComponent::backgroundColourId), // Updated v5.6.31
                             new CtrlrPanelUtilities(owner),
                             true
                             );

    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().addChangeListener(this);
    
	ctrlrPanelFindProperty.reset(new CtrlrPanelFindProperty(owner, props));
	addAndMakeVisible(ctrlrPanelFindProperty.get());
	setSize (216, 364);
}

CtrlrPanelProperties::~CtrlrPanelProperties()
{
    tabbedComponent->getTabbedButtonBar().removeChangeListener(this);

    CtrlrPanelComponentProperties *p = dynamic_cast <CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent (0));
	if (p)
	{
		owner.getOwner().getCtrlrManagerOwner().removeListener (p);
	}
    deleteAndZero (tabbedComponent);
}

void CtrlrPanelProperties::paint (Graphics& g)
{
    g.fillAll(findColour(DocumentWindow::backgroundColourId));
}

void CtrlrPanelProperties::resized()    	// Adapted v5.6.36.4 @dobo365
{
    const int labelHeight = 24;  // Height for the top label
    const int findHeight = 24;   // Height for the Search box
    const int currentTab = tabbedComponent->getCurrentTabIndex();

    propLabel->setBounds(0, 0, getWidth(), labelHeight);

    if (currentTab == 0)
    {
        propLabel->setText("PROPERTIES", dontSendNotification);
    }
    else if (currentTab == 1)
    {
        propLabel->setText("RESOURCES", dontSendNotification);
    }
    else if (currentTab == 2)
    {
        propLabel->setText("XML DATA", dontSendNotification);
    }

    if (currentTab == 0)
    {
        ctrlrPanelFindProperty->setVisible(true);
        ctrlrPanelFindProperty->setBounds(0, labelHeight, getWidth() - (int)owner.getOwner().getOwner().getProperty(Ids::ctrlrTabBarDepth), findHeight);
        tabbedComponent->setBounds(0, labelHeight + findHeight, getWidth() - 0, getHeight() - labelHeight - findHeight);
    }
    else
    {
        ctrlrPanelFindProperty->setVisible(false);
        tabbedComponent->setBounds(0, labelHeight, getWidth() - 0, getHeight() - labelHeight);
    }
}

void CtrlrPanelProperties::lookAndFeelChanged() // Added v5.6.31
{
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::frontTextColourId, findColour(Label::textColourId)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabTextColourId, findColour(Label::textColourId).withAlpha(0.6f)); // Added v5.6.31
    tabbedComponent->getTabbedButtonBar().setColour(TabbedButtonBar::tabOutlineColourId, findColour(Slider::textBoxOutlineColourId)); // Added v5.6.31
    repaint(); // Added v5.6.31
}

void CtrlrPanelProperties::changeListenerCallback(ChangeBroadcaster* source)
{
    for (int i = 0; i < tabbedComponent->getNumTabs(); i++)
    {
        if (i == tabbedComponent->getTabbedButtonBar().getCurrentTabIndex())
        {
            // This is for v3 where the color is forced. Should add an If..then..else checking v4 LnF
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, Colour(0xffcccccc));   // v5.6.34: set to Light grey. !! also changes bottom part. By @dobo365 DB
        }
        else
        {
            tabbedComponent->getTabbedButtonBar().setTabBackgroundColour(i, getLookAndFeel().findColour(TabbedComponent::backgroundColourId));
        }
    }

    if (source == &tabbedComponent->getTabbedButtonBar())
    {
        resized(); // Re-trigger the layout update cleanly
    }
}

void CtrlrPanelProperties::refreshAll()
{
	for (int i=0; i<tabbedComponent->getNumTabs(); i++)
	{
		CtrlrPanelComponentProperties *cp = dynamic_cast<CtrlrPanelComponentProperties*>(tabbedComponent->getTabContentComponent(i));
		if (cp!=0)
		{
			cp->refreshAll();
		}
	}
}

void CtrlrPanelProperties::layoutChanged()
{
	if ((bool)owner.getProperty(Ids::uiPanelPropertiesOnRight) == true)
	{
		tabbedComponent->setOrientation(TabbedButtonBar::TabsAtLeft);
	}
	else
	{
		tabbedComponent->setOrientation(TabbedButtonBar::TabsAtRight);
	}
}
