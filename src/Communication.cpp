#include "Communication.hpp"
#include "Xeryon.hpp"
#include <iostream>
#include <fstream>

using namespace Xeryon;

Xeryon::Communication::Communication(Controller *controller, const std::string &COMPort, int baud)
    : controller_(controller), COMPort_(COMPort), baud_(baud), io_(), serial_(io_) {}

Xeryon::Communication::~Communication()
{
    closeCommunication();
}

void Communication::start()
{
    try
    {
        serial_.open(COMPort_);
        configureSerialPort_();

        // Begin asynchronous reading
        startAsyncRead_();

        // Start I/O thread (event loop)
        stop_ = false;
        ioThread_ = std::thread([this]()
                                {
            try {
                io_.run();
            } catch (const std::exception& e) {
                std::cerr << "I/O thread error: " << e.what() << std::endl;
            } });

        std::cout << "Started async communication on " << COMPort_ << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error starting communication: " << e.what() << std::endl;
        throw;
    }
}

void Communication::configureSerialPort_()
{
    serial_.set_option(asio::serial_port_base::baud_rate(baud_));
    serial_.set_option(asio::serial_port_base::character_size(8));
    serial_.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    serial_.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
    serial_.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));
}

void Communication::startAsyncRead_()
{
    serial_.async_read_some(
        asio::buffer(readBuffer_),
        [this](const asio::error_code &ec, std::size_t bytesTransferred)
        {
            handleRead_(ec, bytesTransferred);
        });
}

void Communication::handleRead_(const asio::error_code &ec, std::size_t bytesTransferred)
{
    if (stop_)
        return;

    if (!ec)
    {
        for (std::size_t i = 0; i < bytesTransferred; ++i)
        {
            char c = readBuffer_[i];
            if (c == '\n')
            {
                // std::cout << "Line reveived: " << lineBuffer_ << std::endl;
                handleLine_(lineBuffer_);
                lineBuffer_.clear();
            }
            else if (c != '\r')
            {
                lineBuffer_.push_back(c);
            }
        }
        startAsyncRead_(); // continue reading
    }
    else if (ec != asio::error::operation_aborted)
    {
        std::cerr << "Serial read error: " << ec.message() << std::endl;
    }
}

void Communication::sendCommand(const std::string &command)
{
    std::string cmd = command;

    while (!cmd.empty() && (cmd.back() == '\n' || cmd.back() == '\r'))
        cmd.pop_back();
    cmd += '\n';

    std::lock_guard<std::mutex> lock(writeMutex_);
    bool idle = writeQueue_.empty();
    writeQueue_.push(std::move(cmd));
    // std::cout << "sendCommand recieved: " << command << "\n";

    if (idle && !writeInProgress_)
    {
        startNextWrite_();
    }
}

void Communication::startNextWrite_()
{
    if (writeQueue_.empty() || !serial_.is_open())
    {
        // std::cout << "startNextWrite_: Queue empty or serial closed\n";
        writeInProgress_ = false;
        return;
    }

    writeInProgress_ = true;
    const std::string &nextCmd = writeQueue_.front();
    // std::cout << "Writing next cmd to controller: " << nextCmd;
    std::ofstream logFile("log.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << "Command send: " << nextCmd;
        logFile.close();
    }

    asio::async_write(serial_, asio::buffer(nextCmd),
                      [this](const asio::error_code &ec, std::size_t bytesWritten)
                      {
                          // std::cout << "Write callback triggered! ec=" << ec.message()
                          //         << " bytes=" << bytesWritten << "\n";

                          std::lock_guard<std::mutex> lock(writeMutex_);
                          // std::cout << "Write callback: acquired lock\n";

                          if (!ec)
                          {
                              // std::cout << "Write successful, popping queue\n";
                              writeQueue_.pop();
                              if (!writeQueue_.empty())
                              {
                                  // std::cout << "Queue not empty, writing next\n";
                                  startNextWrite_();
                              }
                              else
                              {
                                  // std::cout << "Queue empty, setting writeInProgress_ = false\n";
                                  writeInProgress_ = false;
                              }
                          }
                          else
                          {
                              std::cerr << "Serial write error: " << ec.message() << std::endl;
                              writeInProgress_ = false;
                          }
                      });
    // std::cout << "async_write initiated\n";
}

void Communication::closeCommunication()
{
    if (stop_.exchange(true))
        return; // already stopping

    asio::error_code ec;
    serial_.cancel(ec);
    serial_.close(ec);
    io_.stop();

    if (ioThread_.joinable())
        ioThread_.join();

    std::cout << "Communication closed." << std::endl;
}

void Communication::handleLine_(const std::string &line)
{
    if (line.find('=') == std::string::npos)
        return;

    std::string axisLabel;
    std::string reading = line;
    auto colonPos = line.find(':');

    if (colonPos != std::string::npos)
    {
        axisLabel = line.substr(0, colonPos);
        reading = line.substr(colonPos + 1);
    }

    if (!controller_)
        return;

    Axis *axis = nullptr;

    if (!axisLabel.empty())
    {
        // Axis specified - get that specific axis
        axis = controller_->getAxis(axisLabel[0]);
    }
    else
    {
        // No axis specified - use default (first axis)
        auto &allAxes = controller_->getAllAxis();
        if (!allAxes.empty())
        {
            axis = allAxes[0].get();
        }
    }

    // Only process if we have a valid axis
    if (axis)
    {
        axis->receiveData(reading);
    }
    // If axis is nullptr (not found), do nothing
}
