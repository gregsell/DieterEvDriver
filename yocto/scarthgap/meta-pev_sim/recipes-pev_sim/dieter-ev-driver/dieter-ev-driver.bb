# recipes-pev_sim/DieterEvDriver/DieterEvDriver.bb
SUMMARY = "DieterEvDriver EVerest board support module"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "git://github.com/gregsell/DieterEvDriver.git;protocol=https;branch=main"
# commit hash; 7.8.2026
SRCREV = "4ad68ea41b617b8fc29781b7b32df780f567e0f3"

S = "${WORKDIR}/git"

inherit cmake pkgconfig python3native

DEPENDS = " \
    everest-core \
    evcli-native \
    sigslot \
"

INSANE_SKIP:${PN} = "already-stripped useless-rpaths arch file-rdeps"


FILES:${PN} += "${libexecdir}/everest/modules/DieterEvDriver/*"

do_install:append() {
    rm -f ${D}${datadir}/everest/version_information.txt
    rmdir --ignore-fail-on-non-empty ${D}${datadir}/everest 2>/dev/null || true
    rmdir --ignore-fail-on-non-empty ${D}${datadir} 2>/dev/null || true
}

EXTRA_OECMAKE += " \
    -DDISABLE_EDM=ON \
    -DBUILD_TESTING=OFF \
    -DEVEREST_ENABLE_RUN_SCRIPT_GENERATION=OFF \
    -DPEV-SIM_USE_PYTHON_VENV=OFF \
"
