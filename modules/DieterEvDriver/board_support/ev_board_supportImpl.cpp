// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "ev_board_supportImpl.hpp"
#include "DieterEvDriver.hpp"
#include "everest/logging.hpp"
#include "generated/types/board_support_common.hpp"
#include "generated/types/ev_board_support.hpp"
#include <cstring>
#include <string>

// helper namespace
namespace {

// Helper: check if string starts with prefix
bool starts_with(const std::string& s, const char* pfx) {
    return s.rfind(pfx, 0) == 0;
}
// Helper: trim whitespace
std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

}

namespace module {
namespace board_support {

void ev_board_supportImpl::write_to_serial(const std::string& msg) {
    if ((msg.back() != '\n') && (msg.find(':') != std::string::npos)) { // check if colon exists and is newline -terminated
        EVLOG_error << "invalid message synax: " << msg;
        return;
    }
    if (!serial_port_ready) {
        EVLOG_info << "Ignoring write, serial port not ready: " << msg;
        return;
    }
    if (serial_port_ && serial_port_->is_open()) {
        EVLOG_info << "TX: " << msg;
        boost::system::error_code ec;
        boost::asio::write(*serial_port_, boost::asio::buffer(msg), ec);
        if (ec) {
            EVLOG_error << "Serial write failed: " << ec.message();
        }
    } else {
        EVLOG_error << "TX FAILED, port is unexpectedly closed for msg: " << msg;
    }
}

void ev_board_supportImpl::update_power_state() {
    types::board_support_common::BspEvent bspe;
    if (allow_power_on_ && connector_lock_confirmed && (cp_state_ == types::ev_board_support::EvCpState::C
                                                    ||  cp_state_ == types::ev_board_support::EvCpState::D)) {
        bspe.event = types::board_support_common::Event::PowerOn;
        write_to_serial("set_contactor:1");
    } else {
        bspe.event = types::board_support_common::Event::PowerOff;
        write_to_serial("set_contactor:0");
    }
    publish_bsp_event(bspe);
}

void ev_board_supportImpl::on_serial_line(const std::string& raw) {
    const std::string line = trim(raw);
    if (line.empty()) return;
    EVLOG_info << "received raw:  " << raw;  

    const int pos_separator = line.find(':');
    // extract key
    std::string key = line.substr(0, pos_separator);
    std::string value = line.substr(pos_separator+1, line.length());
    EVLOG_info << "found key: " << key << " and value: " << value;

    try{
        if (key.compare("cp_duty_cycle") == 0) {
            if (std::stoi(value) != cp_duty_cycle_) {
                cp_duty_cycle_ = std::clamp(std::stoi(value), 0, 100);   // update state var
                types::board_support_common::BspMeasurement bspm;                        // publish respective Events
                bspm.cp_pwm_duty_cycle = cp_duty_cycle_;
                bspm.proximity_pilot = {types::board_support_common::Ampacity::None};// This is not implemented on MCU side
                publish_bsp_measurement(bspm);   
            }
        }
        else if (key.compare("connector_lock_confirmed") == 0) {
            connector_lock_confirmed = (std::stoi(value) != 0);                     // update state var        
        }
    } catch (...) {
        EVLOG_error << "could not parse line: " << line;
    }
}   

void ev_board_supportImpl::serial_reader_thread() {
    while (running) { // outer while-loop implements auto-reconnect
    int serial_fail_count = 0;
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
        // in serial_reader_thread(), nach dem catch-Block:
        if (++serial_fail_count > 5) {
            mod->p_board_support->raise_error(
                mod->p_board_support->error_factory->create_error(
                    "generic/CommunicationFault", "", "Serial port repeatedly unavailable"));
        }
        if (running) std::this_thread::sleep_for(std::chrono::seconds(3)); // wait after serial comm. error
    }

    EVLOG_info << "Serial reader thread finished.";
} 

void ev_board_supportImpl::publish_all_var() {  // publish all VAR as defined here: https://everest.github.io/nightly/reference/interfaces/ev_board_support.html#ev-board-support
    types::board_support_common::BspMeasurement bspm;          
    bspm.cp_pwm_duty_cycle = cp_duty_cycle_;
    bspm.proximity_pilot = {types::board_support_common::Ampacity::None};// This is not implemented on MCU side
    publish_bsp_measurement(bspm);  

    types::board_support_common::BspEvent bspe;
    bspe.event = types::board_support_common::string_to_event(types::ev_board_support::ev_cp_state_to_string(cp_state_));
    publish_bsp_event(bspe);

    // add VAR: ev_info [optional]

    EVLOG_info << "published bsp_measurement: " << bspm;
    EVLOG_info << "published bsp_event: " << bspe;
}

void ev_board_supportImpl::init() {
    running = true;
    serial_thread_ = std::thread(&ev_board_supportImpl::serial_reader_thread, this); 
    EVLOG_info << "init";
}

void ev_board_supportImpl::ready() {
    EVLOG_info << "ready";
    // publish_all_var(); // is this needed here?
}

void ev_board_supportImpl::handle_enable(bool& value) {
    EVLOG_info << "handle_enable: " << value;
    // std::this_thread::sleep_for(std::chrono::milliseconds(500)); // optional delay to wait for values from MCU
    publish_all_var();
}

void ev_board_supportImpl::handle_set_cp_state(types::ev_board_support::EvCpState& cp_state) {
    //if (cp_state == cp_state_) return; // gute oder schlechte Idee?
    EVLOG_info << "change c_p state from " << cp_state_ << " to " << cp_state;

    if (cp_state == types::ev_board_support::EvCpState::B ||
        cp_state == types::ev_board_support::EvCpState::C ||
        cp_state == types::ev_board_support::EvCpState::D) {
        // lock connector
        if (!connector_lock_confirmed) {    // check if connector is locked already, as it could damage the motor
            write_to_serial("set_connector_lock:1");
            std::this_thread::sleep_for(std::chrono::seconds(1));   // add delay and wait for feedback
            if (!connector_lock_confirmed) {
                EVLOG_error << "connector lock not closing";
                //const std::string oldEvent = types::ev_board_support::ev_cp_state_to_string(cp_state_);
                //publish_bsp_event(types::board_support_common::BspEvent:: string_to_event(oldEvent)); // publish previous state
                return;
            }
        }
        if (cp_state == types::ev_board_support::EvCpState::C ||
            cp_state == types::ev_board_support::EvCpState::D) {
            write_to_serial("set_state_c:1");                   // enable mosfet, to pull Cp down
            // note that the contactors are not enabled here, as this is also dependent on allow_power_on
            // this is checked by the update_power_state method called at the end of this scope
        }
        else  {
            write_to_serial("set_state_c:0");
        }

     } else { //if (cp_state == types::ev_board_support::EvCpState::E) {
        // short of Cp to PE (connection lost)
        // unlock connector instantly
        write_to_serial("set_state_c:0");
        if (connector_lock_confirmed) { // check if connector is unlocked already, as it could damage the motor
            write_to_serial("set_connector_lock:0");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (connector_lock_confirmed) {
                EVLOG_error << "connector lock not opening";
            }
        }
     }
    
    cp_state_ = cp_state;
    update_power_state();
}

void ev_board_supportImpl::handle_allow_power_on(bool& value) {
    allow_power_on_ = value;
    if (!allow_power_on_) {
        // send msg to disable relays
        write_to_serial("set_contactor:0");
    }
    update_power_state();
}

void ev_board_supportImpl::handle_diode_fail(bool& value) {
    EVLOG_info << "handle_diode_fail: " << value;
}

void ev_board_supportImpl::handle_set_ac_max_current(double& current) {
    EVLOG_info << "handle_set_ac_max_current: " << current;
}

void ev_board_supportImpl::handle_set_three_phases(bool& three_phases) {
    EVLOG_info << "handle_set_three_phases: " << three_phases;
}

void ev_board_supportImpl::handle_set_rcd_error(double& rcd_current_mA) {
    EVLOG_info << "handle_set_rcd_error: " << rcd_current_mA;
}

ev_board_supportImpl::~ev_board_supportImpl() {
    running = false;

    if (serial_port_ && serial_port_->is_open()) {
        boost::system::error_code ignore_ec;
        serial_port_->cancel(ignore_ec);
        serial_port_->close(ignore_ec);
    }
    if (serial_thread_.joinable()) serial_thread_.join();

    EVLOG_info << "All threads stopped cleanly.";
}

} // namespace board_support
} // namespace module