#include "Xeryon.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

using Xeryon::Axis;
using Xeryon::Communication;
using Xeryon::Controller;

Controller::Controller(std::string com_port, int baud, bool settingsFromSettingFile, std::string pathToSettingsFile) : comm_(this, com_port, baud), masterSettings_(), axisList_(), settingsFilePath_(pathToSettingsFile), useSettingsFile_(settingsFromSettingFile)
{
    std::cout << "Created controller\n";
}

void Controller::start()
{
    if (this->axisList_.size() < 1)
    {
        std::cerr << "Cannot start without stages!";
        return;
    }
    comm_.start();
    for (auto &axis : axisList_)
    {
        axis->reset();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (useSettingsFile_)
    {
        this->readSettings();

        if (AUTO_SEND_SETTINGS)
        {
            this->sendMasterSettings();
            for (auto &axis : axisList_)
            {
                axis->sendSettings();
            }
        }
    }

    for (auto &axis : axisList_)
    {
        axis->sendCommand("ENBL=1");
    }

    for (auto &axis : axisList_)
    {
        axis->sendCommand("HLIM=?");
        axis->sendCommand("LLIM=?");
        axis->sendCommand("SSPD=?");
        axis->sendCommand("PTO2=?");
        axis->sendCommand("PTOL=?");
        if (axis->getStage().getEncoderResolutionCommand().find("XRTA") != std::string::npos)
        {
            axis->sendCommand("ENBL=3");
        }
    }
}

bool Controller::isSingleAxisSystem()
{
    return axisList_.size() < 2;
}

void Controller::stop()
{
    for (auto &axis : axisList_)
    {
        axis->stop();
    }
    this->comm_.closeCommunication();
    std::cout << "Programm stopped running\n";
}

void Controller::stopMovement()
{
    for (auto &axis : axisList_)
    {
        axis->stopMovement();
    }
}

void Controller::reset()
{
    for (auto &axis : axisList_)
    {
        axis->reset();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    this->readSettings();
    if (AUTO_SEND_SETTINGS)
    {
        for (auto &axis : axisList_)
        {
            axis->sendSettings();
        }
    }
}

std::vector<std::unique_ptr<Axis>> &Controller::getAllAxis()
{
    return this->axisList_;
}

Axis *Controller::addAxis(Stage stage, char axisLetter)
{
    std::cout << "Creating new axis with letter: " << axisLetter << "\n";
    // Construct the Axis in-place inside the vector to avoid unnecessary
    // heap allocation and to keep ownership simple (vector holds Axis by value).
    axisList_.emplace_back(std::make_unique<Axis>(this, stage, axisLetter));
    // Return a copy (or move) of the newly added Axis. If you prefer returning
    // a reference, change the header to `Axis& addAxis(...)`.
    return axisList_.back().get();
}

Communication &Controller::getCommunication()
{
    return this->comm_;
}

Axis *Controller::getAxis(char letter)
{
    for (auto &axis : axisList_)
    {
        if (axis->getLetter() == letter)
        {
            return axis.get();
        }
    }
    return nullptr;
}

void Controller::setMasterSetting(const std::string &tag, int value, bool fromSettingsFile)
{
    masterSettings_[tag] = value;
    if (!fromSettingsFile)
    {
        this->comm_.sendCommand(tag + "=" + std::to_string(value));
    }
}

void Controller::readSettings()
{
    std::ifstream file;
    file.open(this->settingsFilePath_);

    if (!file.is_open())
    {
        std::cerr << "No settings file found at: " << this->settingsFilePath_ << std::endl;
        return;
    }

    std::string line;
    std::size_t pos;

    while (std::getline(file, line))
    {
        // Check if it's a command and not a comment or blank line
        if (line.find('=') != std::string::npos && line.find('%') != 0)
        {
            // Strip spaces, \n, and \r
            while ((pos = line.find(' ')) != std::string::npos)
                line.erase(pos, 1);
            while ((pos = line.find('\r')) != std::string::npos)
                line.erase(pos, 1);
            while ((pos = line.find('\n')) != std::string::npos)
                line.erase(pos, 1);

            // Default select the first axis
            Axis *axis = axisList_[0].get();

            // Check if axis is specified
            if (line.find(':') != std::string::npos)
            {
                axis = getAxis(line[0]);
                if (axis == nullptr)
                    continue;     // No valid axis? ==> IGNORE and loop further
                line.erase(0, 2); // Strip "X:" from command
            }
            else if (!isSingleAxisSystem())
            {
                // This line doesn't contain ":", so it doesn't specify an axis.
                // BUT it's a multi-axis system ==> so these settings are for the master.

                // Ignore comments
                if ((pos = line.find('%')) != std::string::npos)
                    line.erase(pos);

                pos = line.find('=');
                std::string tag = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                this->setMasterSetting(tag, std::atoi(value.c_str()), true);
                continue;
            }

            // Ignore comments
            if ((pos = line.find('%')) != std::string::npos)
                line.erase(pos);

            pos = line.find('=');
            std::string tag = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            std::cout << "Setting for axis " << axis->getLetter() << ": " << tag << " = " << value << std::endl;
            axis->setSetting(tag, value, true, true); // doNotSendThrough = true
        }
    }

    file.close();
}

void Controller::sendMasterSettings(bool axis)
{
    std::string prefix = "";
    if (!axis)
    {
        prefix = std::string(1, axisList_.front()->getLetter()) + ":";
    }
    for (auto &setting : masterSettings_)
    {
        this->comm_.sendCommand(prefix + setting.first + "=" + std::to_string(setting.second));
    }
}

void Controller::saveMasterSettings(bool axis)
{
    if (!axis)
    {
        comm_.sendCommand("SAVE=0");
    }
    else
    {
        comm_.sendCommand(std::string(1, axisList_.front()->getLetter()) + ":SAVE=0");
    }
}