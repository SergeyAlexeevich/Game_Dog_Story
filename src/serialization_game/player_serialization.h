#pragma once

#include <boost/serialization/serialization.hpp>

#include "../domain_models/player.h"
#include "bag_serialization.h"
#include "../application/game_manager.h"

namespace serialization {

    // PlayerRepr (PlayerRepresentation) - сериализованное представление класса Player
    class PlayerRepr{
    public:
        PlayerRepr() = default;
        explicit PlayerRepr(const domain::Player& player)
        : dog_id_{player.GetDogId()}
        , session_id_{player.GetCurrentSession()->GetGameSessionId()}
        , token_{std::string{player.GetToken()}}
        , bag_repr_{player.GetBag()}
        , score_{player.GetScore()} {
        }

        // Восстанавливаем данные сессии
        [[nodiscard]] domain::Player Restore(const app::AllGameSessionsList& all_sessions) const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

    private:
        size_t dog_id_;
        size_t session_id_;
        std::string token_;
        serialization::BagRepr bag_repr_;
        size_t score_ = 0;
    };

    template <typename Archive>
    inline void PlayerRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& dog_id_;
        ar& session_id_;
        ar& token_;
        ar& bag_repr_;
        ar& score_;
    }

} // namespace serialization