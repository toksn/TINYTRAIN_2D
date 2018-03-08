#include "TextMenu.h"
#include "GameStateBase.h"
namespace tgf
{
	namespace gui
	{
		// textmenu default constructor
		// Note: The onKeyPressed, onMouseMove/Pressed input functions are not autmatically mapped to any event callbacks in this ctor.
		// They can be called manually or bound to events of a gamestate.
		// moveSelection() and executeSelectedEntry() can be used manually as well
		TextMenu::TextMenu()
		{
			selection_ = 0;
			allowmouse_ = true;
			gs_ = nullptr;

			maxEntryHeight_ = -1;
		}

		// textmenu constructor within a given area and gamestate to bind to the input events.
		TextMenu::TextMenu(GameStateBase* gs)
		{
			selection_ = 0;
			allowmouse_ = true;
			gs_ = gs;

			maxEntryHeight_ = -1;
			
			// do event bindings
			gs_->bindEventCallback(sf::Event::EventType::MouseMoved, this, &tgf::gui::TextMenu::onMouseMove);
			gs_->bindEventCallback(sf::Event::EventType::MouseButtonPressed, this, &tgf::gui::TextMenu::onMousePressed);
			gs_->bindEventCallback(sf::Event::EventType::KeyPressed, this, &tgf::gui::TextMenu::onKeyPressed);
		}

		TextMenu::~TextMenu()
		{
			if (gs_)
				gs_->unbindAllCallbacks(this);
		}

		void TextMenu::onUpdate(float dt)
		{
			//todo color pulsation LERP of selected entry

			/*for (int i = 0; i < menuentries_.size(); i++)
			{
				if (i == selection_)
					menuentries_[i].text_.setFillColor(sf::Color::Red);
				else
					menuentries_[i].text_.setFillColor(sf::Color::White);
			}*/
		}

		void TextMenu::onDraw(sf::RenderTarget * target)
		{
			if (background_ != nullptr)
				target->draw(*background_);

			for (auto e : menuentries_)
				target->draw(e.text_);
		}

		void TextMenu::appendItem(sf::Text a_text, std::function<void(void)> a_func)
		{
			sf::Uint8 transparency = a_func ? 255 : 128;

			if (menuentries_.size() == selection_)
				a_text.setFillColor(sf::Color(255, 0, 0, transparency));
			else
				a_text.setFillColor(sf::Color(255, 255, 255, transparency));

			menuentries_.push_back(Entry(a_text, a_func));
		}

		void TextMenu::moveSelection(int move_by)
		{
			if (move_by != 0)
			{
				// old selection white
				menuentries_[selection_].text_.setFillColor(sf::Color(255, 255, 255, menuentries_[selection_].func_? 255 : 128 ));
				// new selection red
				selection_ = (selection_ + move_by + menuentries_.size()) % menuentries_.size();
				menuentries_[selection_].text_.setFillColor(sf::Color(255, 0, 0, menuentries_[selection_].func_ ? 255 : 128));
			}
		}

		void TextMenu::onMouseMove(sf::Event& e)
		{
			if (allowmouse_)
			{
				// mouse move to select state
				int i = getEntryIndexAtPosition(e.mouseMove.x, e.mouseMove.y);
				if (i != -1)
					moveSelection(i - selection_);
			}
		}

		void TextMenu::onKeyPressed(sf::Event& e)
		{
			if (menuentries_.size() > 1)
			{
				// arrow keys to select state
				if (e.key.code == sf::Keyboard::Key::Up)
					moveSelection(-1);
				else if (e.key.code == sf::Keyboard::Key::Down)
					moveSelection(1);
				// enter key to execute current selection
				else if (e.key.code == sf::Keyboard::Key::Return)
					executeSelectedEntry();
			}
		}
		
		// left mouse click = use selected state
		void TextMenu::onMousePressed(sf::Event& e)
		{
			if (allowmouse_ && e.mouseButton.button == sf::Mouse::Left)
			{
				int i = getEntryIndexAtPosition(e.mouseButton.x, e.mouseButton.y);
				if (i != -1)
					executeSelectedEntry();
			}
		}

		void TextMenu::executeSelectedEntry()
		{
			if (menuentries_.size() > 0 && menuentries_[selection_].func_)
				menuentries_[selection_].func_();
		}

		int TextMenu::getEntryIndexAtPosition(int x, int y)
		{
			if (area_.contains(x, y))
			{
				for (int i = 0; i < menuentries_.size(); i++)
					if (menuentries_[i].globalbounds_.contains(x, y))
						return i;
			}
			
			return -1;
		}
		
		void TextMenu::setArea(sf::FloatRect a_area)
		{
			area_ = a_area;
			recalcMenuPositions();
		}

		void TextMenu::setBackground(sf::RectangleShape* bg)
		{
			background_ = bg;
		}

		sf::RectangleShape* TextMenu::getBackground()
		{
			return background_;
		}

		void TextMenu::recalcMenuPositions()
		{
			if (menuentries_.size())
			{
				//calculate vertical space for each entry
				float entryheight = area_.height / menuentries_.size();
				float adjust_height = 0.0f;
				if (maxEntryHeight_ > -1 && entryheight > maxEntryHeight_)
				{
					adjust_height = (entryheight - maxEntryHeight_)*menuentries_.size() / 2.0f;
					entryheight = entryheight < maxEntryHeight_ ? entryheight : maxEntryHeight_;
				}				

				// set new charactersize
				float charactersize = entryheight * 0.6f;
				for (auto& e : menuentries_)
				{
					e.text_.setCharacterSize(charactersize);
					e.localbounds_ = e.text_.getLocalBounds(); 
					if (e.localbounds_.width > area_.width)
						charactersize = charactersize * (0.9f*area_.width / e.localbounds_.width);
				}
				// character size changed because at least one entry exeeded the line, re-apply char size to all entries
				if (charactersize != entryheight * 0.6f)
				{
					for (auto& e : menuentries_)
					{
						e.text_.setCharacterSize(charactersize);
						e.localbounds_ = e.text_.getLocalBounds();
					}

					adjust_height = (entryheight - charactersize/0.6f)*menuentries_.size() / 2.0f;
					entryheight = charactersize / 0.6f;
				}

				for (int i = 0; i < menuentries_.size(); i++)
				{
					//calc position
					auto& text = menuentries_[i].text_;
					auto textSize = menuentries_[i].localbounds_;

					menuentries_[i].text_.setPosition(area_.left + (area_.width - textSize.width) / 2, area_.top + adjust_height + entryheight * i + (entryheight - textSize.height) / 2);
					menuentries_[i].globalbounds_ = menuentries_[i].text_.getGlobalBounds();
				}
			}
		
			if (background_ != nullptr)
			{
				background_->setPosition(area_.left, area_.top);
				background_->setSize(sf::Vector2f(area_.width, area_.height));
			}
		}
	}
}