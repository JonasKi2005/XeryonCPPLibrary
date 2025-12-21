#include "Distance.hpp"
#include <stdexcept>

namespace Xeryon
{
    Distance operator""_mm(long double value)
    {
        return Distance(value, Distance::Type::MM);
    }

    Distance operator""_mm(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::MM);
    }

    Distance operator""_mu(long double value)
    {
        return Distance(value, Distance::Type::MU);
    }

    Distance operator""_mu(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::MU);
    }

    Distance operator""_nm(long double value)
    {
        return Distance(value, Distance::Type::NM);
    }

    Distance operator""_nm(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::NM);
    }

    Distance operator""_inch(long double value)
    {
        return Distance(value, Distance::Type::INCH);
    }

    Distance operator""_inch(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::INCH);
    }

    Distance operator""_enc(long double value)
    {
        return Distance(value, Distance::Type::ENC);
    }

    Distance operator""_enc(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::ENC);
    }

    Distance operator""_mrad(long double value)
    {
        return Distance(value, Distance::Type::MRAD);
    }

    Distance operator""_mrad(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::MRAD);
    }

    Distance operator""_rad(long double value)
    {
        return Distance(value, Distance::Type::RAD);
    }

    Distance operator""_rad(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::RAD);
    }

    Distance operator""_deg(long double value)
    {
        return Distance(value, Distance::Type::DEG);
    }

    Distance operator""_deg(unsigned long long value)
    {
        return Distance(static_cast<long double>(value), Distance::Type::DEG);
    }

    Distance::Distance(long double value, Type type) : value_si_(toSI(value, type)), type_(type)
    {
    }

    /**
     * @brief Checks if unit of distance is angular
     * @return true if unit is angular
     */
    bool Distance::isAngular() const
    {
        switch (type_)
        {
        case Type::RAD:
        case Type::MRAD:
        case Type::DEG:
            return true;
        default:
            return false;
        }
    }

    long double Distance::toSI(long double value, Type type)
    {
        switch (type)
        {
        case Type::MM:
            return value * 1e-3; // m
        case Type::MU:
            return value * 1e-6; // m
        case Type::NM:
            return value * 1e-9; // m
        case Type::INCH:
            return value * 0.0254; // m
        case Type::MINCH:
            return value * 0.0000254; // m
        case Type::RAD:
            return value; // rad
        case Type::MRAD:
            return value * 1e-3; // rad
        case Type::DEG:
            return value * M_PI / 180.0; // rad
        case Type::ENC:
            return value; // raw, leave as-is
        default:
            throw std::invalid_argument("Unsupported type");
        }
    }

} // namespace Xeryon
