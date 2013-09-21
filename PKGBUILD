_pkgname=kde-touchpad-config
pkgname=${_pkgname}-git
pkgver=106.a9fb8b3
pkgrel=1
pkgdesc="A KCModule for configuring the touchpad."
arch=('i686' 'x86_64')
url="https://github.com/sanya-m/kde-touchpad-config"
license=('GPL')
depends=('kdelibs' 'xf86-input-synaptics' 'libxcb' 'libx11' 'libxi')
makedepends=('cmake' 'automoc4' 'git')
provides=("${_pkgname}")
conflicts=("${_pkgname}")
source=('git+https://github.com/sanya-m/kde-touchpad-config.git')
md5sums=('SKIP')

pkgver() {
  cd "${_pkgname}"
  printf "%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
  mkdir -p "${_pkgname}/build"
}

build() {
  cd "${_pkgname}/build"
  cmake .. -DCMAKE_BUILD_TYPE=DebugFull -DCMAKE_INSTALL_PREFIX="$(kde4-config --prefix)"
  make
}

package() {
  cd "${_pkgname}/build"
  make DESTDIR="${pkgdir}" install
}
