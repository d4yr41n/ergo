pkgname=ergo
pkgver=0.0.2
pkgrel=1
pkgdesc="A minimal status bar for Wayland compositors"
arch=(x86_64)
url=https://github.com/d4yr41n/ergo
license=(MIT)
depends=(git cairo pango wlroots)
makedepends=(make wayland-protocols)
source=("git+https://github.com/d4yr41n/ergo")
sha256sums=("SKIP")

build() {
	cd "${srcdir}/${pkgname}"
	make
}

package() {
	cd "${srcdir}/${pkgname}"
	make PREFIX="${pkgdir}/usr/" install
	install -Dm645 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
