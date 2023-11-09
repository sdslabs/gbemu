#include "MenuBar.h"

#include "imgui.h"


namespace gbemuGUI
{
	void MainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			// Iterate through the tabs
			for (int i = 0; i < static_cast<int>(MenuBarTabs::NUMBER_OF_TABS); i++)
			{
				// Draw the tabs - very bad way fix this later. TODO.
				if (ImGui::BeginMenu(menubarTabNames[i]))
				{
					switch (static_cast<MenuBarTabs>(i))
					{
					case MenuBarTabs::FILE:
//						FileTab();
						break;
					case MenuBarTabs::EDIT:
//						EditTab();
						break;
					case MenuBarTabs::VIEW:
//						ViewTab();
						break;
					case MenuBarTabs::HELP:
//						HelpTab();
						break;
					default:
						break;
					}

					// End the tab
					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();
		}
	}
}

