EAPI=5

inherit kde4-base

EGIT_REPO_URI="https://github.com/sanya-m/kde-touchpad-config.git"

SLOT="0"
KEYWORDS="amd64 x86"

DEPEND="
    x11-libs/libXi
    x11-libs/libxcb
"

RDEPEND="
    ${DEPEND}
    x11-proto/inputproto
"
