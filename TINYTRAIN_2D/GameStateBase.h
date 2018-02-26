#pragma once
#include <SFML\Graphics.hpp>
#include <functional>

namespace tgf
{
	class Game;

	class GameStateBase
	{
	public:
		virtual void update(float dt) = 0;
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void handleInput(sf::Event& e);

		virtual void onWindowSizeChanged(int w, int h) = 0;

		template<class T> void bindEventCallback(sf::Event::EventType et, T* const object, void(T::* const mf)(sf::Event&))
		{
			// save a pair of the acutal function call and the object pointer for callback deletion
			eventCallbacks_[et].push_back
				( std::make_pair<std::function<void(sf::Event&)>, void*>(std::bind(mf, object, std::placeholders::_1), object) );
		}

		// unbinds all registered callbacks for a specific object (pointer). 
		//
		// Note: only one callback per event is removed for the object as it is assumed
		// for any object to only register one object with each event
		template<class T> void unbindAllCallbacks(T* const object)
		{
			// check the callback function vector for each event
			for (auto vecFuncPointerPair = eventCallbacks_.begin(); vecFuncPointerPair != eventCallbacks_.end(); ++vecFuncPointerPair)
			{
				for (auto f = vecFuncPointerPair->second.begin(); f != vecFuncPointerPair->second.end(); ++f)
				{
					if (f->second == object)
					{
						vecFuncPointerPair->second.erase(f);
						break;
					}
				}
			}
		}

		Game* game_ = 0;

	private:
		// store function pointers to call (along with a raw pointer for compare on unbind), mapped to specific events
		std::map<sf::Event::EventType, std::vector<std::pair<std::function<void(sf::Event&)>, void*>>> eventCallbacks_;
	};
}