pkgname=ergo
pkgver=0.0.2
pkgrel=1
pkgdesc="A minimal status bar for Wayland compositors"
arch=(x86_64)
url=https://github.com/d4yr41n/ergo
license=(MIT)
depends=(cairo pango wlroots)
makedepends=(make wayland-protocols)
source=("https://github.com/d4yr41n/ergo/releases/download/0.0.2/ergo-0.0.2.tar.gz")
sha256sums=("df9035e0beb740f38f2a494579d4cc916b1a5587ad21afc58dee99ae194184f7")

build() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	make
}

package() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	make PREFIX="${pkgdir}/usr/" install
	install -Dm645 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
