EAPI=5

inherit kde4-base

EGIT_REPO_URI="https://github.com/sanya-m/kde-touchpad-config.git"

SLOT="0"
KEYWORDS="amd64 x86"

RDEPEND="
    x11-libs/libX11
    x11-libs/libXi
    x11-libs/libxcb
    x11-drivers/xf86-input-synaptics
"

DEPEND="
    ${RDEPEND}
    x11-proto/inputproto
"
