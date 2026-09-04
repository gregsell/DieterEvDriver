FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://hostapd.conf"

do_install:append() {
    install -m 0600 ${WORKDIR}/hostapd.conf ${D}${sysconfdir}/hostapd.conf
}

FILES:${PN} += "${sysconfdir}/hostapd.conf"

SYSTEMD_AUTO_ENABLE:${PN} = "enable"
