#include "IcoFormat.h"

#include <QtCore/QBuffer>

namespace qiconmaker {

    namespace {

        void put16(QByteArray &out, quint32 value) {
            out.append(char(value & 0xff));
            out.append(char((value >> 8) & 0xff));
        }

        void put32(QByteArray &out, quint32 value) {
            put16(out, value & 0xffff);
            put16(out, value >> 16);
        }

        // Returns image, ARGB32 and square, as a bitmap of an icon: the header of the bitmap,
        // whose height counts the pixels and the mask, the pixels in BGRA from the bottom row up,
        // and the AND mask, one bit for each pixel, set if the pixel is transparent, each row
        // padded to 32 bits.
        QByteArray bitmapOf(const QImage &image) {
            const int size = image.width();
            const int maskStride = ((size + 31) / 32) * 4;
            QByteArray out;
            put32(out, 40);
            put32(out, quint32(size));
            put32(out, quint32(size * 2));
            put16(out, 1);
            put16(out, 32);
            put32(out, 0);
            put32(out, quint32(size * size * 4 + maskStride * size));
            put32(out, 0);
            put32(out, 0);
            put32(out, 0);
            put32(out, 0);
            for (int y = size - 1; y >= 0; --y) {
                for (int x = 0; x < size; ++x) {
                    const auto pixel = image.pixel(x, y);
                    out.append(char(qBlue(pixel)));
                    out.append(char(qGreen(pixel)));
                    out.append(char(qRed(pixel)));
                    out.append(char(qAlpha(pixel)));
                }
            }
            for (int y = size - 1; y >= 0; --y) {
                QByteArray row(maskStride, '\0');
                for (int x = 0; x < size; ++x) {
                    if (qAlpha(image.pixel(x, y)) == 0) {
                        row[x / 8] = char(row[x / 8] | (0x80 >> (x % 8)));
                    }
                }
                out.append(row);
            }
            return out;
        }

        QByteArray pngOf(const QImage &image) {
            QByteArray out;
            QBuffer buffer(&out);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            return out;
        }

    }

    QList<int> IcoFormat::defaultSizes() {
        return {16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 256};
    }

    std::optional<QByteArray> IcoFormat::encode(const QList<QImage> &images, QString &error) {
        QList<QByteArray> data;
        for (const auto &image : images) {
            if (image.width() != image.height() || image.width() < 1 ||
                image.width() > maximumSize) {
                error = QStringLiteral("An image of %1 by %2 pixels cannot be stored in an ICO")
                            .arg(image.width())
                            .arg(image.height());
                return std::nullopt;
            }
            const auto argb = image.convertToFormat(QImage::Format_ARGB32);
            data.push_back(argb.width() == maximumSize ? pngOf(argb) : bitmapOf(argb));
        }

        QByteArray out;
        put16(out, 0);
        put16(out, 1);
        put16(out, quint32(images.size()));
        auto offset = quint32(6 + 16 * images.size());
        for (int i = 0; i < images.size(); ++i) {
            // A width and a height of 256 are written as 0.
            const int size = images.at(i).width();
            out.append(char(size == maximumSize ? 0 : size));
            out.append(char(size == maximumSize ? 0 : size));
            out.append(char(0));
            out.append(char(0));
            put16(out, 1);
            put16(out, 32);
            put32(out, quint32(data.at(i).size()));
            put32(out, offset);
            offset += quint32(data.at(i).size());
        }
        for (const auto &bytes : std::as_const(data)) {
            out.append(bytes);
        }
        return out;
    }

}
