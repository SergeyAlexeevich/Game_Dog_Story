#include "backup_restore_manager.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>

#include "../logging/logger.h"

namespace data_persistence {

    void BackupRestoreManager::Restore(app::GameManager &manager, const model::Game &game) {
        std::lock_guard lgr(mtx_rest_);
        // Проверяем существование директории и файла
        if (!fs::exists(root_path_.parent_path()) || !fs::exists(root_path_)) {
            return;
        }

        // Создаем объект файлового потока и сразу открываем файл для чтения
        std::ifstream ifs(root_path_, std::ios::in);

        // Проверяем открыт ли файл
        if(!ifs.is_open()){
            throw std::runtime_error("Couldn't open the file for writing"s);
        }

        // Создаем архив boost
        boost::archive::text_iarchive backup{ifs};
            
        try{
            // Создаем объект для десериализации
            serialization::GameManagerRepr manager_repr;
            // Десериализуем данные
            backup >> manager_repr;
            // Восстанавливаем объекты
            manager = std::move(manager_repr.Restore(game));
        } catch (const boost::archive::archive_exception& e) {
            throw std::runtime_error("Serialization error: " + std::string(e.what()));
        }

    }

    void BackupRestoreManager::Serialize(const app::GameManager &manager) {
        SaveGame(manager);
    }

    void BackupRestoreManager::ConnectionToSignals(SerializeSignal &s_signal, RestoreSignal &r_signal) {
        connections_restore_signal_ = r_signal.connect(boost::bind(&BackupRestoreManager::Restore
            , this,  boost::placeholders::_1, boost::placeholders::_2));
        connections_serialize_signal_ = s_signal.connect(boost::bind(&BackupRestoreManager::Serialize
            , this,  boost::placeholders::_1));
    }

    double BackupRestoreManager::GetOldSaveTime() const noexcept {
        return old_save_time_;
    }

    void BackupRestoreManager::SetAutoSave(bool auto_save) {
        auto_save_ = auto_save;
    }

    void BackupRestoreManager::RenameBackupFile() {
        std::lock_guard lgr(mtx_);
        // Переименовываем файл
        if (fs::exists(root_path_)) {
            fs::remove(root_path_);
        }

        if(fs::exists(temp_root_path_)){
            fs::rename(temp_root_path_, root_path_);
        }

    }

    void BackupRestoreManager::SaveGame(const app::GameManager& manager) {
        milliseconds current_t = 
                std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
        // Проверяем что включено автосохранение
        if(auto_save_) {
            // Проверяем что достигли необходимого временного интерала для сохранения
            milliseconds time_diff = current_t - old_time_;
            if(time_diff < period_){
                return;
            }
        }

        // Создаем объект файлового потока и сразу открываем файл на запись
        std::ofstream ofs(temp_root_path_, std::ios::out | std::ios::binary);

        // Проверяем открыт ли файл
        if(!ofs.is_open()){
            throw std::runtime_error("Couldn't open the file for writing"s);
        }

        // Создаем архив boost
        boost::archive::text_oarchive backup{ofs};
        // Сериализуем данные
        serialization::GameManagerRepr manager_repr{manager};
            
        try {
            // Сериализуем и сохраняем данные в файл
            backup << manager_repr;
            // Записываем время сохранения
            old_time_ = current_t;
            ofs.close();
            RenameBackupFile();
        } catch (const boost::archive::archive_exception& e) {
            throw std::runtime_error("Serialization error: " + std::string(e.what()));
        }
    }
} // namespace data_persistence