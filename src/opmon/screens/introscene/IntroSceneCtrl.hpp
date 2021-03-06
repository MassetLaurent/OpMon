/*!
 * \file IntroSceneCtrl.hpp
 * \authors Cyrielle
 * \authors BAKFR
 * \copyright GNU GPL v3.0
 */
#ifndef STARTSCENECTRL_HPP
#define STARTSCENECTRL_HPP

#include <SFML/Window.hpp>

#include "IntroScene.hpp"
#include "src/opmon/screens/base/AGameScreen.hpp"

namespace sf {
class Event;
class RenderTexture;
}  // namespace sf

namespace OpMon {
class GameData;

    /*!
     * \brief Manages the introduction scene and its three phases.
     * \todo Change to IntroSceneCtrl
     */
    class IntroSceneCtrl : public AGameScreen {
    protected:
        IntroSceneData data;

        IntroScene view;
        /*!
         * \brief If `true`, a closing Animations::WinAnim is launched. This is set to `true` when the player has finished entering their name.
         */
        bool animNext = false;

        /*!
         * \brief Determines which screen has to be loaded in loadNextScreen().
         *
         * This integer is filled with some special values determined by macros in GameMenuCtrl.cpp. Currently, there is
         * - LOAD_OVERWORLD : Loads OverworldCtrl at the end of the introduction scene.
         * - LOAD_ANIMATION_OPEN : Loads an opening Animations::WinAnim as a transition for the name input part of the introduction.
         * - LOAD_ANIMATION_CLOSE : Loads a closing Animations::WinAnim as a transition after the name input part of the introduction.
         * Then, loadNextScreen() loads in _next_gs a game screen according the value of this variable.
         */
        int loadNext = 0;

        /*!
         * \brief A screenshot.
         * \details A screenshot of the frame is taken in update(sf::RenderTexture&). It is used as a background for the opening and closing animations before and after the input part of the introduction.
         */
        sf::Texture screenTexture;

    public:
        IntroSceneCtrl(GameData *data);
        GameStatus checkEvent(sf::Event const &event) override;
        GameStatus update(sf::RenderTexture &frame) override;

        void loadNextScreen() override;
        void suspend() override;
        void resume() override;
    };

} // namespace OpMon

#endif
