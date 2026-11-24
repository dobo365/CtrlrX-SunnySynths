#pragma once

#include "CtrlrIDs.h"
#include "CtrlrMacros.h"
#include "CtrlrUtilities.h"

class CtrlrValueTreeEditorLookProvider : public AsyncUpdater
{
	public:
		virtual const String			getUniqueName(const ValueTree &item)	const { return (item.getType().toString()); }
		virtual const AttributedString	getDisplayString(const ValueTree &item)	const { return (AttributedString(getUniqueName(item))); }
		virtual const Font				getItemFont(const ValueTree &item)		const { return (Font(16)); }
		virtual const int				getItemHeight(const ValueTree &item)	const { return (24); }
		virtual bool					canBeSelected(const ValueTree &item)	const { return (true); }
		virtual Drawable*				getIconForItem(const ValueTree &item)	const { return (nullptr); }
		virtual void					itemClicked(const MouseEvent &e, ValueTree &item)	{ }
		virtual void					itemDoubleClicked(const MouseEvent &e, ValueTree &item)	{ }
		virtual const bool				renameItem(const ValueTree &item, const String &newName) const	{ return (false); }
		virtual const bool				canBeRenamed(const ValueTree &item) const						{ return (false); }
		virtual void					handleAsyncUpdate()=0;
		virtual const bool				isInterestedInDragSource(const ValueTree &item, const DragAndDropTarget::SourceDetails &dragSourceDetails) const { return (false); }
		virtual void					itemDropped (ValueTree &targetItem, const DragAndDropTarget::SourceDetails &dragSourceDetails, int insertIndex) {}
		virtual void					itemOpennessChanged(const bool isNowOpen)						{ }
		virtual var						getDragSourceDescription(Array <ValueTree> &selectedTreeItems)	{ return (""); }
};

class CtrlrValueTreeEditorTree : public TreeView, public LookAndFeel_V4 // Updated v5.6.31 then v5.6.36.7 for the mouse hover
{
    public:
        CtrlrValueTreeEditorTree(const String &name, CtrlrPanel &_owner) : TreeView(name), owner(_owner)
        {
            // Receive mouse events from the nested viewport/content components too (needed for hover)
            addMouseListener (this, true);
        }

        bool keyPressed (const KeyPress &key);
        void mouseMove (const MouseEvent &e) override;
        void mouseExit (const MouseEvent &e) override;

        // Only compare this pointer, never dereference it: the item may have been deleted
        const TreeViewItem *getHoveredItem() const { return hoveredItem; }

    private:
        void setHoveredItem (const TreeViewItem *newItem);

        CtrlrPanel &owner;
        const TreeViewItem *hoveredItem = nullptr;
};

class CtrlrValueTreeEditorItem : public TreeViewItem, public ValueTree::Listener, public LookAndFeel_V4 // Updated v5.6.31
{
	public:
		CtrlrValueTreeEditorItem(CtrlrValueTreeEditorLookProvider &_provider, ValueTree _treeToEdit, const Identifier &_nameIdentifier=Ids::name);
		~CtrlrValueTreeEditorItem();
		bool mightContainSubItems();
		String getUniqueName () const;
		void itemSelectionChanged (bool isNowSelected);
		int	getItemWidth () const;
		int	getItemHeight () const;
		bool canBeSelected () const;
		void itemOpennessChanged (bool isNowOpen);
		bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails &dragSourceDetails);
		void itemDropped (const DragAndDropTarget::SourceDetails &dragSourceDetails, int insertIndex);
		var getDragSourceDescription ();
		void paintItem (Graphics &g, int width, int height);
		void itemClicked (const MouseEvent &e);
		void itemDoubleClicked (const MouseEvent &e);
		void keyPressed (const KeyPress &key);
		// const bool canHandleDragAndDrop();  // Removed 5.6.34. Thanks to @dobo365
		ValueTree &getTree() { return (treeToEdit); }
		void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property);
		void valueTreeChildrenChanged (ValueTree &treeWhoseChildHasChanged){}
		void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged){}
		void valueTreeChildAdded (ValueTree& parentTree, ValueTree& child){}
        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int){}
        void valueTreeChildOrderChanged (ValueTree& parentTreeWhoseChildrenHaveMoved, int, int){}
		JUCE_LEAK_DETECTOR(CtrlrValueTreeEditorItem)

	private:
		ValueTree treeToEdit;
		CtrlrValueTreeEditorLookProvider &provider;
		Identifier nameIdentifier;
};
