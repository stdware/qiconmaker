#include "IconSource.h"

#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

namespace qiconmaker {

    std::optional<IconSource> IconSource::read(const QString &file, QString &error) {
        IconSource source;
        source.m_file = file;

        const auto suffix = QFileInfo(file).suffix().toLower();
        if (suffix == QLatin1String("svg") || suffix == QLatin1String("svgz")) {
            auto renderer = std::make_shared<QSvgRenderer>(file);
            if (!renderer->isValid()) {
                error = QStringLiteral("%1 is no valid SVG drawing").arg(file);
                return std::nullopt;
            }
            // The view box, or the size of the drawing if it has none
            const auto box = renderer->viewBoxF();
            if (box.isEmpty() || !qFuzzyCompare(box.width(), box.height())) {
                error = QStringLiteral("%1 is not square").arg(file);
                return std::nullopt;
            }
            source.m_renderer = std::move(renderer);
            return source;
        }

        QImageReader reader(file);
        const auto image = reader.read();
        if (image.isNull()) {
            error = QStringLiteral("%1 cannot be read: %2").arg(file, reader.errorString());
            return std::nullopt;
        }
        if (image.width() != image.height()) {
            error = QStringLiteral("%1 is not square").arg(file);
            return std::nullopt;
        }
        source.m_image = image.convertToFormat(QImage::Format_ARGB32);
        return source;
    }

    QString IconSource::file() const {
        return m_file;
    }

    bool IconSource::isVector() const {
        return m_renderer != nullptr;
    }

    int IconSource::pixelSize() const {
        return m_renderer ? 0 : m_image.width();
    }

    std::optional<QImage> IconSource::image(int size) const {
        if (m_renderer) {
            QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            m_renderer->render(&painter, QRectF(0, 0, size, size));
            painter.end();
            return image.convertToFormat(QImage::Format_ARGB32);
        }
        if (m_image.width() < size) {
            return std::nullopt;
        }
        return m_image.width() == size ? m_image : reduced(m_image, size);
    }

    QImage IconSource::reduced(const QImage &image, int size) {
        // QImage::scaled() converts an image with alpha to premultiplied ARGB32 before scaling,
        // so that a transparent pixel adds nothing of its color to its neighbors.
        return image.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
            .convertToFormat(QImage::Format_ARGB32);
    }

}
