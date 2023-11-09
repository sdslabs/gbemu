#pragma once

#include "gui/ui-flags/UIFlags.h"
#include <string>

// Using X macros to define the tab names
#define MENU_BAR_TABS \
X(FILE, "File")      \
X(EDIT, "Edit")      \
X(VIEW, "View")      \
X(HELP, "Help")      \
X(NUMBER_OF_TABS, "") \

namespace gbemuGUI
{
#define X(en, str) en,
	enum class MenuBarTabs : size_t {
		MENU_BAR_TABS
	};
#undef X

#define X(en, str) str,
	static const char* menubarTabNames[] = {
		MENU_BAR_TABS
	};
#undef X
	void MainMenuBar();

	void FileTab();
	void EditTab();
	void ViewTab();
	void HelpTab();
};
