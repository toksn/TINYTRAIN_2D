#pragma once
#include "GameStateBase.h"
#include <functional>

namespace tinytrain
{
	struct menuEntry
	{
		sf::Text text_;									// text for the entry to display
		std::function<void(void)> func_;				// function to call when menuEntry is executed

		// cached values
		sf::FloatRect localbounds_;
		sf::FloatRect globalbounds_;

		menuEntry(sf::Text a_text, std::function<void(void)> a_func)
		{
			text_ = a_text;
			func_ = a_func;
			localbounds_ = text_.getLocalBounds();
			globalbounds_ = text_.getGlobalBounds();
		}
	};

	class GameState_MainMenu : public tgf::GameStateBase
	{
	public:
		GameState_MainMenu(tgf::Game* game);
		~GameState_MainMenu();

		// Inherited via GameStateBase
		virtual void update(float dt) override;
		virtual void draw(sf::RenderTarget * target) override;
		virtual void onWindowSizeChanged(int w, int h) override;
		virtual void handleInput(sf::Event &e) override;

		

	protected:
		void executeSelectedEntry();

		int getEntryIndexAtPosition(int x, int y);
		void onStart();
		void onQuit();

		std::vector<menuEntry> menuentries_;
		sf::Font font_;
		int selection_;
	};
}