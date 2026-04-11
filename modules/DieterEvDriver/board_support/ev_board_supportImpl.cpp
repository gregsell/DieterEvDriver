// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "ev_board_supportImpl.hpp"
#include "DieterEvDriver.hpp"
#include "everest/logging.hpp"
#include "generated/types/ev_board_support.hpp"
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

void ev_board_supportImpl::init() {
    running = true;
    serial_thread_ = std::thread(&ev_board_supportImpl::serial_reader_thread, this); 
    EVLOG_info << "init";
}

void ev_board_supportImpl::ready() {
    EVLOG_info << "ready";
    // DIRTY FIX: no pwm measurement in hardware, 
    // thus we for now assume DC operation with HLC and thus set 5% dutycycle
    // not the best solution, but it worked for uhi22 and pyPLC:  https://github.com/uhi22/pyPLC/blob/079cd684e23f26b047687fb81fa2a27d1b097044/pyPlcHomeplug.py#L975
    types::board_support_common::BspMeasurement bspm;
    bspm.cp_pwm_duty_cycle = 5.;
    // This is not implemented on MCU side yet
    bspm.proximity_pilot = {types::board_support_common::Ampacity::None};
    publish_bsp_measurement(bspm);
}

void ev_board_supportImpl::handle_enable(bool& value) {
    // your code for cmd enable goes here
    EVLOG_info << "handle_enable: " << value;
}

void ev_board_supportImpl::handle_set_cp_state(types::ev_board_support::EvCpState& cp_state) {
    //cp_state_= cp_state;
    EVLOG_info << "new c_p state: " << cp_state;
    
    if (cp_state == types::ev_board_support::EvCpState::B) {
        outvalue &= ~1; // set c_p state bit ->B
        if (cp_state_== types::ev_board_support::EvCpState::A) {
            outvalue |= 4; // enable connector lock iff transition A->B
        }
        else if (cp_state_== types::ev_board_support::EvCpState::C) {
            outvalue &= ~4; // disable connector lock iff transition C->B
        }
    } 
    else if (cp_state == types::ev_board_support::EvCpState::C) {
        outvalue |= 1; // set c_p state bit ->C
    }
    write_to_serial();
    update_power_state(); // rethink
}

void ev_board_supportImpl::handle_allow_power_on(bool& value) {
    allow_power_on_ = value;
    if (!allow_power_on_) {
        // send msg to disable relays
        outvalue &= 2;
        write_to_serial();
    }
    update_power_state();
}

void ev_board_supportImpl::update_power_state() {
    types::board_support_common::BspEvent bspe;
    if (allow_power_on_ && (cp_state_ == types::ev_board_support::EvCpState::C
                        ||  cp_state_ == types::ev_board_support::EvCpState::D)) {
        bspe.event = types::board_support_common::Event::PowerOn;
    } else {
        bspe.event = types::board_support_common::Event::PowerOff;
    }
    publish_bsp_event(bspe);
}

void ev_board_supportImpl::write_to_serial() {
    std::string msg = outvalue_prefix + std::to_string(outvalue) + "\n";
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

void ev_board_supportImpl::on_serial_line(const std::string& raw) {
    const std::string line = trim(raw);
    if (line.empty()) return;
    EVLOG_info << "received raw:  " << raw;  
    /*
    // turns out everest does not need vehicle-side high voltage measurements
    if (starts_with(line, "A0=")) {
        try {
            int A0_raw = std::stoi(line.substr(3));
            int inlet_voltage = A0_raw / 1024.0 * 1.08 * (6250) / (4.7+4.7); // change later according to exact resistor values
            // possible to verify with A1 reading
            EVLOG_info << "     inlet voltage " << inlet_voltage;
        } catch (...) {
            EVLOG_error << "could not parse inlet voltage line";
        }
    }
        */
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