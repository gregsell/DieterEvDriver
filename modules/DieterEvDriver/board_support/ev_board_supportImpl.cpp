// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "ev_board_supportImpl.hpp"
#include <everest/logging.hpp>

namespace module {
namespace board_support {

void ev_board_supportImpl::init() {
    running = true;
    serial_thread_ = std::thread(&ev_board_supportImpl::serial_reader_thread, this);
    EVLOG_info << "config serial port:  " << mod->config.serial_port;
    EVLOG_info << "config baud rate     " << mod->config.baud_rate;  
}

void ev_board_supportImpl::ready() {
        // possible GPIO reset
    EVLOG_info << "ready";
}

void ev_board_supportImpl::handle_enable(bool& value) {
    // your code for cmd enable goes here
    if(true);
}

void ev_board_supportImpl::handle_set_cp_state(types::ev_board_support::EvCpState& cp_state) {
    // your code for cmd set_cp_state goes here
}

void ev_board_supportImpl::handle_allow_power_on(bool& value) {
    // your code for cmd allow_power_on goes here
}

void ev_board_supportImpl::handle_diode_fail(bool& value) {
    // your code for cmd diode_fail goes here
}

void ev_board_supportImpl::handle_set_ac_max_current(double& current) {
    // your code for cmd set_ac_max_current goes here
}

void ev_board_supportImpl::handle_set_three_phases(bool& three_phases) {
    // your code for cmd set_three_phases goes here
}

void ev_board_supportImpl::handle_set_rcd_error(double& rcd_current_mA) {
    // your code for cmd set_rcd_error goes here
}

void ev_board_supportImpl::serial_reader_thread() {
    while (running) { // outer while-loop implements auto-reconnect
        try {
             EVLOG_info << "Attempting to open serial port " << mod->config.serial_port << " at " << mod->config.baud_rate << " baud...";
            serial_port_ = std::make_unique<boost::asio::serial_port>(io_ctx_);
            serial_port_->open(mod->config.serial_port);
            serial_port_->set_option(boost::asio::serial_port_base::baud_rate(mod->config.baud_rate));
            serial_port_->set_option(boost::asio::serial_port_base::character_size(8));
            serial_port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
            serial_port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
            serial_port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

            EVLOG_info << "Serial port successfully opened.";
            serial_port_ready = true;

            std::string rx_buffer;
            std::vector<char> read_buf(256);

            while (running) {
                boost::system::error_code ec;
                size_t n = serial_port_->read_some(boost::asio::buffer(read_buf), ec);
                if (ec) {
                    EVLOG_error << "Serial read error: " << ec.message();
                    break;
                }

                if (n > 0) {
                    rx_buffer.append(read_buf.data(), n);
                    size_t pos;
                    while ((pos = rx_buffer.find('\n')) != std::string::npos) {
                        on_serial_line(rx_buffer.substr(0, pos));
                        rx_buffer.erase(0, pos + 1);
                    }
                }
            }
        } catch (const std::exception& e) {
            EVLOG_error << "Serial port error: " << e.what();
        }
        // in case of error the connection aborts, port is closed, short wait and reconnect
        serial_port_ready = false;
        if (serial_port_ && serial_port_->is_open()) {
            boost::system::error_code ignore_ec;
            serial_port_->close(ignore_ec); // clangd warning regarding unused return object wrong
        }

        if (running) std::this_thread::sleep_for(std::chrono::seconds(5)); // wait after serial comm. error
    }

    EVLOG_info << "Serial reader thread finished.";
}

void ev_board_supportImpl::on_serial_line(const std::string& raw) {

}

} // namespace board_support
} // namespace module
