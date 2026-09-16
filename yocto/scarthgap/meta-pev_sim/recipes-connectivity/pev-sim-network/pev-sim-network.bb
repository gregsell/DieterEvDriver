SUMMARY = "PEV-SIM WLAN AP network configuration"
DESCRIPTION = "Static wlan0 network config for the PEV-SIM WiFi hotspot (AP mode)."
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "file://wlan0-ap.network"

S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/wlan0-ap.network ${D}${sysconfdir}/systemd/network/wlan0-ap.network
}

FILES:${PN} += "${sysconfdir}/systemd/network/wlan0-ap.network"

RDEPENDS:${PN} = "hostapd dnsmasq wpa-supplicant iw"
