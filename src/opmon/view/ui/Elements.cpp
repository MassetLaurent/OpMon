/*
Elements.cpp
Author : Cyrielle
Contributor : BAKFR
File under GNU GPL v3.0 license
 */
#include "Elements.hpp"

#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <cmath>

#include "src/utils/ResourceLoader.hpp"

/* Converts degrees to radians */
#define DEG *(3.14159265 / 180)

namespace OpMon {
    namespace Ui {

        MapLayer::MapLayer(sf::Vector2i size, const int tilesCodes[], sf::Texture &tileset)
        : tileset(tileset){
            tiles.setPrimitiveType(sf::Quads);
            tiles.resize(size.x * size.y * 4);

            for(int i = 0; i < size.y; i++) {
                for(int j = 0; j < size.x; j++) {

                    //The software we use (Tiled map editor) starts the first tile at 1, and leaves 0 for void. This line substracts one to every square.

                    int tileNumber = tilesCodes[(i * size.x) + j] - 1;

                    //Now that every void (0) became -1, this replaces every -1 by the "official" void tile.
                    if(tileNumber == -1) {
                        tileNumber = 257; //official void tile
                    }

                    int tx = tileNumber % (tileset.getSize().x / 32);
                    int ty = tileNumber / (tileset.getSize().x / 32);

                    sf::Vertex *quad = &tiles[((i * size.x) + j) * 4];

                    quad[0].position = sf::Vector2f(j * 32, i * 32);
                    quad[1].position = sf::Vector2f((j + 1) * 32, i * 32);
                    quad[2].position = sf::Vector2f((j + 1) * 32, (i + 1) * 32);
                    quad[3].position = sf::Vector2f(j * 32, (i + 1) * 32);

                    quad[0].texCoords = sf::Vector2f(tx * 32, ty * 32);
                    quad[1].texCoords = sf::Vector2f((tx + 1) * 32, ty * 32);
                    quad[2].texCoords = sf::Vector2f((tx + 1) * 32, (ty + 1) * 32);
                    quad[3].texCoords = sf::Vector2f(tx * 32, (ty + 1) * 32);
                }
            }
        }

        void MapLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const {
            states.transform *= getTransform();

            states.texture = &tileset;

            target.draw(tiles, states);
        }

        Transformation::Transformation(unsigned int const &time, MovementData const md, RotationData const rd, ScaleData const sd, sf::Transform *sprite)
          : time(time)
          , md(md)
          , rd(rd)
          , sd(sd)
          , sprite(sprite) {
            if(sprite != nullptr) {
                attach(sprite, true);
            }
        }

        Transformation::~Transformation() {
        }

        sf::Transform *Transformation::attach(sf::Transform *sprite, bool replace) {
            if(this->sprite != nullptr && !replace) {
                return nullptr;
            }
            t = 0;
            sf::Transform *oldSprite = detach();
            this->sprite = sprite;
            return oldSprite;
        }

        sf::Transform *Transformation::detach() {
            sf::Transform *toReturn = sprite;
            sprite = nullptr;
            return toReturn;
        }

        float Transformation::calcFormula(std::vector<double> formula, FormulaMode mode, int t) {
            float toReturn = 0.0;
            switch(mode) {
            case FormulaMode::POLYNOMIAL:
                for(unsigned int i = 0; i < formula.size(); i++) {
                    toReturn += formula[i] * pow((double)t, (double)i);
                }
                break;
            case FormulaMode::MULTIFUNCTIONS:
                unsigned int i = 0;
                while(i < formula.size()) { //Warning : if an incorrect function is put here, with an incorrect size, it will very likely cause a segmentation fault and the game will crash.
                    switch((int)round(formula[i])) {
                    case F_NOTHING: //*1 function : {0, a} -> a
                        toReturn += formula[i + 1];
                        i += 2;
                        break;
                    case F_SINE: //Sine function : {1, a, b, c} -> a·sin(b·t + c)
                        toReturn += (formula[i + 1] * std::sin(formula[i + 2] * (double)t + formula[i + 3]));
                        i += 4;
                        break;
                    case F_COSINE: //Cosine function : {2, a, b, c} -> a·cos(b·t + c)
                        toReturn += (formula[i + 1] * std::cos(formula[i + 2] * (double)t + formula[i + 3]));
                        i += 4;
                        break;
                    case F_ABS: //Absolute value function : {3, a, b, c} -> a·|b·t + c|
                        toReturn += (formula[i + 1] * std::abs(formula[i + 2] * (double)t + formula[i + 3]));
                        i += 4;
                        break;
                    case F_POW: //Power function : {4, a, b, c} -> a·(t + b)^c
                        toReturn += (formula[i + 1] * std::pow((double)t + formula[i + 2], formula[i + 3]));
                        t += 4;
                        break;
                    }
                }
                break;
            }
            return toReturn;
        }

        sf::Vector2f Transformation::rotateVector(const sf::Vector2f &vect, double angle) {
            return sf::Vector2f(((vect.x * std::cos(angle DEG)) - (vect.y * std::sin(angle DEG))), ((vect.x * std::sin(angle DEG)) + (vect.y * std::cos(angle DEG))));
        }

        bool Transformation::empty() {
            return sprite == nullptr;
        }

        bool Transformation::apply() {
            if(empty() || (t > time && time != 0)) {
                return false;
            }

            //Translation
            if(md.init) {
                sf::Vector2f calc = sf::Vector2f(calcFormula(md.xformula, md.modeX, t), calcFormula(md.yformula, md.modeY, t)); //Calculates the new coordinates
                sprite->translate(rotateVector(calc - lastTranslation, -lastRotation));                                         //Moves by the difference between the old coordinates and the new one.
                                                                                                                                //Rotates the vector to ignore the effects due to the rotation.
                lastTranslation = calc;
            }

            //Rotation
            if(rd.init) {
                float rotation = calcFormula(rd.formula, rd.formulaMode, t);
                sprite->rotate(rotation - lastRotation, rd.origin);
                lastRotation = rotation;
            }

            //Scaling
            if(sd.init) {
                sf::Vector2f calc = sf::Vector2f(calcFormula(sd.xformula, sd.modeX, t), calcFormula(sd.yformula, sd.modeY, t)); //Calculates the new scale
                sprite->scale(calc.x / lastScaling.x, calc.y / lastScaling.y, sd.origin.x, sd.origin.y);                        //Scaling relatively to the last scale to not multiply the different scalings
                lastScaling = calc;
            }

            t++;

            return true;
        }

        const MovementData Transformation::newMovementData(FormulaMode modeX, FormulaMode modeY, std::vector<double> xformula, std::vector<double> yformula, bool relative) {
            MovementData toReturn;
            toReturn.modeX = modeX;
            toReturn.modeY = modeY;
            toReturn.xformula = xformula;
            toReturn.yformula = yformula;
            toReturn.relative = relative;
            toReturn.init = true;
            return toReturn;
        }

        const RotationData Transformation::newRotationData(FormulaMode formulaMode, std::vector<double> formula, sf::Vector2f origin) {
            RotationData toReturn;
            toReturn.formulaMode = formulaMode;
            toReturn.formula = formula;
            toReturn.origin = origin;
            toReturn.init = true;
            return toReturn;
        }

        const ScaleData Transformation::newScaleData(FormulaMode modeX, FormulaMode modeY, std::vector<double> xformula, std::vector<double> yformula, sf::Vector2f origin) {
            ScaleData toReturn;
            toReturn.modeX = modeX;
            toReturn.modeY = modeY;
            toReturn.xformula = xformula;
            toReturn.yformula = yformula;
            toReturn.init = true;
            toReturn.origin = origin;
            return toReturn;
        }

        std::vector<double> Transformation::inverseFormula(std::vector<double> formula, FormulaMode mode) {
            std::vector<double> toReturn;
            switch(mode) {
            case FormulaMode::POLYNOMIAL:
                for(unsigned int i = 0; i < formula.size(); i++) {
                    formula[i] = 0.0 - formula[i];
                }
                break;

            case FormulaMode::MULTIFUNCTIONS:
                int jump = 0;
                for(unsigned int i = 0; i < formula.size(); i++) {
                    if(formula[i] == 0) {
                        jump = 2;
                    } else {
                        jump = 4;
                    }
                    for(int j = 1; j < jump; j++) {
                        formula[i + j] = -formula[i + j];
                    }
                    i += jump;
                }
                break;
            }
            return toReturn;
        }

        Transformation Transformation::inverse() {
            std::vector<double> xformula;
            std::vector<double> yformula;
            std::vector<double> rotFormula;
            if(md.init) {
                xformula = inverseFormula(md.xformula, md.modeX);
                yformula = inverseFormula(md.yformula, md.modeY);
            }
            if(rd.init) {
                rotFormula = inverseFormula(rd.formula, rd.formulaMode);
            }
            return Transformation(time, md.init ? newMovementData(md.modeX, md.modeY, xformula, yformula, md.relative) : md, rd.init ? newRotationData(rd.formulaMode, rotFormula, rd.origin) : rd, sd, sprite);
        }

        sf::Vector2f Transformation::spriteCenter(const sf::Sprite &spr) {
            return sf::Vector2f(spr.getGlobalBounds().width / 2, 3 * spr.getGlobalBounds().height / 4);
        }

    } // namespace Ui
} // namespace OpMon
