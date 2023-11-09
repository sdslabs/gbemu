#include "Window.h"
#include <iostream>

#include <SDL_opengl.h>
#include "dock/DockUI.h"
#include "menubar/MenuBar.h"



namespace gbemuGUI
{
	Window::Window(int width, int height, const char* title)
	{
		m_Width = width;
		m_Height = height;
		m_Title = title;

		// Setup SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
		{
			printf("Error: %s\n", SDL_GetError());
			return;
		}

		// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100
		const char* glsl_version = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
		// GL 3.2 Core + GLSL 150
		const char* glsl_version = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

		// Create gui with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		m_Window = SDL_CreateWindow(m_Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
		m_GLContext = SDL_GL_CreateContext(m_Window);
		SDL_GL_MakeCurrent(m_Window, m_GLContext);
//		SDL_GL_SetSwapInterval(1); // Enable vsync


		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::SetCurrentContext(ImGui::GetCurrentContext());
		ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GLContext);
		ImGui_ImplOpenGL3_Init(glsl_version);
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		gbemuGUI::ImGuiThemeSetup();
		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	}

	void Window::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::Update(color* renderArray)
	{
//		SDL_Event event;
//		while (SDL_PollEvent(&event))
//		{
//			ImGui_ImplSDL2_ProcessEvent(&event);
//			if (event.type == SDL_QUIT)
//				m_Done = true;
//			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_Window))
//				m_Done = true;
//		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		gbemuGUI::InitUI();

		// Update the texture
//		for (int i = 0; i < 160 * 144; i++)
//		{
//			renderArray[i] = 0x00FF00FF;
//		}
		SDL_UpdateTexture(m_Texture, NULL, renderArray, 160 * 4);

		bool show_demo_window = true;
		// 2. Show a simple gui that we create ourselves. We use a Begin/End pair to create a named gui.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

			if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

//			ImGui::Image((ImTextureID)(intptr_t)m_Texture, ImVec2(512, 512));

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			printf("Application average %.3f ms/frame (%.1f FPS)\n", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		gbemuGUI::MainMenuBar();
		gbemuGUI::EndUI();

		ImGui::Render();
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call SDL_GL_MakeCurrent(gui, gl_context) directly)
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_GL_SwapWindow(m_Window);
	}

	void Window::Exit()
	{
		if (m_Done)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
			SDL_GL_DeleteContext(m_GLContext);
			SDL_DestroyWindow(m_Window);
			SDL_Quit();
		}

	}

//	void Window::Run()
//	{
//		while (!m_Done)
//		{
//			Update();
//		}
//		Exit();
//	}
}