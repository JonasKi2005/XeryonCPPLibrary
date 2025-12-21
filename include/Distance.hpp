#pragma once
#include "Stage.hpp"
#include <cstdint>

/**
 * @brief A class representing physical distances and angles with unit conversion
 *
 * This class handles different units of measurement and provides automatic
 * conversion between them. It supports both linear (mm, µm, nm, inch) and
 * angular (mrad, rad, deg) measurements.
 */
namespace Xeryon
{
    class Distance
    {
    public:
        /**
         * @brief Units of measurement supported by the Distance class
         */
        enum Type
        {
            MM,    ///< Millimeters
            MU,    ///< Micrometers
            NM,    ///< Nanometers
            INCH,  ///< Inches
            MINCH, ///< Micro-inches
            ENC,   ///< Encoder units
            MRAD,  ///< Milliradians
            RAD,   ///< Radians
            DEG    ///< Degrees
        };
        /**
         * @brief Constructs a Distance object with a specific value and unit type
         * @param value The numerical value in the specified unit
         * @param type The unit type from the Type enum
         */
        explicit Distance(long double value, Type type);

        /**
         * @brief User-defined literal for millimeter distances
         * @param value The distance in millimeters
         * @return A Distance object representing the millimeter value
         */
        friend Distance operator"" _mm(long double value);
        friend Distance operator"" _mu(long double value);
        friend Distance operator"" _nm(long double value);
        friend Distance operator"" _inch(long double value);
        friend Distance operator"" _enc(long double value);
        friend Distance operator"" _mrad(long double value);
        friend Distance operator"" _rad(long double value);
        friend Distance operator"" _deg(long double value);

        /**
         * @brief Adds two Distance objects
         * @param other The Distance to add to this one
         * @return A new Distance object with the sum, using this object's unit type
         */
        Distance operator+(Distance other) { return Distance(this->value_si_ + other.value_si_, this->type_); }

        /**
         * @brief Subtracts another Distance from this one
         * @param other The Distance to subtract from this one
         * @return A new Distance object with the difference, using this object's unit type
         */
        Distance operator-(Distance other) { return Distance(this->value_si_ - other.value_si_, this->type_); }

        /**
         * @brief Multiplies two Distance objects
         * @param other The Distance to multiply with this one
         * @return A new Distance object with the product, using this object's unit type
         */
        Distance operator*(Distance other) { return Distance(this->value_si_ * other.value_si_, this->type_); }

        /**
         * @brief Divides this Distance by another Distance
         * @param other The Distance to divide by
         * @return A new Distance object with the quotient, using this object's unit type
         */
        Distance operator/(Distance other) { return Distance(this->value_si_ / other.value_si_, this->type_); }

        /**
         * @brief Divides this Distance by a scalar value
         * @param other The scalar value to divide by
         * @return The quotient as a double
         */
        double operator/(double other) { return this->value_si_ / other; }

        /**
         * @brief Compares two Distance objects for equality
         * @param other The Distance to compare with
         * @return true if the distances are equal, false otherwise
         */
        bool operator==(Distance other) { return other.value_si_ == this->value_si_; }

        /**
         * @brief Gets the unit type of this Distance
         * @return The Type enum value representing the unit
         */
        Type type() const { return type_; }

        /**
         * @brief Checks if this Distance represents an angular measurement
         * @return true if the unit is angular (MRAD, RAD, or DEG), false otherwise
         */
        bool isAngular() const;
        operator long double() const { return this->value_si_; }

    private:
        /**
         * @brief Converts a value from the given unit type to SI units (meters or radians)
         * @param value The value to convert
         * @param type The unit type to convert from
         * @return The value in SI units
         */
        static long double toSI(long double value, Type type);

        long double value_si_; ///< The stored value in SI units (meters or radians)
        Type type_;            ///< The display/interface unit type for this Distance
    };

    /**
     * @brief User-defined literal for millimeter distances
     * @param value The distance in millimeters
     * @return A Distance object representing the millimeter value
     */
    Distance operator"" _mm(long double value);
    // integer-literal overloads (allow usage like 5_mm)
    Distance operator"" _mm(unsigned long long value);

    /**
     * @brief User-defined literal for micrometer distances
     * @param value The distance in micrometers
     * @return A Distance object representing the micrometer value
     */
    Distance operator"" _mu(long double value);
    Distance operator"" _mu(unsigned long long value);

    /**
     * @brief User-defined literal for nanometer distances
     * @param value The distance in nanometers
     * @return A Distance object representing the nanometer value
     */
    Distance operator"" _nm(long double value);
    Distance operator"" _nm(unsigned long long value);

    /**
     * @brief User-defined literal for inch distances
     * @param value The distance in inches
     * @return A Distance object representing the inch value
     */
    Distance operator"" _inch(long double value);
    Distance operator"" _inch(unsigned long long value);

    /**
     * @brief User-defined literal for encoder unit distances
     * @param value The distance in encoder units
     * @return A Distance object representing the encoder unit value
     */
    Distance operator"" _enc(long double value);
    Distance operator"" _enc(unsigned long long value);

    /**
     * @brief User-defined literal for milliradian angles
     * @param value The angle in milliradians
     * @return A Distance object representing the milliradian value
     */
    Distance operator"" _mrad(long double value);
    Distance operator"" _mrad(unsigned long long value);

    /**
     * @brief User-defined literal for radian angles
     * @param value The angle in radians
     * @return A Distance object representing the radian value
     */
    Distance operator"" _rad(long double value);
    Distance operator"" _rad(unsigned long long value);

    /**
     * @brief User-defined literal for degree angles
     * @param value The angle in degrees
     * @return A Distance object representing the degree value
     */
    Distance operator"" _deg(long double value);
    Distance operator"" _deg(unsigned long long value);

};