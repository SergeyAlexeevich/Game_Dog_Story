#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include <iostream>
#include <fstream>

#include "../src/serialization_game/geometry_primitives_serialization.h"
#include "../src/serialization_game/lost_object_serialization.h"
#include "../src/serialization_game/dog_serialization.h"
#include "../src/serialization_game/bag_serialization.h"
#include "../src/serialization_game/game_session_serialization.h"
#include "../src/serialization_game/player_serialization.h"
#include "../src/serialization_game/game_manager_serialization.h"
#include "../work_with_json/json_loader.h"
#include "../application/application.h"

using namespace model;
using namespace std::literals;

namespace model {

    using DogPtr = std::shared_ptr<Dog>;

    bool operator==(const DogPtr& lhs, const DogPtr& rhs) {
        return *lhs == *rhs;
    }

    bool operator==(const SessionPtr& lhs, const SessionPtr& rhs) {
        return *lhs == *rhs;
    }


    std::ostream& operator<<(std::ostream& os, const Position& pos) {
        return os << "["s<< pos.x << "," << pos.y << "]";
    }

    std::ostream& operator<<(std::ostream& os, const Point& point) {
        return os << "["s<< point.x << "," << point.y << "]";
    }

    std::ostream& operator<<(std::ostream& os, const Velocity& velocity) {
        return os << "["s<< velocity.vx << "," << velocity.vy << "]";
    }

    std::ostream& operator<<(std::ostream& os, const LostObject& obj) {
        return os << "LostObject(id=" << obj.GetId() 
                << ", pos=" << obj.GetPosition()
                << ", type=" << obj.GetType() 
                << ", value=" << obj.GetScore()
                << ", road_id=" << obj.GetRoad()
                << ", is_restore=" << obj.IsRestore() 
                << ")";
    }

    std::ostream& operator<<(std::ostream& os, const Dog& dog) {
        os << "Dog(name=" << dog.GetName()
        << ", id=" << dog.GetId()
        << ", position=" << dog.GetCurrentPosition()
        << ", velocity=" << dog.GetVelocity()
        << ", road_id=" << dog.GetRoadId()
        << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Bag& bag) {
        os << "Bag(max_capacity=" << bag.GetMaxCapacity()
        << ", total_points=" << bag.GetTotalPoints()
        << ", count=" << bag.GetCountObj()
        << ", objects=[";
        for(const auto& obj : bag.GetObjects()) {
            os << obj << ", ";
        }
        os << "])";
        return os;
    }
}

namespace domain {
        std::ostream& operator<<(std::ostream& os, const domain::Player& player) {
            os << "Player(token=" << player.GetToken()
            << ", dog=" << *player.GetDog()
            << ", session id=" << player.GetDogId()
            << ", bag=" << player.GetBag().GetCountObj()
            << ", score=" << player.GetScore()
            << ", is_restore=" << player.IsRestored()
            << ")";
            return os;
        }
}

    std::ostream& operator<<(std::ostream& os, const std::vector<app::PlayerPtr>& players) {
        os << "[";
        for(const auto& player : players) {
            os << *player << ", ";
        }
        os << "]";
        return os;
    }

namespace {

    using InputArchive = boost::archive::text_iarchive;
    using OutputArchive = boost::archive::text_oarchive;

    struct Fixture {
        //std::stringstream strm;
        Fixture()
        : ofs("/home/sergey/yandex_practicum/cppbackend/sprint4/problems/state_serialization/solution/save/out.txt", std::ios::out)
        , ifs("/home/sergey/yandex_practicum/cppbackend/sprint4/problems/state_serialization/solution/save/out.txt", std::ios::in) {}
        std::fstream ofs;
        std::fstream ifs;
        OutputArchive output_archive{ofs};
        //OutputArchive output_archive{strm};
    };

}  // namespace

SCENARIO_METHOD(Fixture, "Point serialization"s) {
    GIVEN("A point"s) {
        const Point p{10, 20};
        WHEN("point is serialized"s) {
            output_archive << p;
            ofs.close();
            THEN("it is equal to point after serialization"s) {
                InputArchive input_archive{ifs};
                Point restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Position serialization"s) {
    GIVEN("A position"s) {
        const Position p{10, 20};
        WHEN("position is serialized"s) {
            output_archive << p;
            ofs.close();
            THEN("it is equal to position after serialization"s) {
                InputArchive input_archive{ifs};
                Position restored_position;
                input_archive >> restored_position;
                CHECK(p == restored_position);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Velocity serialization"s) {
    GIVEN("A velocity"s) {
        const Velocity v{10, 20};
        WHEN("velocity is serialized"s) {
            output_archive << v;
            ofs.close();
            THEN("it is equal to velocity after serialization"s) {
                InputArchive input_archive{ifs};
                Velocity restored_velocity;
                input_archive >> restored_velocity;
                CHECK(v == restored_velocity);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "LostObject serialization"s) {
    GIVEN("A lostObject") {
        const LostObject l_o({12.5, 0}, 0.6, 100);
         WHEN("lostObject is serialized"s){
            {
                serialization::LostObjectRepr repr(l_o);
                output_archive << repr;
                ofs.close();
            }

            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::LostObjectRepr repr;
                input_archive >> repr;
                const LostObject restored_lost_object = repr.Restore();
                CHECK(l_o.GetId() == restored_lost_object.GetId());
                CHECK(l_o.GetPosition() == restored_lost_object.GetPosition());
                CHECK(l_o.GetRoad() == restored_lost_object.GetRoad());
                CHECK(l_o.GetScore() == restored_lost_object.GetScore());
                CHECK(l_o.GetType() == restored_lost_object.GetType());
            }
         }
    }

}

SCENARIO_METHOD(Fixture, "Bag serialization"s) {
    GIVEN("A lostObject") {
        const Bag bag = [](){
            const size_t max_capacity = 12;
            Bag bag{max_capacity};
            CHECK(bag.GetTotalPoints() == 0u);
            CHECK(bag.GetMaxCapacity() == max_capacity);
            CHECK(bag.IsFull() == false);
            const LostObject l_o_0({12.5, 0}, 0.6, 100);
            LostObject l_o({12.5, 0}, 0.6, 100,true);
            l_o.SetIdForRestore(LostObject::Id{l_o_0.GetId()});
            bag.AddObject(std::move(l_o));
            CHECK(bag.GetTotalPoints() == 100u);
            CHECK(bag.GetCountObj() == 1u);
            return bag;
        }();
         WHEN("Bag is serialized"s){
            {
                serialization::BagRepr repr(bag);
                output_archive << repr;
                ofs.close();
            }

            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::BagRepr repr;
                input_archive >> repr;
                const Bag restored_bag = repr.Restore();
                CHECK(bag.GetMaxCapacity() == restored_bag.GetMaxCapacity());
                CHECK(bag.GetCountObj() == restored_bag.GetCountObj());
                CHECK(bag.GetTotalPoints() == restored_bag.GetTotalPoints());
                CHECK(bag.IsFull() == restored_bag.IsFull());
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization"s) {
    GIVEN("a dog"s) {
        Dog dog("Pluto"s, 99);
        dog.SetDirection(Direction::SOUTH);
        dog.SetPosition({17.3, 31.5});
        dog.SetVelocity(300); // {0,300}
        dog.SetRoadId(71);

        WHEN("dog is serialized"s) {
            {
                serialization::DogRepr repr(dog);
                output_archive << repr;
                ofs.close();
            }
            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(dog.GetId() == restored.GetId());
                CHECK(dog.GetCurrentPosition() == restored.GetCurrentPosition());
                CHECK(dog.GetDirection() == restored.GetDirection());
                CHECK(dog.GetName() == restored.GetName());
                CHECK(dog.GetOldPosition() == restored.GetOldPosition());
                CHECK(dog.GetRoadId() == restored.GetRoadId());
                CHECK(dog.GetVelocity() == restored.GetVelocity());
                CHECK(dog.GetDirectionToString() == restored.GetDirectionToString());
            }
        }
    }
}
/*
model::Game game = 
     json_loader::LoadGame("/home/sergey/yandex_practicum/cppbackend/sprint4/problems/state_serialization/solution/data/config.json"s);


bool operator==(const app::PlayerPtr& lhs, const app::PlayerPtr& rhs) {
    return *lhs == *rhs;
}

SCENARIO_METHOD(Fixture, "GameSession Serialization"s) {
    GIVEN("A gameSession") {
        GameSession session(*game.FindMap(Map::Id{"map1"s}),77);
        CHECK(*session.GetMapId() == "map1"s);
        CHECK(session.GetDogsList().size() == 0u);
        session.AddDogOnMap("Pluto"s, 1u);
        CHECK(session.GetDogsList().size() == 1u);
        CHECK(session.GetLostObjects().size() == 0u);
        session.AddLostObjects(1);
        CHECK(session.GetLostObjects().size() == 1u);
        

        WHEN("gameSession is serialized"s) {
            {
                serialization::GameSessionRepr repr(session);
                output_archive << repr;
                ofs.close();
            }
            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::GameSessionRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore(game);

                CHECK(session.GetGameSessionId() == restored.GetGameSessionId());
                CHECK(*session.GetMapId() == *restored.GetMapId());
                CHECK(session.GetVelocityOnMap() == restored.GetVelocityOnMap());
                CHECK(session.GetDogsList().begin()->first == restored.GetDogsList().begin()->first);
                CHECK(session.GetDogsList().begin()->second == restored.GetDogsList().begin()->second);
                CHECK(session.GetLostObjects().begin()->first == restored.GetLostObjects().begin()->first);
                CHECK(session.GetLostObjects().begin()->second.GetId() == restored.GetLostObjects().begin()->second.GetId());
                CHECK(session.GetLostObjects().begin()->second.GetScore() == restored.GetLostObjects().begin()->second.GetScore());
            }
        }
    }
}

using AllGameSessionsList = std::unordered_map<size_t    // id сессии
                                        , std::shared_ptr<GameSession>>;

SCENARIO_METHOD(Fixture, "Player Serialization"s) {
    GIVEN("A player"s) {
        std::shared_ptr<GameSession> session = std::make_shared<GameSession>(GameSession{*game.FindMap(Map::Id{"map1"s}),77,1}); // id_map, id_session
        AllGameSessionsList list{{77u, session}};
        session->AddDogOnMap("Pluto"s, 0u);
        session->AddDogOnMap("Scoby"s, 1u);
        session->AddDogOnMap("Snuppi"s, 2u);
        session->AddLostObjects(1);
        domain::Player player(session,session->GetDogsList().begin()->second,100); // shared с сессией, shared с псом, емкость рюкзака
        CHECK(player.GetBagCapacity() == 100u);
        CHECK(player.GetBag().GetCountObj() == 0u);
        player.AddObjInBag(LostObject{{0,50}, 7, 99});
        LostObject orig_obj{{0,50}, 7, 99,true};
        // Выравниваем динамические поля
        orig_obj.SetIdForRestore(model::LostObject::Id{player.GetBag().GetObjects().begin()->GetId()});
        orig_obj.SetRoadId(player.GetBag().GetObjects().begin()->GetRoad());
        CHECK((*player.GetObjInBag().begin()) == orig_obj);

        WHEN("player is serialized"s) {
            {
                serialization::PlayerRepr repr(player);
                output_archive << repr;
                ofs.close(); 
            }
            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::PlayerRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore(list);

                CHECK(player.GetCurrentSession() == restored.GetCurrentSession());
                CHECK(player.GetDog() == restored.GetDog());
                CHECK(player.GetToken() == restored.GetToken());
                CHECK(player.GetBag() == restored.GetBag());
                CHECK(player.GetScore() == restored.GetScore());
                CHECK(player.IsRestored() == restored.IsRestored());
            }

        }
    }
}

SCENARIO_METHOD(Fixture, "GameManager Serialization"s) {
    GIVEN("A gameManager"s) {
        app::GameManager manager(game);
        manager.AddPlayer("map1"s, "Pluto"s);
        manager.AddPlayer("map1"s, "Scoby"s);
        manager.AddPlayer("map1"s, "Snuppi"s);

        manager.AddPlayer("map3"s, "Pluto"s);
        manager.AddPlayer("map3"s, "Scoby"s);
        manager.AddPlayer("map3"s, "Snuppi"s);

            WHEN("gameManager is serialized"s) {
            {
                serialization::GameManagerRepr repr(manager);
                output_archive << repr;
                ofs.close();   
            }
            THEN("it can be deserialized"s) {
                InputArchive input_archive{ifs};
                serialization::GameManagerRepr repr;
                input_archive >> repr;
                auto restored = repr.Restore(game);
                
                THEN("players list data is correctly restored"s) {
                    std::vector<app::PlayerPtr> origin(manager.GetPlayers().begin(), manager.GetPlayers().end());
                    std::vector<app::PlayerPtr> rest(restored.GetPlayers().begin(), restored.GetPlayers().end());
                    
                    // Сортируем по токенам для корректного сравнения
                    std::sort(origin.begin(), origin.end(), [](const auto& lhs, const auto& rhs) {
                        return lhs->GetToken() < rhs->GetToken();
                    });
                    
                    std::sort(rest.begin(), rest.end(), [](const auto& lhs, const auto& rhs) {
                        return lhs->GetToken() < rhs->GetToken();
                    });

                    for(int i = 0; i < origin.size(); ++i) {
                        CHECK(origin[i]->GetCurrentSession()->GetGameSessionId() == rest[i]->GetCurrentSession()->GetGameSessionId());
                        CHECK(origin[i]->GetDog()== rest[i]->GetDog());
                        CHECK(origin[i]->GetBag()== rest[i]->GetBag());
                        CHECK(origin[i]->GetScore()== rest[i]->GetScore());
                        CHECK(origin[i]->IsRestored()== rest[i]->IsRestored());
                        CHECK(origin[i]->GetToken() == rest[i]->GetToken());
                        CHECK(origin[i]->GetRoadId() == rest[i]->GetRoadId());
                    }
                }     

                THEN("playerByToken list data is correctly restored"s) {
                    for(const auto [key, value] : manager.GetTokenIndex()){
                        auto item0 = manager.FindPlayerByToken(std::string(key));
                        auto item = restored.FindPlayerByToken(key);
                        auto token = restored.GetTokenIndex();
                        CHECK(key == token.find(key)->first);
                        CHECK(manager.FindPlayerByToken(key)->GetToken() == restored.FindPlayerByToken(key)->GetToken());
                    }
                }              
            }
        }
    }
}

*/