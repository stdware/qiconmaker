#ifndef QICONMAKER_ICNSFORMAT_H
#define QICONMAKER_ICNSFORMAT_H

#include <optional>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QImage>

namespace qiconmaker {

    /// The icon format of macOS, ICNS, with each image stored as PNG, which macOS 10.7 and later
    /// reads.
    class IcnsFormat {
    public:
        /// Returns the sizes in pixels that an ICNS holds: 16, 32, 64, 128, 256, 512 and 1024,
        /// which are the sizes of an icon by default as well.
        static QList<int> sizes();

        /// Encodes \a images. Each image is stored under every element type of its size, so that
        /// the image of 32 pixels serves 32 points at 1x and 16 points at 2x, as
        /// <tt>iconutil</tt> stores an iconset.
        /// \return the file, or std::nullopt with \a error set if an image is not square or has
        /// a size that sizes() lacks.
        static std::optional<QByteArray> encode(const QList<QImage> &images, QString &error);
    };

}

#endif // QICONMAKER_ICNSFORMAT_H
