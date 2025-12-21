#pragma once

#include <string>
#include <cmath>
#include <optional>
#include <vector>

/**
 * @brief A class representing a Xeryon motion stage with its properties and capabilities
 *
 * This class encapsulates the characteristics and behavior of different Xeryon stage models,
 * including linear and rotary stages. It handles stage-specific parameters such as encoder
 * resolution, speed multipliers, and amplitude/phase settings.
 */

namespace Xeryon
{
    // Constants
    constexpr double AMPLITUDE_MULTIPLIER = 1456.0;
    constexpr double PHASE_MULTIPLIER = 182;
    class Stage
    {
    public:
        /**
         * @brief Enumeration of all supported Xeryon stage models
         *
         * Includes various models of XLS (Linear Stages), XLA (Linear Actuators),
         * and XRTU (Rotary Stages) with different specifications and capabilities.
         */
        enum class Type
        {
            XLS_312,
            XLS_1250,
            XLS_1250_OLD,
            XLS_1250_OLD_2,
            XLS_78,
            XLS_5,
            XLS_1,
            XLS_312_3N,
            XLS_1250_3N,
            XLS_1250_3N_OLD,
            XLS_78_3N,
            XLS_5_3N,
            XLS_1_3N,
            XLA_312,
            XLA_1250,
            XLA_78,
            XLA_OL,
            XLA_OL_3N,
            XLA_312_3N,
            XLA_1250_3N,
            XLA_78_3N,
            XLA_312_5N,
            XLA_1250_5N,
            XLA_78_5N,
            XLA_312_10N,
            XLA_1250_10N,
            XLA_78_10N,
            XLA_312_OLD,
            XLA_1250_OLD,
            XLA_78_OLD,
            XRTA,
            XRTU_40_3,
            XRTU_40_19,
            XRTU_40_49,
            XRTU_40_109,
            XRTU_30_3,
            XRTU_30_19,
            XRTU_30_49,
            XRTU_30_109,
            XRTU_60_3,
            XRTU_60_19,
            XRTU_60_49,
            XRTU_60_109,
            XRTU_30_109_OLD,
            XRTU_40_73_OLD,
            XRTU_40_3_OLD
        };

        /**
         * @brief Constructs a Stage object with specific parameters
         * @param type The model type of the stage
         * @param isLinear Whether the stage is a linear stage (true) or rotary stage (false)
         * @param encoderResolutionCommand The command string for setting encoder resolution
         * @param encoderResolution The resolution of the stage's encoder in native units
         * @param speedMultiplier The multiplier used for speed calculations
         */
        Stage(Type type, bool isLinear, const std::string &encoderResolutionCommand,
              double encoderResolution, double speedMultiplier)
            : type_(type), isLinear_(isLinear), encoderResolutionCommand_(encoderResolutionCommand), encoderResolution_(encoderResolution), speedMultiplier_(speedMultiplier), amplitudeMultiplier_(AMPLITUDE_MULTIPLIER), phaseMultiplier_(PHASE_MULTIPLIER) {}

        /**
         * @brief Gets the model type of the stage
         * @return The Stage::Type enum value representing the stage model
         */
        Type getType() const { return type_; }

        /**
         * @brief Checks if the stage is a linear stage
         * @return true for linear stages, false for rotary stages
         */
        bool isLinear() const { return isLinear_; }

        /**
         * @brief Gets the command string used to set the encoder resolution
         * @return The encoder resolution command string
         */
        const std::string &getEncoderResolutionCommand() const { return encoderResolutionCommand_; }

        /**
         * @brief Gets the encoder resolution in native units
         * @return The encoder resolution value
         */
        double getEncoderResolution() const { return encoderResolution_; }

        /**
         * @brief Gets the speed multiplier used for velocity calculations
         * @return The speed multiplier value
         */
        double getSpeedMultiplier() const { return speedMultiplier_; }

        /**
         * @brief Gets the amplitude multiplier for stage control
         * @return The amplitude multiplier value
         */
        double getAmplitudeMultiplier() const { return amplitudeMultiplier_; }

        /**
         * @brief Gets the phase multiplier for stage control
         * @return The phase multiplier value
         */
        double getPhaseMultiplier() const { return phaseMultiplier_; }

        /**
         * @brief Creates a Stage object based on a command string
         * @param stageCommand The command string identifying the stage type
         * @return An optional Stage object - contains a value if the command is valid,
         *         empty if the command doesn't match any known stage
         */
        static std::optional<Stage> getStage(const std::string &stageCommand);

        /**
         * @brief Creates a new Stage object of the specified type
         * @param type The Stage::Type enum value for the desired stage model
         * @return A Stage object configured for the specified type
         */
        static Stage create(Type type);

        /**
         * @brief Gets a list of all available stage configurations
         * @return A vector containing Stage objects for all supported stage models
         */
        static std::vector<Stage> getAllStages();

    private:
        Type type_;                            ///< The model type of this stage
        bool isLinear_;                        ///< Whether this is a linear stage (true) or rotary stage (false)
        std::string encoderResolutionCommand_; ///< Command string for setting the encoder resolution
        double encoderResolution_;             ///< The resolution of the stage's encoder in native units
        double speedMultiplier_;               ///< Multiplier used for converting speed values
        double amplitudeMultiplier_;           ///< Multiplier used for amplitude calculations
        double phaseMultiplier_;               ///< Multiplier used for phase calculations

        /**
         * @brief Removes all whitespace characters from a string
         * @param str The input string to process
         * @return A new string with all whitespace characters removed
         */
        static std::string removeWhitespace(const std::string &str);
    };
};
