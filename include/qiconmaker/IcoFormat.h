#ifndef QICONMAKER_ICOFORMAT_H
#define QICONMAKER_ICOFORMAT_H

#include <optional>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QImage>

namespace qiconmaker {

    /// The icon format of Windows, ICO.
    class IcoFormat {
    public:
        /// The largest width and height of an image of an ICO
        static constexpr int maximumSize = 256;

        /// Returns the sizes of an icon of an application by default: 16, 20, 24, 30, 32, 36,
        /// 40, 48, 60, 64, 72, 80, 96 and 256, the union of the sizes that the title bar, the
        /// taskbar and the Start menu use at each scale, as the page on the construction of app
        /// icons of the documentation of Windows lists them.
        static QList<int> defaultSizes();

        /// Encodes \a images, in the order given. An image of 256 pixels is stored as PNG, and
        /// every other image as a 32-bit BGRA bitmap with an AND mask, which every reader of
        /// icons of Windows accepts.
        /// \return the file, or std::nullopt with \a error set if an image is not square or is
        /// larger than maximumSize.
        static std::optional<QByteArray> encode(const QList<QImage> &images, QString &error);
    };

}

#endif // QICONMAKER_ICOFORMAT_H
