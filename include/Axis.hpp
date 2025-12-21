#pragma once

#include "Stage.hpp"
#include "Distance.hpp"
#include <iostream>
#include <map>
#include <chrono>
#include <optional>
#include <thread>
#include <condition_variable>
#include <cstdint>

constexpr bool DISABLE_WAITING = false;
constexpr bool DEBUG_MODE = false;
const int DEFAULT_POLI_VALUE = 200;

/**
 * @author Jonas King
 * @brief Axis class abstracts a single axis from the xeryon controller
 * @date 12.11.2025
 */
namespace Xeryon
{
    class Controller; // forward declaration
    class Axis
    {
    public:
        /**
         * @brief Construct an Axis bound to a Controller and a Stage
         * @param controller Pointer to the owning Controller
         * @param stage Stage configuration for this axis
         * @param letter Single-letter identifier for the axis (e.g. 'A')
         */
        Axis(Controller *controller, Stage stage, char letter);

        /**
         * @brief Search for the encoder index in the given direction
         * @param direction Search direction (1 or -1). 0 may trigger an auto-direction search
         * @param forceWaiting If true, wait synchronously for the index to be found
         * @return true if index was found and valid, false otherwise
         */
        bool findIndex(int direction = 0, bool forceWaiting = false);

        /**
         * @brief Move the axis in the direction implied by the Distance value
         * @param d The distance or direction value to move (can be positive/negative)
         */
        void move(const Distance &d);

        /**
         * @brief Set the desired position (DPOS) for the axis
         * @param d Target position as a Distance object
         * @param outputToConsole If true print status messages to stdout
         * @param forceWaiting If true block until movement completes
         * @return true if the command succeeded and the position was reached
         */
        bool setDPOS(const Distance &d, bool outputToConsole = true, bool forceWaiting = false);

        /** Brief setter helpers for trigger/reference values */
        void setTRGS(const Distance &value);
        void setTRGW(const Distance &value);
        void setTRGP(const Distance &value);
        void setTRGN(const Distance &value);

        /**
         * @brief Read the current desired/encoder positions
         * @return Current DPOS as Distance
         */
        Distance getDPOS();
        Distance getEPOS();

        /**
         * @brief Get or set the display/working unit for this axis
         */
        Distance::Type getUnit() { return this->unit_; }
        void setUnit(Distance::Type unit);

        /**
         * @brief Move the axis by a relative step value
         * @param value Step to apply
         * @param forceWaiting If true, wait for the movement to finish
         */
        void step(const Distance &value, bool forceWaiting = false);

        /**
         * @brief Enable/disable logging of axis messages
         */
        void startLogging();
        void endLogging();

        /**
         * @brief Get the current frequency reported by the axis
         * @return Frequency value (FREQ)
         */
        int32_t getFrequency();

        /**
         * @brief Set a configuration setting on the axis
         * @param tag Setting name
         * @param value Value to set (string to allow reading from settings file)
         * @param fromSettingsFile Whether this value originated from a settings file
         * @param doNotSendThrough If true do not send this to the controller immediately
         */
        void setSetting(const std::string &tag, const std::string &value, bool fromSettingsFile = false, bool doNotSendThrough = false);

        /**
         * @brief Start a scan (continuous movement) in the given direction
         * @param direction 1 or -1
         * @param execTime Optional duration to run the scan
         * @param unitLimit If true wait until unit limits (ends) are reached
         */
        void startScan(int direction, std::chrono::duration<double> execTime = std::chrono::duration<double>(0), bool unitLimit = false);
        void stopScan();

        /**
         * @brief Set motor speed (controller units)
         */
        void setSpeed(int32_t value);

        /**
         * @brief Retrieve a previously stored setting
         * @return optional value present if the setting exists
         */
        std::optional<int32_t> getSetting(const std::string &tag);

        void setPTOL(int32_t value);
        void setPTO2(int32_t value);
        void reset();
        void stopMovement();

        /** Status bit helpers (return true if corresponding status bit is set) */
        bool isThermalProtection1();
        bool isThermalProtection2();
        bool isForceZero();
        bool isMotorOn();
        bool isClosedLoop();
        bool isEncoderAtIndex();
        bool isEncoderValid();
        bool isSearchingIndex();
        bool isPositionReached();
        bool isEncoderError();
        bool isScanning();
        bool isAtRightEnd();
        bool isAtLeftEnd();
        bool isErrorLimit();
        bool isSearchingOptimalFrequency();
        bool isSafetyTimeoutTriggered();
        bool isPositionFailTriggered();

        /**
         * @brief Return the axis identifier letter
         */
        char getLetter();

        /**
         * @brief Apply settings-specific multipliers to values coming from a settings file
         * @param tag The settings tag
         * @param value The value to adjust
         * @return Adjusted integer value ready to be sent to the controller
         */
        int applySettingsMultiplier(const std::string &tag, int32_t value);

        /**
         * @brief Send a raw command string to this axis's controller prefixing with axis letter
         */
        void sendCommand(const std::string &command);

        /**
         * @brief Send all stored settings to the controller
         */
        void sendSettings();

        /**
         * @brief Get the Stage configuration for this axis
         */
        Stage getStage() { return this->stage_; }

        /**
         * @brief Request the axis to stop immediately
         */
        void stop();

        /**
         * @brief Read an internal data value by tag
         */
        std::optional<int> getData(std::string tag);

        void saveSettings();

        /**
         * @brief Convert encoder units (raw controller counts) into a Distance
         * @param value Raw encoder units
         * @param type Target Distance type
         */
        Distance convertEncoderUnitsToUnits(int value, Distance::Type type);

        /**
         * @brief Convert a Distance into encoder units
         */
        double convertUnitsToEncoder(Distance value);

        /**
         * @brief Receive a line of data from the communication layer and process it
         */
        void receiveData(const std::string &data);

    private:
        char axisLetter_;
        Controller *controller_;
        std::map<std::string, int32_t> settings_;
        mutable std::mutex dataMutex_;
        mutable std::mutex settingsMutex_;
        std::map<std::string, int32_t> data_;
        Stage stage_;
        Distance::Type unit_;
        std::condition_variable cv_;
        std::mutex cv_m;
        int updateNB_;
        bool wasValidDPOS_;
        bool isLogging;
        bool error = false;
        int previousEpos_[2];
        int previousTime_[2];

        int massToCFREQ(int mass);
        void sendCommand_(const std::string &command);
        void waitForUpdate_();
        int getStatBitAtIndex(int idx);
        bool isWithinTol(int DPOS);
    };
};