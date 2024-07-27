#include <fmt/format.h>

#include <iostream>
#include <variant>

namespace helper {
    template <class... Ts> struct overload : Ts... {
        using Ts::operator()...;
    };

    template <class... Ts> overload(Ts...) -> overload<Ts...>;  // no need in C++20, MSVC?
}  // namespace helper

namespace state {
    struct Base {
        // protected:
        unsigned int last_evtid_{0};
        unsigned int last_stateid_{0};

        // Base operator=(const Base& v) {
        //     Base tmp{v.last_evtid_, v.last_stateid_};
        //     std::cout << "copy" << std::endl;
        //     return tmp;
        // }
    };
    struct PlayerAliveWithPower : state::Base {
        unsigned int health_{0};
        unsigned int remainingLives_{0};
    };

    struct PlayerAlive : state::Base {
        unsigned int health_{0};
        unsigned int remainingLives_{0};
    };

    struct PlayerDead : state::Base {
        unsigned int remainingLives_{0};
    };

    struct GameOver : state::Base {};
}  // namespace state

/*
[[[STATE MACHINE : DEFINED STATE]]]
*/
enum STATE {
    ALIVE_WITH_POWER = 1,
    ALIVE,
    DEAD,
    GAMEOVER,
};
using HealthState = std::variant<state::PlayerAliveWithPower, state::PlayerAlive, state::PlayerDead,
                                 state::GameOver>;

/*
[[[STATE MACHINE :DEFINED EVENT]]]
*/
enum EVENT {
    NONE,
    HITBYMONSTER = 1,
    HEAL,
    RESTART,
};
namespace event {
    struct HitByMonster {
        unsigned int forcePoints_{0};
        unsigned int evtid_{HITBYMONSTER};
    };

    struct Heal {
        unsigned int points_{0};
        unsigned int evtid_{HEAL};
    };

    struct Restart {
        unsigned int startHealth_{0};
        unsigned int evtid_{RESTART};
    };
}  // namespace event

using PossibleEvent = std::variant<event::HitByMonster, event::Heal, event::Restart>;

/*
[[[STATE MACHINE ON-EVENT-PROCESS]]]
*/

#if 0
// [state1]
HealthState onEvent(const state::PlayerAliveWithPower& alive, const event::HitByMonster& monster,
                    bool flg) {
    std::cout << fmt::format("PlayerAliveWithPower -> HitByMonster force {}\n",
                             monster.forcePoints_);
    if (flg) {
        return state::PlayerAliveWithPower{alive.health_, alive.remainingLives_, monster.evtid_};
    }

    if (alive.health_ > monster.forcePoints_) {
        return state::PlayerAliveWithPower{alive.health_ - monster.forcePoints_,
                                           alive.remainingLives_};
    }

    if (alive.remainingLives_ > 0) {
        return state::PlayerDead{alive.remainingLives_ - 1};
    }

    return state::GameOver{};
}
#endif

// [state2]
HealthState onEvent(const state::PlayerAlive& alive, const event::HitByMonster& monster, bool flg) {
    std::cout << fmt::format("PlayerAlive -> HitByMonster force {}\n", monster.forcePoints_);

    if (flg) {
        return state::PlayerAlive{monster.evtid_, ALIVE, alive.health_, alive.remainingLives_};
    }

    if (alive.health_ > monster.forcePoints_) {
        // return state::PlayerAlive{alive.health_ - monster.forcePoints_, alive.remainingLives_,
        //                           monster.evtid_};
        return state::PlayerAlive{monster.evtid_, ALIVE, alive.health_ - monster.forcePoints_,
                                  alive.remainingLives_};
    }

    if (alive.remainingLives_ > 0) {
        return state::PlayerDead{monster.evtid_, ALIVE, alive.remainingLives_ - 1};
    }

    return state::GameOver{monster.evtid_, ALIVE};
}

#if 0
// [state1]
HealthState onEvent(const state::PlayerAliveWithPower& alive, const event::Heal& healingBonus,
                    bool flg) {
    std::cout << fmt::format("PlayerAliveWithPower -> Heal points {}\n", healingBonus.points_);
    return state::PlayerAliveWithPower{alive.health_ + healingBonus.points_, alive.remainingLives_};
}
#endif

// [state2]
HealthState onEvent(const state::PlayerAlive& alive, const event::Heal& healingBonus, bool flg) {
    if (flg) {
        // not-impl
        return state::PlayerAliveWithPower{healingBonus.evtid_, ALIVE_WITH_POWER, alive.health_,
                                           alive.remainingLives_};
    }

    std::cout << fmt::format("PlayerAlive -> Heal points {}\n", healingBonus.points_);

    // return state::PlayerAliveWithPower{alive.health_ + healingBonus.points_,
    // alive.remainingLives_};
    return state::PlayerAlive{healingBonus.evtid_, ALIVE, alive.health_ + healingBonus.points_,
                              alive.remainingLives_};
}

// [state3]
HealthState onEvent(const state::PlayerDead& dead, const event::Restart& restart, bool flg) {
    std::cout << fmt::format("PlayerDead -> restart\n");

    return state::PlayerAlive{restart.evtid_, DEAD, restart.startHealth_, dead.remainingLives_};
}

// [state4]
HealthState onEvent(const state::GameOver& over, const event::Restart& restart, bool flg) {
    std::cout << fmt::format("GameOver -> restart\n");

    std::cout << "Game Over, please restart the whole game!\n";

    // return over;
    return state::GameOver{restart.evtid_, GAMEOVER};
}

// [state:EXCEPT]
HealthState onEvent(const auto&, const auto&, bool flg) {
    throw std::logic_error{"Unsupported state transition"};
}

////////////////////////////////////////////////////////////////////////////////////////////
// HealthState f2(auto a) { return a; }  // 引数型

#if 1
// 　AIの構造体
struct EnemyStateAI {
    int hp;  //  体力
    int evtid;
    bool flg;
};

// skip-condition
EnemyStateAI aiStates[] = {
    {80, HITBYMONSTER, true},  // TODO
    {80, HITBYMONSTER, true},
    {80, HEAL, false},
};
#endif

/*
[[[STATE MACHINE]]]
*/
class GameStateMachine {
  public:
    void startGame(unsigned int health, unsigned int lives) {
        // state_ = state::PlayerAlive{ALIVE, NONE, health, lives};
        state_ = state::PlayerAlive{ALIVE, NONE, health, lives};
    }

    bool checkEvent(int event) {
        return aiStates[event].flg;
#if 0
    switch (event) {
      case HITBYMONSTER:
        // state Overwrite
        std::cout << "### event0" << std::endl;
        // state_ = state::PlayerAliveWithPower{200};
        break;
      case HEAL:
        // no-process
        break;
      case RESTART:
        // no-process
        return true;
      default:
        // state Overwrite
        std::cout << "### EXCEPTION!!!! (not-impl)" << std::endl;
        break;
    }
#endif
        return false;
    }
    void processEvent(const PossibleEvent& event) {
#if 0
    switch (state_.index()) {
      case 0:
        // std::cout << "aa" << std::get<0>(state_) << std::endl;
        std::cout << "### state0" << std::endl;
        // std::cout << "### state0" << event << std::endl;
        //<< aiStates[0].state;  << std::endl;
        break;
      case 1:
        std::cout << "### state1" << std::endl;
        break;
      case 2:
        std::cout << "### state2" << std::endl;
        break;
    }
#endif

        // 【tips】いまさらautoキーワード
        // https://qiita.com/ai56go/items/8df80a8416735b21a7f2

        state_ = std::visit(helper::overload{[this](const auto& state, const auto& evt) {
                                // state = state_;

                                bool nothing = false;
                                // if (checkEvent(event.index())) {
                                if (checkEvent(evt.evtid_)) {
                                    std::cout << "### event1 NOTHING" << evt.evtid_ << std::endl;
                                    // nothing = true;
                                    //  return state_;
                                }

                                // call event-process
                                // switch(auto i=f2(0)){
                                if (typeid(state) == typeid(state::PlayerAliveWithPower)) {
                                    std::cout << typeid(state).name() << std::endl;
                                    std::cout << fmt::format("====my-event\n");
                                }

                                return onEvent(state, evt, nothing);
                            }},
                            state_, event);
    }

#if 1
    void reportCurrentState() {
        std::visit(helper::overload{
                       [](const state::PlayerAliveWithPower& alive) {
                           std::cout << fmt::format(
                               "PlayerAliveWithPower {} remaining lives {} (last-evt:{}, sts:{})\n",
                               alive.health_, alive.remainingLives_, alive.last_evtid_,
                               alive.last_stateid_);
                       },
                       [](const state::PlayerAlive& alive) {
                           std::cout << fmt::format(
                               "PlayerAlive {} remaining lives {} (last-evt:{}, sts:{})\n",
                               alive.health_, alive.remainingLives_, alive.last_evtid_,
                               alive.last_stateid_);
                       },
                       [](const state::PlayerDead& dead) {
                           std::cout << fmt::format(
                               "PlayerDead, remaining lives {} (last-evt:{}, sts:{})\n",
                               dead.remainingLives_, dead.last_evtid_, dead.last_stateid_);
                       },
                       [](const state::GameOver& over) {
                           std::cout << fmt::format("GameOver (last-evt:{}, sts:{})\n",
                                                    over.last_evtid_, over.last_stateid_);
                       }},
                   state_);
    }
#endif

  private:
    HealthState state_;
};

/*
[[[TEST_EVENT]]]
*/
void GameHealthFSMTest() {
    std::cout << fmt::format("sizeof(HealthState):   {}\n", sizeof(HealthState));
    std::cout << fmt::format("sizeof(PossibleEvent): {}\n", sizeof(PossibleEvent));

    GameStateMachine game;
    game.startGame(100, 1);

    try {
        // game.processEvent(event::HitByMonster{30, 2});
        game.processEvent(event::HitByMonster{30});
        game.reportCurrentState();

        // game.processEvent(event::HitByMonster{30, 0});
        game.processEvent(event::HitByMonster{30});
        game.reportCurrentState();

        //
        game.processEvent(event::Heal{10});
        game.reportCurrentState();
        game.processEvent(event::HitByMonster{30});
        game.reportCurrentState();

        game.processEvent(event::HitByMonster{30});
        game.reportCurrentState();

        // game.processEvent(event::HitByMonster{30});
        // game.reportCurrentState();

        std::cout << fmt::format("====1\n");
        game.processEvent(event::Restart{110});
        game.reportCurrentState();

#if 1
        game.processEvent(event::HitByMonster{60});
        game.reportCurrentState();
        game.processEvent(event::HitByMonster{80});
        game.reportCurrentState();

        std::cout << fmt::format("====\n");
        game.processEvent(event::Restart{110});
        game.reportCurrentState();
#endif

    } catch (std::exception& ex) {
        std::cout << "Exception! " << ex.what() << '\n';
    }
}
