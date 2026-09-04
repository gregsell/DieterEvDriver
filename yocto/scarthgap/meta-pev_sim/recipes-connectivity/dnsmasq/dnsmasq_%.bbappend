FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://dnsmasq-wlan0.conf \
    file://wait-for-hostapd.conf \
"

do_install:append() {
    install -d ${D}${sysconfdir}/dnsmasq.d
    install -m 0644 ${WORKDIR}/dnsmasq-wlan0.conf ${D}${sysconfdir}/dnsmasq.d/pev-sim-ap.conf

    install -d ${D}${systemd_system_unitdir}/dnsmasq.service.d
    install -m 0644 ${WORKDIR}/wait-for-hostapd.conf ${D}${systemd_system_unitdir}/dnsmasq.service.d/wait-for-hostapd.conf
}

FILES:${PN} += " \
    ${sysconfdir}/dnsmasq.d/pev-sim-ap.conf \
    ${systemd_system_unitdir}/dnsmasq.service.d/wait-for-hostapd.conf \
"

SYSTEMD_AUTO_ENABLE:${PN} = "enable"
