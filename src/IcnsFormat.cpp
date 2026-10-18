#include "IcnsFormat.h"

#include <QtCore/QBuffer>
#include <QtCore/QMap>

namespace qiconmaker {

    namespace {

        struct Element {
            const char *type;
            int size;
        };

        // The element types of PNG data and their sizes in pixels, in the order in which
        // iconutil writes them. ic11, ic12, ic13, ic14 and ic10 hold the images at 2x of 16, 32,
        // 128, 256 and 512 points.
        const Element elements[] = {
            {"icp4", 16  },
            {"icp5", 32  },
            {"ic11", 32  },
            {"ic12", 64  },
            {"ic07", 128 },
            {"ic13", 256 },
            {"ic08", 256 },
            {"ic14", 512 },
            {"ic09", 512 },
            {"ic10", 1024},
        };

        void put32(QByteArray &out, quint32 value) {
            out.append(char((value >> 24) & 0xff));
            out.append(char((value >> 16) & 0xff));
            out.append(char((value >> 8) & 0xff));
            out.append(char(value & 0xff));
        }

        QByteArray pngOf(const QImage &image) {
            QByteArray out;
            QBuffer buffer(&out);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            return out;
        }

    }

    QList<int> IcnsFormat::sizes() {
        return {16, 32, 64, 128, 256, 512, 1024};
    }

    std::optional<QByteArray> IcnsFormat::encode(const QList<QImage> &images, QString &error) {
        QMap<int, QByteArray> pngs;
        for (const auto &image : images) {
            if (image.width() != image.height() || !sizes().contains(image.width())) {
                error = QStringLiteral("An image of %1 by %2 pixels cannot be stored in an ICNS")
                            .arg(image.width())
                            .arg(image.height());
                return std::nullopt;
            }
            pngs.insert(image.width(), pngOf(image.convertToFormat(QImage::Format_ARGB32)));
        }

        QByteArray body;
        for (const auto &element : elements) {
            const auto it = pngs.constFind(element.size);
            if (it == pngs.constEnd()) {
                continue;
            }
            body.append(element.type, 4);
            put32(body, quint32(8 + it->size()));
            body.append(*it);
        }

        QByteArray out("icns");
        put32(out, quint32(8 + body.size()));
        out.append(body);
        return out;
    }

}
