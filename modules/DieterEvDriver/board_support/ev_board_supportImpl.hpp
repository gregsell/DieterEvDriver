// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#ifndef BOARD_SUPPORT_EV_BOARD_SUPPORT_IMPL_HPP
#define BOARD_SUPPORT_EV_BOARD_SUPPORT_IMPL_HPP

//
// AUTO GENERATED - MARKED REGIONS WILL BE KEPT
// template version 3
//

#include <cstdint>
#include <generated/interfaces/ev_board_support/Implementation.hpp>

#include "../DieterEvDriver.hpp"
#include "generated/types/ev_board_support.hpp"

// ev@75ac1216-19eb-4182-a85c-820f1fc2c091:v1
// insert your custom include headers here
#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <string>
// ev@75ac1216-19eb-4182-a85c-820f1fc2c091:v1

namespace module {
namespace board_support {

struct Conf {};

class ev_board_supportImpl : public ev_board_supportImplBase {
public:
    ev_board_supportImpl() = delete;
    ev_board_supportImpl(Everest::ModuleAdapter* ev, const Everest::PtrContainer<DieterEvDriver>& mod, Conf& config) :
        ev_board_supportImplBase(ev, "board_support"), mod(mod), config(config){};

    // ev@8ea32d28-373f-4c90-ae5e-b4fcc74e2a61:v1
    // insert your public definitions here
    virtual ~ev_board_supportImpl();// not sure if needed.

    // ev@8ea32d28-373f-4c90-ae5e-b4fcc74e2a61:v1

protected:
    // command handler functions (virtual)
    virtual void handle_enable(bool& value) override;
    virtual void handle_set_cp_state(types::ev_board_support::EvCpState& cp_state) override;
    virtual void handle_allow_power_on(bool& value) override;
    virtual void handle_diode_fail(bool& value) override;
    virtual void handle_set_ac_max_current(double& current) override;
    virtual void handle_set_three_phases(bool& three_phases) override;
    virtual void handle_set_rcd_error(double& rcd_current_mA) override;

    // ev@d2d1847a-7b88-41dd-ad07-92785f06f5c4:v1
    // insert your protected definitions here
    // ev@d2d1847a-7b88-41dd-ad07-92785f06f5c4:v1

private:
    const Everest::PtrContainer<DieterEvDriver>& mod;
    const Conf& config;

    virtual void init() override;
    virtual void ready() override;

    // ev@3370e4dd-95f4-47a9-aaec-ea76f34a66c9:v1
    // state var; these are needed as everest is driven by _change_ of events.
    // The last instance in the chain, the MCU, is supposed to keep track of states.
    // as this is not the case (for Dieter) we do that here.
    bool allow_power_on_{false};
    types::ev_board_support::EvCpState cp_state_{types::ev_board_support::EvCpState::A};
    int8_t outvalue{0}; // commands for Dieter
    std::string outvalue_prefix{"do000"}; // a fourth digit (outvalue) plus newline will be added

    std::atomic<bool> running{false};
    std::atomic<bool> serial_port_ready{false};
    boost::asio::io_context io_ctx_;
    std::unique_ptr<boost::asio::serial_port> serial_port_;
    std::thread serial_thread_;

    void serial_reader_thread();
    void on_serial_line(const std::string& raw);
    void map_name_to_event(const std::string& name);
    void write_to_serial();
    void update_power_state();
    // ev@3370e4dd-95f4-47a9-aaec-ea76f34a66c9:v1
};

// ev@3d7da0ad-02c2-493d-9920-0bbbd56b9876:v1
// insert other definitions here
// ev@3d7da0ad-02c2-493d-9920-0bbbd56b9876:v1

} // namespace board_support
} // namespace module

#endif // BOARD_SUPPORT_EV_BOARD_SUPPORT_IMPL_HPP
