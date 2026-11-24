#include "stdafx.h"
#include "CtrlrValueTreeEditor.h"
#include "CtrlrInlineUtilitiesGUI.h"

//=========================================================================================================
bool CtrlrValueTreeEditorTree::keyPressed (const KeyPress &key)
{
	if (getNumSelectedItems() == 1)
	{
		if (getSelectedItem(0))
		{
			CtrlrValueTreeEditorItem *item = dynamic_cast<CtrlrValueTreeEditorItem*>(getSelectedItem(0));

			if (item != nullptr)
			{
				item->keyPressed(key);
			}
		}
	}

	return (TreeView::keyPressed(key));
}

void CtrlrValueTreeEditorTree::mouseMove (const MouseEvent &e)
{
    TreeView::mouseMove (e);

    // Convert to this tree's coordinates, since the event may come from a nested child
    const Point<int> pos = e.getEventRelativeTo (this).getPosition();
    setHoveredItem (getItemAt (pos.y));
}

void CtrlrValueTreeEditorTree::mouseExit (const MouseEvent &e)
{
    TreeView::mouseExit (e);

    // Ignore exits towards a nested child (viewport, content), only clear when really leaving the tree
    if (!contains (e.getEventRelativeTo (this).getPosition()))
    {
        setHoveredItem (nullptr);
    }
}

void CtrlrValueTreeEditorTree::setHoveredItem (const TreeViewItem *newItem)
{
    if (hoveredItem != newItem)
    {
        hoveredItem = newItem;
        repaint();
    }
}

//=========================================================================================================

CtrlrValueTreeEditorItem::CtrlrValueTreeEditorItem(CtrlrValueTreeEditorLookProvider &_provider, ValueTree _treeToEdit, const Identifier &_nameIdentifier)
	: treeToEdit(_treeToEdit), provider(_provider), nameIdentifier(_nameIdentifier)
{
	treeToEdit.addListener (this);
    setDrawsInLeftMargin (true); // Allow the item to paint over the indent area, up to the tree's left edge
}

CtrlrValueTreeEditorItem::~CtrlrValueTreeEditorItem()
{
	treeToEdit.removeListener (this);
}

bool CtrlrValueTreeEditorItem::mightContainSubItems()
{
	return (treeToEdit.getNumChildren() ? true : false);
}

String CtrlrValueTreeEditorItem::getUniqueName () const
{
	return (provider.getUniqueName(treeToEdit));
}

void CtrlrValueTreeEditorItem::itemSelectionChanged (bool isNowSelected)
{
}

int	CtrlrValueTreeEditorItem::getItemWidth () const
{
	//return (jmax<int>(152, 48 + provider.getItemFont(treeToEdit).getStringWidth (provider.getUniqueName(treeToEdit)))); // Updated v5.6.31
    return -1;
}

int	CtrlrValueTreeEditorItem::getItemHeight () const
{
	return (provider.getItemHeight(treeToEdit));
}

bool CtrlrValueTreeEditorItem::canBeSelected () const
{
	return (provider.canBeSelected(treeToEdit));
}

void CtrlrValueTreeEditorItem::itemOpennessChanged (bool isNowOpen)
{
	if (isNowOpen)
	{
		if (getNumSubItems() == 0)
		{
			for (int i=0; i<treeToEdit.getNumChildren(); i++)
			{
				addSubItem (new CtrlrValueTreeEditorItem (provider, treeToEdit.getChild(i), nameIdentifier));
			}
		}
	}
	else
	{
		clearSubItems();
	}

	provider.itemOpennessChanged(isNowOpen);
}

var CtrlrValueTreeEditorItem::getDragSourceDescription ()
{
	Array <ValueTree> selectedTreeItems;
	String returnValue;

	if (getOwnerView())
	{
		for (int i=0; i<getOwnerView()->getNumSelectedItems(); i++)
		{
			CtrlrValueTreeEditorItem *item = dynamic_cast<CtrlrValueTreeEditorItem*>(getOwnerView()->getSelectedItem(i));
			selectedTreeItems.add (item->getTree());
		}
	}

	if ((returnValue = provider.getDragSourceDescription(selectedTreeItems)) != "")
	{
		return (returnValue);
	}
	else
	{
		for (int i=0; i<selectedTreeItems.size(); i++)
		{
			returnValue << selectedTreeItems[i].getType().toString() + ";";
		}
	}

	return (returnValue);
}

bool CtrlrValueTreeEditorItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails &dragSourceDetails)
{
	return (provider.isInterestedInDragSource (treeToEdit, dragSourceDetails));
}

// GUI For TreeView Items. Updated v5.6.31 then v5.6.36.7 completely adapted for mouse hover
void CtrlrValueTreeEditorItem::paintItem (Graphics &g, int width, int height)
{
    std::unique_ptr<Drawable> icon(provider.getIconForItem (treeToEdit));
	auto &currentLnF = getDefaultLookAndFeel(); // Added v5.6.31

    // Default (V3 and lower): soft lavender with black text
    Colour selectedFill (0xFFADADCC);
    Colour selectedText (Colours::black);

    // V4: take the colours from the active colour scheme
    if (auto *view = getOwnerView())
    {
        if (auto *lfV4 = dynamic_cast<LookAndFeel_V4*> (&view->getLookAndFeel()))
        {
            auto &scheme = lfV4->getCurrentColourScheme();
            selectedFill = scheme.getUIColour (LookAndFeel_V4::ColourScheme::highlightedFill);
            selectedText = scheme.getUIColour (LookAndFeel_V4::ColourScheme::highlightedText);
        }
    }

    // Hover state is tracked by the tree
    bool isHovered = false;
    if (auto *tree = dynamic_cast<CtrlrValueTreeEditorTree*> (getOwnerView()))
    {
        isHovered = canBeSelected() && (tree->getHoveredItem() == this);
    }

    if (isSelected() || isHovered)
    {
        // Paint from the left edge of the tree (setDrawsInLeftMargin(true) must be set in the constructor)
        const int indentX = getItemPosition (false).getX();

        g.setColour (isSelected() ? selectedFill : selectedFill.withAlpha (0.45f));
        g.fillRect (-indentX, 0, width + indentX, height);
    }
    g.setColour(currentLnF.findColour(TextEditor::textColourId)); // Added v5.6.31. Was g.setColour (Colours::black);
       
	AttributedString as = provider.getDisplayString(treeToEdit);
	as.setJustification (Justification (Justification::centredLeft));
	as.draw (g, Rectangle <float> (24.0, 0.0, width - 24.0, height));

	if (icon)
		icon->drawWithin(g, Rectangle<float>(4,0, 16, height), RectanglePlacement::centred, 1.0f);
}

void CtrlrValueTreeEditorItem::itemDropped (const DragAndDropTarget::SourceDetails &dragSourceDetails, int insertIndex)
{
	provider.itemDropped (treeToEdit, dragSourceDetails, insertIndex);
}

void CtrlrValueTreeEditorItem::itemClicked (const MouseEvent &e)
{
	provider.itemClicked (e, treeToEdit);
}

void CtrlrValueTreeEditorItem::itemDoubleClicked (const MouseEvent &e)
{
	provider.itemDoubleClicked (e, treeToEdit);
}

void CtrlrValueTreeEditorItem::keyPressed (const KeyPress &key)
{
	if (key.getKeyCode() == 65649) // F2
	{
		if (!provider.canBeRenamed(treeToEdit))
			return;

		AlertWindow wnd("Rename item","Rename current item type: " + treeToEdit.getType().toString(), AlertWindow::QuestionIcon, getOwnerView());
		wnd.addTextEditor ("name", treeToEdit.getProperty(nameIdentifier), "Name:", false);
		wnd.addButton ("OK", 1, KeyPress(KeyPress::returnKey));
		wnd.addButton ("Cancel", 0, KeyPress(KeyPress::escapeKey));
		if (wnd.runModalLoop())
		{
			if (provider.renameItem (treeToEdit, wnd.getTextEditorContents("name")))
			{
				treeToEdit.setProperty (Ids::name, wnd.getTextEditorContents("name"), nullptr);
			}

			provider.triggerAsyncUpdate();
		}
	}
}

void CtrlrValueTreeEditorItem::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
	repaintItem();
}
