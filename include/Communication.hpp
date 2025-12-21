#pragma once

#include <asio.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <optional>
#include <functional>

namespace Xeryon
{
    class Axis;
    class Controller;
    /**
     * @brief Handles serial communication with the Xeryon controller device
     *
     * This class encapsulates standalone Asio serial-port handling, async reads,
     * and a write queue so other parts of the library can send commands
     * in a thread-safe manner. It forwards incoming lines to the owning
     * `Controller` for processing.
     */
    class Communication
    {
    public:
        /**
         * @brief Construct a Communication object
         * @param controller Pointer to the owning `Controller` instance
         * @param COMPort Serial device path (e.g. "/dev/ttyACM0")
         * @param baud Baud rate for the serial port
         */
        Communication(Controller *controller, const std::string &COMPort, int baud);

        /**
         * @brief Clean up resources and stop I/O thread
         */
        ~Communication();

        /**
         * @brief Start the communication thread and begin async I/O
         */
        void start();

        /**
         * @brief Queue a command to be sent to the controller
         * @param command The textual command to send (without trailing newline)
         */
        void sendCommand(const std::string &command);

        /**
         * @brief Close the serial port and stop the I/O thread
         */
        void closeCommunication();

    private:
        /**
         * @brief Configure serial port options (baud, parity, timeouts)
         */
        void configureSerialPort_();

        /**
         * @brief Start an asynchronous read operation into the internal buffer
         */
        void startAsyncRead_();

        /**
         * @brief Async read completion callback
         * @param ec Asio error code
         * @param bytesTransferred Number of bytes received
         */
        void handleRead_(const asio::error_code &ec, std::size_t bytesTransferred);

        /**
         * @brief Start writing the next queued message (internal)
         */
        void startNextWrite_();

        /**
         * @brief Handle a complete line received from the controller
         * @param line The received textual line
         */
        void handleLine_(const std::string &line);

        Controller *controller_;
        std::string COMPort_;
        int baud_;

        asio::io_context io_;
        asio::serial_port serial_;
        std::thread ioThread_;

        std::mutex writeMutex_;
        std::queue<std::string> writeQueue_;
        bool writeInProgress_ = false;

        std::array<char, 512> readBuffer_{};
        std::string lineBuffer_;

        std::atomic<bool> stop_{false};
    };

};
