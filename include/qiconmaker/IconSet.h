#ifndef QICONMAKER_ICONSET_H
#define QICONMAKER_ICONSET_H

#include <map>
#include <optional>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QImage>

#include <qiconmaker/IconSource.h>

namespace qiconmaker {

    /// The sources of the images of an icon: a master, and a source for each of some sizes, such
    /// as a drawing aligned to the pixel grid of a small size.
    class IconSet {
    public:
        explicit IconSet(IconSource master);

        /// Sets \a source as the source of the image at \a size, in place of the master.
        void setSource(int size, IconSource source);

        /// Returns the images at \a sizes, in the order of \a sizes. A size with a source of its
        /// own takes the image of that source, which provides the size exactly: an SVG is
        /// rendered, and a PNG has the size. Every other size takes the image of the master.
        /// \return the images, or std::nullopt with \a error set if a PNG of a size has another
        /// size, or if a PNG master is smaller than a size that takes it.
        std::optional<QList<QImage>> images(const QList<int> &sizes, QString &error) const;

    private:
        IconSource m_master;
        std::map<int, IconSource> m_sources;
    };

}

#endif // QICONMAKER_ICONSET_H
