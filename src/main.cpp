#include "gui/gui.h"
#include "core/gameBoy.h"

int main(int argv, char** argc)
{

	gbemuGUI::Window window = gbemuGUI::Window(1280, 720, "gbemu");

	GBE* gbe = new GBE();

	SDL_Texture* texture = nullptr;
	color* renderArray = gbe->getRenderArray();
	SDL_Renderer* renderer = nullptr;
	SDL_UpdateTexture(texture, NULL, renderArray, 160 * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	int cyclesForPPUEExecution = 0;
	while(!window.IsDone())
	{
		cyclesForPPUEExecution += gbe->update();
		renderArray = gbe->getRenderArray();
		if (cyclesForPPUEExecution >= (456 + 204 + 80 + 172))
		{
			window.Update(renderArray);
			cyclesForPPUEExecution = 0;
		}
	}

	window.Exit();


	return 0;
}