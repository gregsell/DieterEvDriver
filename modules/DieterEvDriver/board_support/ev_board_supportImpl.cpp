// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "ev_board_supportImpl.hpp"
#include <everest/logging.hpp>

namespace module {
namespace board_support {

void ev_board_supportImpl::init() {
    EVLOG_info << "init PEVSIM 2xxxxx";
}

void ev_board_supportImpl::ready() {
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

} // namespace board_support
} // namespace module
