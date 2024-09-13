pkgname=ergo
pkgver=0.0.1
pkgrel=1
pkgdesc="A minimal status bar for Wayland status bar"
arch=(x86_64)
url=https://github.com/d4yr41n/ergo
license=(MIT)
depends=(cairo pango wlroots)
makedepends=(make wayland-protocols)
source=("https://github.com/d4yr41n/ergo/releases/download/0.0.1/ergo-0.0.1.tar.gz")
sha256sums=("6aa9f317ffc2f19932d5b2f9beb1437630e7681bbecae8fa539dcc7e2224062e")

build() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	make
}

package() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	make PREFIX="${pkgdir}/usr/" install
	install -Dm645 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
