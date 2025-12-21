#include "Axis.hpp"
#include "Xeryon.hpp"

#include <thread>
#include <chrono>
#include <math.h>
#include <cmath>

using namespace Xeryon;

Axis::Axis(Controller *controller, Stage stage, char letter)
    : axisLetter_(letter),
      controller_(controller),
      stage_(stage),
      unit_(Distance::Type::MM),
      updateNB_(0),
      wasValidDPOS_(false),
      isLogging(false),
      previousEpos_{0, 0},
      previousTime_{0, 0}
{
}

bool Axis::findIndex(int direction, bool forceWaiting)
{
    std::cout << "Started searching for index at axis: " << this->axisLetter_ << "\n";
    this->sendCommand_("INDX=" + std::to_string(direction));
    this->wasValidDPOS_ = false;

    if (DISABLE_WAITING == false || forceWaiting == true)
    {
        std::cout << "Seaching index for axis " << this->axisLetter_ << ".\n";
        this->waitForUpdate_();
        this->waitForUpdate_();
        int i = 0;
        while (!this->isEncoderValid())
        {
            // std::cout << "Stat value: " << this->getData("STAT").value_or(0) << std::endl;
            //  std::cout << "Entered while loop -> encoder is not valid\n";
            if (!this->isSearchingIndex())
            {
                std::cout << "Index is not found, but stopped searching for index.\n";
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    std::cout << "Index found of Axis:" << this->axisLetter_ << " found\n";
    return true;
}

void Axis::move(const Distance &d)
{
    int direction = 0;
    if (d > 0)
    {
        direction = 1;
    }
    else if (d < 0)
    {
        direction = -1;
    }
    this->sendCommand_("MOVE=" + std::to_string(direction));
}

bool Axis::setDPOS(const Distance &d, bool outputToConsole, bool forceWaiting)
{
    long double value = d;
    std::cout << "Value of d = " << d << std::endl;
    int DPOS = this->convertUnitsToEncoder(d);
    std::cout << "Converted dpos = " << DPOS << std::endl;

    error = false;

    this->sendCommand_("DPOS=" + std::to_string(DPOS));
    this->wasValidDPOS_ = false;

    if (DISABLE_WAITING == false || forceWaiting == true || DEBUG_MODE == false)
    {
        while (!(this->isPositionReached() && this->isWithinTol(DPOS)))
        {
            if (this->isAtLeftEnd() || this->isAtRightEnd())
            {
                std::cout << "DPOS is out of range.\n";
                error = true;
                return false;
            }

            if (this->isErrorLimit())
            {
                std::cout << "Position not reached. (5) ELIM Triggered.\n";
                error = true;
                return false;
            }

            if (this->isSafetyTimeoutTriggered())
            {
                std::cout << "Position not reached. (6) TOU2 (Timeout 2) triggered.\n";
                error = true;
                return false;
            }

            if (this->isPositionFailTriggered())
            {
                std::cout << "Position not reached. (8) TOU3 (Timeout 3) triggered, 'position fail' status bit 21 went high.\n";
                error = true;
                return false;
            }

            if (this->isThermalProtection1() || this->isThermalProtection2())
            {
                std::cout << "Position not reached. (7) amplifier error.\n";
                error = true;
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return true;
}

void Axis::setTRGS(const Distance &value)
{
    int v = (int)this->convertUnitsToEncoder(value);
    this->sendCommand("TRGS=" + std::to_string(v));
}

void Axis::setTRGW(const Distance &value)
{
    int v = (int)this->convertUnitsToEncoder(value);
    this->sendCommand("TRGW=" + std::to_string(v));
}

void Axis::setTRGP(const Distance &value)
{
    int v = (int)this->convertUnitsToEncoder(value);
    this->sendCommand("TRGP=" + std::to_string(v));
}

void Axis::setTRGN(const Distance &value)
{
    int v = (int)this->convertUnitsToEncoder(value);
    this->sendCommand("TRGN=" + std::to_string(v));
}

Distance Axis::getDPOS()
{
    return this->convertEncoderUnitsToUnits(this->getData("DPOS").value_or(0), this->unit_);
}

Distance Axis::getEPOS()
{
    return this->convertEncoderUnitsToUnits(this->getData("EPOS").value_or(0), this->unit_);
}

void Axis::setUnit(Distance::Type unit)
{
    this->unit_ = unit;
}

void Axis::step(const Distance &value, bool forceWaiting)
{
    int v = (int)this->convertUnitsToEncoder(value);
    int newDPOS = 0;
    if (this->wasValidDPOS_)
    {
        newDPOS = this->getData("DPOS").value_or(0) + v;
    }
    else
    {
        newDPOS = this->getData("EPOS").value_or(0) + v;
    }

    if (!this->stage_.isLinear())
    {
        int encoderUnitsPerRotation = (int)this->convertUnitsToEncoder(Distance(360, Distance::DEG));
        newDPOS = -encoderUnitsPerRotation / 2 * (newDPOS / (encoderUnitsPerRotation / 2) % 2) + (newDPOS % (encoderUnitsPerRotation / 2));
    }

    this->setDPOS(Distance(newDPOS, Distance::ENC));
    if (!DISABLE_WAITING)
    {
        this->waitForUpdate_();
    }
}

void Axis::startLogging()
{
    this->isLogging = true;
}

void Axis::endLogging()
{
    this->isLogging = false;
}

int Axis::getFrequency()
{
    return this->getData("FREQ").value_or(0);
}

void Axis::setSetting(const std::string &tag, const std::string &value, bool fromSettingsFile, bool doNotSendThrough)
{
    std::string newTag = tag;
    int v = std::atoi(value.c_str());
    if (fromSettingsFile)
    {
        v = this->applySettingsMultiplier(tag, v);
        // std::cout << "Not stuck after applySettingsMultiplier\n";
        if (tag.find("MASS") != std::string::npos)
        {
            newTag = "CFRQ";
        }
    }
    if (tag.find('?') == std::string::npos)
    {
        std::lock_guard<std::mutex> lock(settingsMutex_);
        // std::cout << "Setting setting: " << newTag << " with value: " << value << " in Axis: " << this->axisLetter_ << std::endl;
        this->settings_[newTag] = v;
    }
    if (!doNotSendThrough)
    {
        this->sendCommand_(newTag + "=" + value);
    }
}

void Axis::startScan(int direction, std::chrono::duration<double> execTime, bool unitLimit)
{
    this->sendCommand_("SCAN=" + std::to_string(direction));
    this->wasValidDPOS_ = false;

    if (execTime.count() != 0)
    {
        std::this_thread::sleep_for(execTime);
        this->sendCommand_("SCAN=0");
    }

    if (unitLimit)
    {
        this->waitForUpdate_();
        if (direction > 0)
        {
            while (!this->isAtRightEnd())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else
        {
            while (!this->isAtLeftEnd())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Axis::stopScan()
{
    this->sendCommand_("SCAN=0");
    this->wasValidDPOS_ = false;
}

// TODO: understand what they are converting
void Axis::setSpeed(int value)
{
    int speed = value;
    if (this->stage_.isLinear())
    {
        speed = value;
    }
    else
    {
        speed = value * 100;
    }
    this->setSetting("SSPD", std::to_string(speed));
}

std::optional<int32_t> Axis::getSetting(const std::string &tag)
{
    std::cout << "getSettings call with tag: " << tag << std::endl;
    std::lock_guard<std::mutex> lock(settingsMutex_);
    if (settings_.find(tag) != settings_.end())
        return settings_[tag];
    return std::nullopt;
}

void Axis::setPTOL(int32_t value)
{
    this->setSetting("PTOL", std::to_string(value));
}

void Axis::setPTO2(int32_t value)
{
    this->setSetting("PTO2", std::to_string(value));
}

void Axis::reset()
{
    this->sendCommand("RSET=0");
    this->wasValidDPOS_ = false;
}

void Axis::stopMovement()
{
    this->sendCommand("STOP=0");
    wasValidDPOS_ = false;
}

bool Axis::isThermalProtection1()
{
    return this->getStatBitAtIndex(2) == 1;
}

bool Axis::isThermalProtection2()
{
    return this->getStatBitAtIndex(3) == 1;
}

bool Axis::isForceZero()
{
    return this->getStatBitAtIndex(4) == 1;
}

bool Axis::isMotorOn()
{
    return this->getStatBitAtIndex(5) == 1;
}

bool Axis::isClosedLoop()
{
    return this->getStatBitAtIndex(6) == 1;
}

bool Axis::isEncoderAtIndex()
{
    return this->getStatBitAtIndex(7) == 1;
}

bool Axis::isEncoderValid()
{
    return this->getStatBitAtIndex(8) == 1;
}

bool Axis::isSearchingIndex()
{
    return this->getStatBitAtIndex(9) == 1;
}

bool Axis::isPositionReached()
{
    return this->getStatBitAtIndex(10) == 1;
}

bool Axis::isEncoderError()
{
    return this->getStatBitAtIndex(12) == 1;
}

bool Axis::isScanning()
{
    return this->getStatBitAtIndex(13) == 1;
}

bool Axis::isAtRightEnd()
{
    return this->getStatBitAtIndex(15) == 1;
}

bool Axis::isAtLeftEnd()
{
    return this->getStatBitAtIndex(14) == 1;
}

bool Axis::isErrorLimit()
{
    return this->getStatBitAtIndex(16) == 1;
}

bool Axis::isSearchingOptimalFrequency()
{
    return this->getStatBitAtIndex(17) == 1;
}

bool Axis::isSafetyTimeoutTriggered()
{
    return this->getStatBitAtIndex(18) == 1;
}

bool Axis::isPositionFailTriggered()
{
    return this->getStatBitAtIndex(21) == 1;
}

char Axis::getLetter()
{
    return this->axisLetter_;
}

int Axis::applySettingsMultiplier(const std::string &tag, int32_t value)
{
    // Convert tag checks to simple tests similar to the Python implementation.
    // Many settings in the settings-file use human-friendly units and need
    // to be converted before being sent to the controller.
    if (tag.find("MAMP") != std::string::npos || tag.find("MIMP") != std::string::npos || tag.find("OFSA") != std::string::npos || tag.find("OFSB") != std::string::npos || tag.find("AMPL") != std::string::npos || tag.find("MAM2") != std::string::npos)
    {
        // Use amplitude multiplier.
        return static_cast<int>(std::lround(value * this->stage_.getAmplitudeMultiplier()));
    }
    else if (tag.find("PHAC") != std::string::npos || tag.find("PHAS") != std::string::npos)
    {
        return static_cast<int>(std::lround(value * this->stage_.getPhaseMultiplier()));
    }
    else if (tag.find("SSPD") != std::string::npos || tag.find("MSPD") != std::string::npos || tag.find("ISPD") != std::string::npos)
    {
        // In the settings file SSPD is expressed in mm/s (or deg/s for rotary)
        // and the controller expects a different unit; multiply by the stage speed multiplier.
        return static_cast<int>(std::lround(value * this->stage_.getSpeedMultiplier()));
    }
    else if (tag.find("LLIM") != std::string::npos || tag.find("RLIM") != std::string::npos || tag.find("HLIM") != std::string::npos)
    {
        // Limits are given in mm (or deg) and have to be converted to encoder units
        if (this->stage_.isLinear())
        {
            double enc = this->convertUnitsToEncoder(Distance(static_cast<long double>(value), Distance::MM));
            return static_cast<int>(std::lround(enc));
        }
        else
        {
            double enc = this->convertUnitsToEncoder(Distance(static_cast<long double>(value), Distance::DEG));
            return static_cast<int>(std::lround(enc));
        }
    }
    else if (tag.find("POLI") != std::string::npos)
    {
        // Store POLI in settings map for later use (original Python stored in an attribute)
        // this->settings_["POLI"] = value;
        return value;
    }
    else if (tag.find("MASS") != std::string::npos)
    {
        // Convert mass to CFRQ. The exact conversion depends on device specifics.
        // We provide a placeholder conversion helper; replace with correct formula if known.
        return this->massToCFREQ(value);
    }
    else if (tag.find("ZON1") != std::string::npos || tag.find("ZON2") != std::string::npos)
    {
        if (this->stage_.isLinear())
        {
            double enc = this->convertUnitsToEncoder(Distance(static_cast<long double>(value), Distance::MM));
            return static_cast<int>(std::lround(enc));
        }
        else
        {
            double enc = this->convertUnitsToEncoder(Distance(static_cast<long double>(value), Distance::DEG));
            return static_cast<int>(std::lround(enc));
        }
    }

    // Default: return value unchanged
    return value;
}

void Axis::sendCommand(const std::string &command)
{
    std::cout << "sendCommand with command: " << command << std::endl;
    std::string tag = command.substr(0, command.find('='));
    std::string value = command.substr(command.find('=') + 1);

    if (std::find(NOT_SETTINGS_COMMAND.begin(), NOT_SETTINGS_COMMAND.end(), tag) != NOT_SETTINGS_COMMAND.end())
    {
        this->sendCommand_(command);
    }
    else
    {
        // std::cout << "Setting setting: from original command: " << command << " with tag: " << tag << " with value: " << value << " in Axis: " << this->axisLetter_ << std::endl;
        this->setSetting(tag, value);
    }
}

void Axis::sendSettings()
{
    // Copy settings while holding the lock
    std::cout << "sendSettings called\n";
    std::map<std::string, int32_t> settingsCopy;
    {
        std::lock_guard<std::mutex> lock(settingsMutex_);
        settingsCopy = this->settings_;
    }
    // Lock is released here

    // Now send commands without holding the lock
    this->sendCommand_(this->stage_.getEncoderResolutionCommand());
    for (const auto &pair : settingsCopy)
    {
        // std::cout << "Sending setting: " << pair.first << " with value: " << pair.second << std::endl;
        this->sendCommand_(pair.first + "=" + std::to_string(pair.second));
    }
}

void Axis::stop()
{
    this->sendCommand_("STOP=0");
    this->wasValidDPOS_ = false;
}

int Axis::massToCFREQ(int mass)
{
    // Conversion table to change the value of the setting "MASS" into a value
    // for the setting "CFRQ" (controller frequency). Mirrors the Python
    // implementation provided by the user.
    // mass is expected to be an integer (e.g. grams)
    if (mass <= 50)
        return 100000;
    if (mass <= 100)
        return 60000;
    if (mass <= 250)
        return 30000;
    if (mass <= 500)
        return 10000;
    if (mass <= 1000)
        return 5000;
    return 3000;
}

std::optional<int32_t> Axis::getData(std::string tag)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (data_.find(tag) != data_.end())
        return data_[tag];
    return std::nullopt;
}

void Axis::saveSettings()
{
    this->sendCommand("SAVE=0");
}

Distance Axis::convertEncoderUnitsToUnits(int value, Distance::Type type)
{
    // Convert a value given in encoder units to a Distance in the requested unit type.
    // This follows the original Python logic where encoder units are scaled by the
    // stage encoder resolution.
    long double encRes = static_cast<long double>(this->stage_.getEncoderResolution());
    long double v = static_cast<long double>(value);

    switch (type)
    {
    case Distance::MM:
    {
        // mm: value * encRes / 1e6
        long double out = v * encRes / 1e6L;
        return Distance(out, Distance::MM);
    }
    case Distance::MU:
    {
        // micrometer: value * encRes / 1e3
        long double out = v * encRes / 1e3L;
        return Distance(out, Distance::MU);
    }
    case Distance::NM:
    {
        // nanometer: value * encRes
        long double out = v * encRes;
        return Distance(out, Distance::NM);
    }
    case Distance::INCH:
    {
        // inch: value * encRes / (25.4 * 1e6)
        long double out = v * encRes / (25.4L * 1e6L);
        return Distance(out, Distance::INCH);
    }
    case Distance::MINCH:
    {
        // micro-inch: value * encRes / (25.4 * 1e3)
        long double out = v * encRes / (25.4L * 1e3L);
        return Distance(out, Distance::MINCH);
    }
    case Distance::ENC:
    {
        // encoder units: return as-is
        return Distance(v, Distance::ENC);
    }
    case Distance::MRAD:
    {
        // milliradian: value * encRes / 1e3
        long double out = v * encRes / 1e3L;
        return Distance(out, Distance::MRAD);
    }
    case Distance::RAD:
    {
        // radian: value * encRes / 1e6
        long double out = v * encRes / 1e6L;
        return Distance(out, Distance::RAD);
    }
    case Distance::DEG:
    {
        // degree: value * encRes / ((2*pi)/360 * 1e6)
        long double factor = (2.0L * static_cast<long double>(M_PI) / 360.0L) * 1e6L;
        long double out = v * encRes / factor;
        return Distance(out, Distance::DEG);
    }
    default:
        throw std::runtime_error("Unexpected unit in convertEncoderUnitsToUnits");
    }
}

/**
 * @brief converts units to encoder units
 * @param value The distance that should be converted to encoder units
 * @return double of the equivalent value in encoder units
 */
double Axis::convertUnitsToEncoder(Distance value)
{
    if (value.type() == Distance::ENC)
        return value;
    if (value.isAngular())
    {
        return round(value * 1e6 / this->stage_.getEncoderResolution());
    }
    else
    {
        return round(value * 1e9 / this->stage_.getEncoderResolution());
    }
}

void Axis::sendCommand_(const std::string &command)
{
    std::cout << "sendingCommand_ called with " << command << std::endl;
    std::string prefix = "";
    if (!controller_->isSingleAxisSystem())
    {
        prefix = std::string(1, this->axisLetter_) + ":";
    }
    this->controller_->getCommunication().sendCommand(prefix + command);
}

void Axis::waitForUpdate_()
{

    int wait_nb = 3;
    // if (settings_.count("POLI"))
    // {
    //     wait_nb = wait_nb * getSetting("POLI").value_or(1) / DEFAULT_POLI_VALUE;
    //     if (this->axisLetter_ == 'A')
    //         std::cout << "Wait nb = " << wait_nb << "\n";
    // }

    int start_nb;
    {
        std::lock_guard<std::mutex> lk(cv_m);
        start_nb = this->updateNB_; // ✅ Read with lock
    }

    std::unique_lock<std::mutex> lk(cv_m);
    cv_.wait(lk, [this, start_nb, wait_nb]
             { return updateNB_ - start_nb >= wait_nb; }); // ✅ FIX: >= not
}

int Axis::getStatBitAtIndex(int idx)
{
    return (this->getData("STAT").value_or(0) >> idx) & 1;
}

bool Axis::isWithinTol(int DPOS)
{
    return false;
}

std::string cleanString(std::string s)
{
    // 1. Remove trailing '\n' and '\r'
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
    {
        s.pop_back();
    }

    // 2. Remove all spaces
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

    return s;
}

void Axis::receiveData(const std::string &data)
{
    if (data.find('=') != std::string::npos)
    {
        std::string tag = data.substr(0, data.find('='));
        std::string value = cleanString(data.substr(data.find('=') + 1));

        if (std::find(NOT_SETTINGS_COMMAND.begin(), NOT_SETTINGS_COMMAND.end(), tag) == NOT_SETTINGS_COMMAND.end() && tag != "EPOS" && tag != "DPOS")
        {
            // Let setSetting handle its own locking
            this->setSetting(tag, value, false, true);
        }
        else
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            int32_t value_ = std::atoi(value.c_str());
            this->data_[tag] = value_;
        }

        if (tag.find("EPOS") != std::string::npos)
        {
            this->previousEpos_[0] = this->previousEpos_[1];
            this->previousEpos_[1] = std::atoi(value.c_str());
            {
                std::lock_guard<std::mutex> lk(cv_m);
                this->updateNB_ += 1;
            }
            cv_.notify_all();
        }
        else if (tag.find("TIME") != std::string::npos)
        {
            this->previousTime_[0] = this->previousTime_[1];
            this->previousTime_[1] = std::atoi(value.c_str());

            int t1 = this->previousTime_[0];
            int t2 = this->previousTime_[1];

            if (t2 < t1)
                t2 += 65536;

            if (t2 - t1 > 0)
            {
                std::lock_guard<std::mutex> lock(dataMutex_);
                this->data_["SSPD"] = (this->previousEpos_[1] - this->previousEpos_[0]) / ((t2 - t1) * 10);
            }
        }
    }
}
