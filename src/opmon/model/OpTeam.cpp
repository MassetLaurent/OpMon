/*
  OpTeam.cpp
  Author : Cyrielle
  File under GNU GPL v3.0 license
*/
#include "OpTeam.hpp"

#include "src/opmon/model/Enums.hpp"
#include "src/opmon/model/OpMon.hpp"

namespace OpMon {

    OpTeam::~OpTeam() {
        for(OpMon *op : opteam) {
            if(op != nullptr) {
                delete(op);
            }
        }
    }

    OpTeam::OpTeam(std::string const &name) {
        this->name = name;
    }

    void OpTeam::heal() {
        for(OpMon *opmon : opteam) {
            opmon->heal(opmon->getStatHP());
            opmon->confused = false;
            opmon->setStatus(Status::NOTHING);
        }
    }

    bool OpTeam::addOpMon(OpMon *toAdd) {
        if(opteam.size() < 6) {
            opteam.push_back(toAdd);
            return true;
        } else {
            return false;
        }
    }

    OpMon *OpTeam::removeOp(int number) {
        if(opteam.size() == 1) { //If there is only one OpMon, return nullptr
            return nullptr;
        }
        OpMon *toReturn = opteam[number];
        opteam.erase(opteam.begin() + number);
        return toReturn;
    }

    OpMon *OpTeam::operator[](int id) const {
        return opteam[id];
    }

    bool OpTeam::isKo() const {
        unsigned int ko = 0;
        for(OpMon *opmon : opteam) {
            if(opmon->getHP() <= 0) {
                ko++;
            }
        }
        return (ko == opteam.size());
    }

    int OpTeam::getSize() const {
        return opteam.size();
    }

} // namespace OpMon
