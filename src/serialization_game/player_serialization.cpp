#include "player_serialization.h"

namespace serialization {

[[nodiscard]] domain::Player PlayerRepr::Restore(const app::AllGameSessionsList& all_sessions) const {
        // Получаем итератор на нужную сессия
        auto it_session = all_sessions.find(session_id_);

        // Проверяем существование сессии
        if (it_session == all_sessions.end()) {
            throw std::runtime_error("Session is null during player restoration. \nRecovery is not possible!");
        }

        // Получаем список псов в сессии
        auto dog_list = it_session->second->GetDogsList();
        // Получаем итератор на нужного пса
        auto it_dog = dog_list.find(dog_id_);

        // Проверяем существование пса
        if (it_dog == dog_list.end()) {
            throw std::runtime_error("Dog is null during player restoration. \nRecovery is not possible!");
        }

        // Восстанавливаем рюкзак
        model::Bag restored_bag = bag_repr_.Restore();
        // Создаем объект игрока
        domain::Player player{it_session->second, it_dog->second, restored_bag.GetMaxCapacity(), true};
        model::Bag temp = restored_bag;
        player.SetBag(std::move(temp));
        player.SetData(score_, token_);
            
        return player;
    }

} // namespace serialization