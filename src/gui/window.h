#include "imgui.h"
#include "imgui/imgui/backends/imgui_impl_sdl2.h"
#include "imgui/imgui/backends/imgui_impl_opengl3.h"
#include <iostream>
#include "gui/ui-flags/UIFlags.h"
#include "common/maths/vec.h"
#include "common/types.h"

#include <SDL.h>

namespace gbemuGUI
{
	class Window
	{
	private:
		SDL_Window* m_Window;
		SDL_GLContext m_GLContext;
		SDL_Renderer* m_Renderer;
		SDL_Texture* m_Texture;

		int m_Width, m_Height;
		const char* m_Title;


		static Window* s_Instance;

		bool m_Done = false;

	public:
		~Window() = default;

		Window(int width, int height, const char* title);

//		void Run();

		void Clear() const;

		void Update(color* renderArray);

		void Exit();

		int GetWidth() const { return m_Width; }

		int GetHeight() const { return m_Height; }

		float GetAspectRatio() const { return (float)m_Width / (float)m_Height; }

		bool IsDone() const { return m_Done; }
	};
};
