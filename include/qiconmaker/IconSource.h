#ifndef QICONMAKER_ICONSOURCE_H
#define QICONMAKER_ICONSOURCE_H

#include <memory>
#include <optional>

#include <QtCore/QString>
#include <QtGui/QImage>

class QSvgRenderer;

namespace qiconmaker {

    /// A square drawing from which the images of an icon are made: a vector drawing in SVG,
    /// rendered at any size, or an image in PNG, reduced to a smaller size.
    class IconSource {
    public:
        /// Reads \a file, an SVG if its suffix is \c .svg or \c .svgz and a PNG otherwise.
        /// \return the source, or std::nullopt with \a error set if the file cannot be read or
        /// the drawing is not square.
        static std::optional<IconSource> read(const QString &file, QString &error);

        /// The file that the source was read from
        QString file() const;

        /// Returns whether the source is a vector drawing.
        bool isVector() const;

        /// The width and height of a PNG in pixels, or 0 for an SVG
        int pixelSize() const;

        /// Returns the image at \a size pixels square, in the format ARGB32. An SVG is rendered at
        /// the size. A PNG of the size is returned as it is, and a larger PNG is reduced.
        /// \return the image, or std::nullopt if the source is a PNG smaller than \a size.
        std::optional<QImage> image(int size) const;

        /// Returns \a image reduced to \a size pixels square with the smooth transformation of
        /// Qt, which averages the pixels that each pixel of the result covers.
        static QImage reduced(const QImage &image, int size);

    private:
        IconSource() = default;

        QString m_file;
        std::shared_ptr<QSvgRenderer> m_renderer;
        QImage m_image;
    };

}

#endif // QICONMAKER_ICONSOURCE_H
