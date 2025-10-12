#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>

#include <boost/signals2.hpp>

#include "../application/application.h"
#include "../serialization_game/game_manager_serialization.h"

namespace data_persistence {

    using namespace std::literals;
    using std::chrono::milliseconds;

    namespace fs = std::filesystem;
    namespace sig = boost::signals2;

    using TimeType = std::optional<double>;

    class BackupRestoreManager {
    public:
        using SerializeSignal = sig::signal<void(const app::GameManager& manager, double current_time)>;
        using RestoreSignal = sig::signal<void(app::GameManager& manager, const model::Game& game)>;

        BackupRestoreManager (const fs::path& root_path, TimeType save_interval, bool auto_save)
        : root_path_{root_path}
        , temp_root_path_{root_path_}
        , save_interval_{save_interval}
        , auto_save_(auto_save)
        , old_time_{std::chrono::duration_cast<milliseconds>(
                                        std::chrono::steady_clock::now().time_since_epoch()) } {
            auto file_name = "temp_"s + temp_root_path_.filename().string();
            temp_root_path_.replace_filename(file_name);
            if(auto_save){
                period_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::milli>(save_interval_.value()));
            }
        }
        ~BackupRestoreManager() = default;

        void ConnectionToSignals(SerializeSignal& s_signal, RestoreSignal& r_signal);

        double GetOldSaveTime() const noexcept;

        void Restore(app::GameManager& manager, const model::Game& game);

        void Serialize(const app::GameManager& manager);

        void SetAutoSave(bool auto_save);

        void RenameBackupFile();

        void SaveGame(const app::GameManager& manager);

    private:
        const fs::path root_path_;
        fs::path temp_root_path_;
        TimeType save_interval_;
        bool auto_save_ = false;
        double old_save_time_ = 0;
        milliseconds period_{static_cast<uint64_t>(save_interval_.value())};
        milliseconds old_time_;
        boost::signals2::connection connections_serialize_signal_;
        boost::signals2::connection connections_restore_signal_;

        std::mutex mtx_save_;
        std::mutex mtx_rest_;
        std::mutex mtx_rename_;
        std::mutex mtx_;
    };
} // namespace data_persistence