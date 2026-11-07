#ifndef __CTRLR_SETTINGS__
#define __CTRLR_SETTINGS__

#include "CtrlrManager/CtrlrManager.h"


class CtrlrSettings : public Component,
					  public juce::ValueTree::Listener     // Added v5.6.36.5 @dobo365 to only react when a Preference has been modified
{
	public:
		CtrlrSettings (CtrlrManager &_owner);
		~CtrlrSettings();
		void paint (Graphics& g);
		void resized();
		void lookAndFeelChanged(); // Added v5.6.36.5 @dobo365
		void restart();
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override; // Added v5.6.36.5 @dobo365
		JUCE_LEAK_DETECTOR(CtrlrSettings)

	private:
		CtrlrManager &owner;
		// PropertyPanel* propertyPanel;
		// Main UI element containing the vertical tabs
		std::unique_ptr<TabbedComponent> tabbedComponent;
		bool settingsChanged = false;    // Added v5.6.36.5 to only react when a Preference has been modified
		// Map tracking existing property panels linked to their respective section names
		std::map<String, std::unique_ptr<PropertyPanel>> sectionPanels;
};


#endif
