SUMMARY = "PEV-SIM WLAN AP network configuration"
DESCRIPTION = "Static wlan0 network config for the PEV-SIM WiFi hotspot (AP mode)."
LICENSE = "CLOSED"

SRC_URI = "file://wlan0-ap.network"

S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/wlan0-ap.network ${D}${sysconfdir}/systemd/network/wlan0-ap.network
}

FILES:${PN} += "${sysconfdir}/systemd/network/wlan0-ap.network"

RDEPENDS:${PN} = "hostapd dnsmasq wpa-supplicant iw"
