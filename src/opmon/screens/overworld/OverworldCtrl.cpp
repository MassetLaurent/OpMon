/*
  OverworldCtrl.cpp
  Author : Cyrielle
  Contributors : BAKFR, Navet56
  File under GNU GPL v3.0 license
 */
#include "OverworldCtrl.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <memory>

#include "src/opmon/screens/animation/AnimationCtrl.hpp"
#include "src/opmon/screens/battle/BattleCtrl.hpp"
#include "src/opmon/screens/gamemenu/GameMenuCtrl.hpp"
#include "src/opmon/core/Player.hpp"
#include "src/opmon/core/GameData.hpp"
#include "src/opmon/screens/animation/Animations.hpp"
#include "src/opmon/screens/gamemenu/GameMenuData.hpp"
#include "Overworld.hpp"
#include "src/opmon/screens/base/AGameScreen.hpp"
#include "src/opmon/view/elements/Map.hpp"
#include "src/opmon/view/elements/Position.hpp"
#include "src/opmon/view/ui/Dialog.hpp"
#include "src/opmon/view/ui/Jukebox.hpp"
#include "src/opmon/core/GameStatus.hpp"

//Defines created to make the code easier to read
#define LOAD_BATTLE 1
#define LOAD_MENU_OPEN 2
#define LOAD_MENU 3
#define LOAD_MENU_CLOSE 4

namespace OpMon {

	OverworldCtrl::OverworldCtrl(Player &player, GameData *gamedata)
	: data(gamedata, &player)
	, view("Player's room", this->data)
	, player(player) {}

	GameStatus OverworldCtrl::checkEvent(sf::Event const &events) {
		if(loadNext == LOAD_MENU_OPEN) {
			loadNext = LOAD_MENU;
			return GameStatus::NEXT_NLS;
		} else if(loadNext == LOAD_MENU) {
			loadNext = LOAD_MENU_CLOSE;
			return GameStatus::NEXT_NLS;
		}
		auto &overworld = view;

		bool is_dialog_open = overworld.getDialog() && !overworld.getDialog()->isDialogOver();

		switch(events.type) {
		case sf::Event::KeyPressed:
			//The key equals starts the debug mode
			if(events.key.code == sf::Keyboard::Equal) {
				view.debugMode = !view.debugMode;
				if(!view.debugMode) {
					debugCol = false;
					overworld.setCameraLock(false);
				}
			}
			if(view.debugMode) {
				//R reboots the game
				if(events.key.code == sf::Keyboard::R) {
					return GameStatus::REBOOT;
				}
				//Debug, chooses the layers to print
				if(events.key.code == sf::Keyboard::F10) {
					overworld.printlayer[0] = !overworld.printlayer[0];
				}
				if(events.key.code == sf::Keyboard::F11) {
					overworld.printlayer[1] = !overworld.printlayer[1];
				}
				if(events.key.code == sf::Keyboard::F12) {
					overworld.printlayer[2] = !overworld.printlayer[2];
				}

				if(events.key.code == sf::Keyboard::C) {
					overworld.printCollisions = !overworld.printCollisions;
				}
				if(events.key.code == sf::Keyboard::N) {
					debugCol = !debugCol;
				}
				if(events.key.code == sf::Keyboard::Numpad5) {
					overworld.setCameraLock(!overworld.isCameraLocked());
				}
				if(events.key.code == sf::Keyboard::PageUp) {
					data.decrementItorMap();
					overworld.tp(data.getCurrentItorMap(), sf::Vector2i(5, 5));
				}
				if(events.key.code == sf::Keyboard::PageDown) {
					data.incrementItorMap();
					overworld.tp(data.getCurrentItorMap(), sf::Vector2i(5, 5));
				}
				if(events.key.code == sf::Keyboard::B) {
					overworld.tp("Road 14", sf::Vector2i(10, 32));
				}
			}
			if(events.key.code == sf::Keyboard::M) {
				loadNext = LOAD_MENU_OPEN;
				return GameStatus::NEXT_NLS;
			}
		default:
			break;
		}
		if(overworld.isCameraLocked()) {
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) {
				overworld.moveCamera(Side::TO_DOWN);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad4)) {
				overworld.moveCamera(Side::TO_LEFT);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad8)) {
				overworld.moveCamera(Side::TO_UP);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad6)) {
				overworld.moveCamera(Side::TO_RIGHT);
			}
		}

		return is_dialog_open ? checkEventsDialog(events, overworld) : checkEventsNoDialog(events, player);
	}

	GameStatus OverworldCtrl::checkEventsDialog(sf::Event const &events, Overworld &overworld) {
		switch(events.type) {
		case sf::Event::KeyPressed:
			if(events.key.code == data.getGameDataPtr()->getKeyTalk()) {
				overworld.getDialog()->pass();
			}
			break;
		default:
			break;
		}
		return GameStatus::CONTINUE;
	}

	GameStatus OverworldCtrl::checkEventsNoDialog(sf::Event const &event, Player &player) {
		checkAction(event, player, view);
		checkMove(player, view);

		if(view.getBattleDeclared() != nullptr) {
			if(view.getBattleDeclared()->isOver()) {
				view.endBattle();
			} else {
				loadNext = LOAD_BATTLE;
				return GameStatus::NEXT;
			}
		}

		return GameStatus::CONTINUE;
	}

	GameStatus OverworldCtrl::update(sf::RenderTexture &frame) {
		bool is_dialog_open = view.getDialog() && !view.getDialog()->isDialogOver();
		if(!is_dialog_open) {
			updateEvents(data.getMap(player.getMapId())->getEvents(), player, view);
		}

		GameStatus toReturn = view.update();
		frame.draw(view);
		screenTexture = frame.getTexture();
		return toReturn;
	}

	void OverworldCtrl::loadNextScreen() {
		data.getGameMenuData().setBackground(screenTexture);
		switch(loadNext) {
		case LOAD_BATTLE:
			_next_gs = std::make_unique<BattleCtrl>(data.getPlayer().getOpTeam(), view.getBattleDeclared(), data.getGameDataPtr(), data.getPlayerPtr());
			break;
		case LOAD_MENU_OPEN:
			_next_gs = std::make_unique<AnimationCtrl>(std::make_unique<Animations::WooshAnim>(screenTexture, data.getGameMenuData().getMenuTexture(), Animations::WooshDir::UP, 15, true));
			break;
		case LOAD_MENU:
			_next_gs = std::make_unique<GameMenuCtrl>(data.getGameMenuData(), player);
			break;
		case LOAD_MENU_CLOSE:
			_next_gs = std::make_unique<AnimationCtrl>(std::make_unique<Animations::WooshAnim>(screenTexture, data.getGameMenuData().getMenuTexture(), Animations::WooshDir::DOWN, 15, false));
			break;
		default:
			throw Utils::UnexpectedValueException(std::to_string(loadNext), "a view to load in OverworldCtrl::loadNextScreen()");
		}
	}

	void OverworldCtrl::suspend() {
		if(loadNext == LOAD_BATTLE) {
			data.getGameDataPtr()->getJukebox().pause();
		}
	}

	void OverworldCtrl::resume() {
		data.getGameDataPtr()->getJukebox().play(data.getCurrentMap()->getBg());
	}

	void OverworldCtrl::checkMove(Player &player, Overworld &overworld) {
		if(!overworld.justTp && !player.getPosition().isAnim() && !player.getPosition().isLocked()) {
			//TODO Factorise code
			if(sf::Keyboard::isKeyPressed(overworld.getData().getGameDataPtr()->getKeyUp())) {
				overworld.startPlayerAnimation();
				move(Side::TO_UP, player, overworld);
			} else if(sf::Keyboard::isKeyPressed(overworld.getData().getGameDataPtr()->getKeyDown())) {
				overworld.startPlayerAnimation();
				move(Side::TO_DOWN, player, overworld);
			} else if(sf::Keyboard::isKeyPressed(overworld.getData().getGameDataPtr()->getKeyLeft())) {
				overworld.startPlayerAnimation();
				move(Side::TO_LEFT, player, overworld);
			} else if(sf::Keyboard::isKeyPressed(overworld.getData().getGameDataPtr()->getKeyRight())) {
				overworld.startPlayerAnimation();
				move(Side::TO_RIGHT, player, overworld);
			}
		}
	}

	void OverworldCtrl::move(Side direction, Player &player, Overworld &overworld) {
		player.getPosition().move(direction, overworld.getData().getCurrentMap(), debugCol);

		Elements::Map *map = overworld.getData().getCurrentMap();
		std::list<Elements::AbstractEvent*> eventList = map->getEvent(player.getPosition().getPosition());
		actionEvents(eventList, player, Elements::EventTrigger::GO_IN, overworld);
	}

	void OverworldCtrl::checkAction(sf::Event const &event, Player &player, Overworld &overworld) {
		//If the player isn't moving, then this checks if the player want to activate an event.
		if(!player.getPosition().isAnim()) {
			//Get the event coordinates and activate it if the player interacted with it.
			if(sf::Keyboard::isKeyPressed(overworld.getData().getGameDataPtr()->getKeyInteract())) {
				int lx = player.getPosition().getPosition().x;
				int ly = player.getPosition().getPosition().y;
				switch(player.getPosition().getDir()) {
				case Side::TO_UP:
					ly--;
					break;
				case Side::TO_DOWN:
					ly++;
					break;
				case Side::TO_LEFT:
					lx--;
					break;
				case Side::TO_RIGHT:
					lx++;
					break;
				default:
					break;
				}

				std::list<Elements::AbstractEvent *> eventList = overworld.getData().getCurrentMap()->getEvent(sf::Vector2i(lx, ly));

				//Checks if the event has been already triggered in the last frame.
				//List of all elements present in eventList and already triggered in the last frame.
				std::list<std::list<Elements::AbstractEvent *>::iterator> commonList;
				for(std::list<Elements::AbstractEvent *>::iterator currentEvent = eventList.begin(); currentEvent != eventList.end(); ++currentEvent) {
					bool alreadyUsed = false;
					for(std::list<Elements::AbstractEvent*>::iterator usedEvent = usedList.begin(); usedEvent != usedList.end(); ++usedEvent) {
						if(*usedEvent == *currentEvent) {
							alreadyUsed = true;
							commonList.push_back(currentEvent);
						}
					}
				}
				//Resets the used list with the current events
				usedList = eventList;
				//Deletes from eventList all events already triggered in the last frame.
				for(std::list<Elements::AbstractEvent *>::iterator itor : commonList)
					eventList.erase(itor);

				actionEvents(eventList, player, Elements::EventTrigger::PRESS, overworld);
			}
		}

		usedList.clear();

		//Searches for events at the same position as the player and activates them if they are triggered when the playeris in them.
		if(!player.getPosition().isMoving()) {
			std::list<Elements::AbstractEvent *> eventList = overworld.getData().getCurrentMap()->getEvent(player.getPosition().getPosition());
			actionEvents(eventList, player, Elements::EventTrigger::BE_IN, overworld);
		}
	}

	void OverworldCtrl::actionEvents(std::list<Elements::AbstractEvent *> &events, Player &player, Elements::EventTrigger toTrigger, Overworld &overworld) {
		//Checks if the player points at the right direction to activate the events. If yes, calls the events' action methods.
		Side ppDir = player.getPosition().getDir();
		for(std::list<Elements::AbstractEvent*>::iterator itor = events.begin(); itor != events.end(); ++itor) {
			if((*itor)->getEventTrigger() == toTrigger) {
				bool go = false;
				if((((*itor)->getSide() & SIDE_UP) == SIDE_UP) && ppDir == Side::TO_UP) {
					go = true;
				} else if((((*itor)->getSide() & SIDE_DOWN) == SIDE_DOWN) && ppDir == Side::TO_DOWN) {
					go = true;
				} else if((((*itor)->getSide() & SIDE_RIGHT) == SIDE_RIGHT) && ppDir == Side::TO_RIGHT) {
					go = true;
				} else if((((*itor)->getSide() & SIDE_LEFT) == SIDE_LEFT) && ppDir == Side::TO_LEFT) {
					go = true;
				}
				if(go) {
					(*itor)->action(player, overworld);
				}
			}
		}
	}

	void OverworldCtrl::updateEvents(std::vector<Elements::AbstractEvent *> &events, Player &player, Overworld &overworld) {
		for(Elements::AbstractEvent *event : events) {
			event->update(player, overworld);
		}
	}

} // namespace OpMon
