#pragma once
#pragma once
#include "Entity.h"
#include <functional>

namespace tgf
{
	class GameStateBase;
	namespace gui
	{
		class TextMenu : public Entity
		{
		public:
			struct Entry
			{
				sf::Text text_;									// text for the entry to display
				std::function<void(void)> func_;				// function to call when menuEntry is executed

																// cached values
				sf::FloatRect localbounds_;
				sf::FloatRect globalbounds_;

				Entry(sf::Text a_text, std::function<void(void)> a_func)
				{
					text_ = a_text;
					func_ = a_func;
					localbounds_ = text_.getLocalBounds();
					globalbounds_ = text_.getGlobalBounds();
				}
			};

			TextMenu();
			TextMenu(GameStateBase * gs);
			~TextMenu();

			// Inherited via Entity
			virtual void update(float dt) override;
			virtual void draw(sf::RenderTarget * target) override;

			void appendItem(sf::Text a_text, std::function<void(void)> a_func);

			virtual void moveSelection(int move_by);
			void onMouseMove(sf::Event & e);
			void onKeyPressed(sf::Event & e);
			void onMousePressed(sf::Event & e);
			void executeSelectedEntry();

			void setArea(sf::FloatRect a_area);
			void recalcMenuPositions();

			void setBackground(sf::RectangleShape* bg);
			sf::RectangleShape* getBackground();

			// menu entry vector.
			// Note: be sure that selection_ is in a valid range after removing any entries!
			std::vector<Entry> menuentries_;

			int selection_;
			bool allowmouse_;
			int maxEntryHeight_;
		protected:
			int getEntryIndexAtPosition(int x, int y);
						
			sf::RectangleShape* background_;
			sf::FloatRect area_;

			GameStateBase* gs_;
		};
	}
}