#include "IconSet.h"

namespace qiconmaker {

    IconSet::IconSet(IconSource master) : m_master(std::move(master)) {
    }

    void IconSet::setSource(int size, IconSource source) {
        m_sources.insert_or_assign(size, std::move(source));
    }

    std::optional<QList<QImage>> IconSet::images(const QList<int> &sizes, QString &error) const {
        QList<QImage> images;
        images.reserve(sizes.size());
        for (const int size : sizes) {
            const auto it = m_sources.find(size);
            if (it != m_sources.end()) {
                const auto &source = it->second;
                // A source of a size is drawn for that size, so a PNG of another size is a
                // mistake rather than an image to scale.
                if (!source.isVector() && source.pixelSize() != size) {
                    error = QStringLiteral("%1 is %2 pixels, not %3")
                                .arg(source.file())
                                .arg(source.pixelSize())
                                .arg(size);
                    return std::nullopt;
                }
                images.push_back(*source.image(size));
                continue;
            }
            const auto image = m_master.image(size);
            if (!image) {
                error = QStringLiteral("%1 is %2 pixels, smaller than %3")
                            .arg(m_master.file())
                            .arg(m_master.pixelSize())
                            .arg(size);
                return std::nullopt;
            }
            images.push_back(*image);
        }
        return images;
    }

}
