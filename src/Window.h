#pragma once

#include <string>
#include <map>

#include <sdl/SDL.h>
#include <glm/glm.hpp>

class Window
{
public:
	Window(const char* title, const glm::uvec2& dimensions) : m_title(title), m_dimensions(dimensions)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		m_sdlWindow = SDL_CreateWindow(
			m_title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			static_cast<int>(m_dimensions.x),
			static_cast<int>(m_dimensions.y),
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
		);
	}

	virtual ~Window()
	{
		SDL_DestroyWindow(m_sdlWindow);
		SDL_Quit();
	}

public:
	SDL_GLContext createContext() const
	{
		const auto glContext = SDL_GL_CreateContext(m_sdlWindow);
		SDL_GL_MakeCurrent(m_sdlWindow, glContext);
		return glContext;
	}

	void run()
	{
		handleEvents();
		init();
		while (!m_shouldClose)
		{
			handleEvents();
			update(calcDeltaTime());
			SDL_GL_SwapWindow(m_sdlWindow);
		}
	}

	void close()
	{
		m_shouldClose = true;
	}

protected:
	virtual void init() = 0;
	virtual void update(const float& deltaTime) = 0;

protected:
	virtual void onResize(const glm::uvec2& windowDimensions)
	{
		m_dimensions = windowDimensions;
	}

protected:
	bool isKeyDown(SDL_Keycode keycode)
	{
		return keyboardState[keycode];
	}

	bool isKeyPressed(SDL_Keycode keycode)
	{
		return keyboardState[keycode] && !lastKeyboardState[keycode];
	}

	bool isKeyReleased(SDL_Keycode keycode)
	{
		return !keyboardState[keycode] && lastKeyboardState[keycode];
	}

	bool isMouseButtonDown(uint8_t mouseButton)
	{
		return mouseState[mouseButton];
	}

	bool isMouseButtonPressed(uint8_t mouseButton)
	{
		return mouseState[mouseButton] && !lastMouseState[mouseButton];
	}

	bool isMouseButtonReleased(uint8_t mouseButton)
	{
		return !mouseState[mouseButton] && lastMouseState[mouseButton];
	}

	const glm::vec2& getMouseMovement() const
	{
		return mouseMotion;
	}

private:
	void handleEvents()
	{
		lastKeyboardState = keyboardState;
		lastMouseState = mouseState;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				close();
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					onResize({event.window.data1, event.window.data2});
				break;
			case SDL_KEYDOWN:
				keyboardState[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				keyboardState[event.key.keysym.sym] = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseState[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				mouseState[event.button.button] = false;
				break;
			case SDL_MOUSEMOTION:
				mouseMotion = {event.button.x, event.button.y};
				break;
			default:
				break;
			}
		}
	}

	float calcDeltaTime()
	{
		const uint64_t lastFrameCount = frameCount;
		frameCount = SDL_GetPerformanceCounter();
		return static_cast<float>(frameCount - lastFrameCount)
			/ static_cast<float>(SDL_GetPerformanceFrequency());
	}

private:
	SDL_Window* m_sdlWindow = nullptr;
	std::string m_title = "Window";
	glm::uvec2 m_dimensions = {800u, 600u};

	bool m_shouldClose = false;
	uint64_t frameCount = 0;

	std::map<SDL_Keycode, bool> lastKeyboardState;
	std::map<SDL_Keycode, bool> keyboardState;

	std::map<uint8_t, bool> lastMouseState;
	std::map<uint8_t, bool> mouseState;

	glm::vec2 mouseMotion = {0.0f, 0.0f};
};
