#pragma once

#include <vector>
#include <map>
#include <string>
#include <optional>
#include <memory>
#include "Communication.hpp"
#include "Stage.hpp"
#include "Axis.hpp"
#include "Distance.hpp"

namespace Xeryon
{
    class Communication;
    constexpr bool AUTO_SEND_SETTINGS = true;
    inline std::vector<std::string> NOT_SETTINGS_COMMAND = {"DPOS", "EPOS", "HOME", "ZERO", "RSET", "INDX", "STEP", "MOVE", "STOP", "CONT", "SAVE", "STAT", "TIME", "SRNO", "SOFT", "XLA3", "XLA1", "XRT1", "XRT3", "XLS1", "XLS3", "SFRQ", "SYNC"};
    class Controller
    {
    public:
        /**
         * @brief Create a controller that manages communication and axes
         * @param com_port Serial device path
         * @param baud Baud rate
         * @param settingsFromSettingFile Load settings from file on startup
         * @param pathToSettingsFile Path to the settings file
         */
        Controller(std::string com_port, int baud, bool settingsFromSettingFile = true, std::string pathToSettingsFile = "settings.txt");

        /**
         * @brief Start the controller and all managed axes (opens communication)
         */
        void start();

        /**
         * @brief Return true if the system has only a single axis configured
         */
        bool isSingleAxisSystem();

        /**
         * @brief Stop the controller and close communication
         */
        void stop();

        /**
         * @brief Stop movement for all axes
         */
        void stopMovement();

        /**
         * @brief Reset all axes and re-read settings
         */
        void reset();

        /**
         * @brief Access all managed axes
         * @return Reference to internal vector of unique_ptr<Axis>
         */
        std::vector<std::unique_ptr<Axis>> &getAllAxis();

        /**
         * @brief Add a new axis to the controller
         * @param stage Stage configuration
         * @param axisLetter Single-letter id for the axis
         * @return Pointer to the newly created Axis (owned by controller)
         */
        Axis *addAxis(Stage stage, char axisLetter);

        /**
         * @brief Access the Communication instance used by this controller
         */
        Communication &getCommunication();

        /**
         * @brief Find an axis by its letter identifier
         * @return Pointer to axis if found, otherwise nullptr
         */
        Axis *getAxis(char letter);

        /**
         * @brief Set a master/global setting
         * @param tag Setting name
         * @param value Numeric value
         * @param fromSettingsFile Whether the value was loaded from a settings file
         */
        void setMasterSetting(const std::string &tag, int value, bool fromSettingsFile = false);

    private:
        Communication comm_;
        std::map<std::string, int> masterSettings_;
        std::vector<std::unique_ptr<Axis>> axisList_;
        std::string settingsFilePath_;
        bool useSettingsFile_;
        void readSettings();
        void sendMasterSettings(bool axis = false);
        void saveMasterSettings(bool axis = false);
    };
};
