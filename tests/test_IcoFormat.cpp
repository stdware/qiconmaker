#include <algorithm>

#include <QtCore/QBuffer>
#include <QtGui/QImageReader>
#include <QtTest/QTest>

#include <qiconmaker/IcoFormat.h>
#include <qiconmaker/IconSource.h>

using namespace qiconmaker;

namespace {

    QList<QImage> imagesAt(const QList<int> &sizes) {
        QString error;
        const auto source =
            IconSource::read(QStringLiteral(QICONMAKER_TEST_DATA "/drawing.svg"), error);
        if (!source) {
            qFatal("%s", qPrintable(error));
        }
        QList<QImage> images;
        for (const int size : sizes) {
            images.push_back(*source->image(size));
        }
        return images;
    }

    quint32 read16(const QByteArray &data, int at) {
        return quint8(data.at(at)) | quint32(quint8(data.at(at + 1))) << 8;
    }

    quint32 read32(const QByteArray &data, int at) {
        return read16(data, at) | read16(data, at + 2) << 16;
    }

}

class test_IcoFormat : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void the_default_sizes_end_with_256() {
        const auto sizes = IcoFormat::defaultSizes();
        QCOMPARE(sizes.first(), 16);
        QCOMPARE(sizes.last(), 256);
        QVERIFY(std::is_sorted(sizes.begin(), sizes.end()));
    }

    // The directory lists the images in order. 256 is written as 0 and stored as PNG, and a
    // smaller image is a bitmap whose height counts the pixels and the mask.
    void the_directory_lists_the_images() {
        QString error;
        const auto data = IcoFormat::encode(imagesAt({16, 32, 256}), error);
        QVERIFY2(data, qPrintable(error));
        QCOMPARE(read16(*data, 0), 0u);
        QCOMPARE(read16(*data, 2), 1u);
        QCOMPARE(read16(*data, 4), 3u);

        quint32 offset = 6 + 16 * 3;
        const int sizes[] = {16, 32, 256};
        for (int i = 0; i < 3; ++i) {
            const int entry = 6 + 16 * i;
            QCOMPARE(int(quint8(data->at(entry))), sizes[i] % 256);
            QCOMPARE(int(quint8(data->at(entry + 1))), sizes[i] % 256);
            QCOMPARE(read16(*data, entry + 6), 32u);
            QCOMPARE(read32(*data, entry + 12), offset);
            const auto length = read32(*data, entry + 8);
            if (sizes[i] == 256) {
                QVERIFY(data->mid(int(offset), 8) == QByteArray("\x89PNG\r\n\x1a\n", 8));
            } else {
                QCOMPARE(read32(*data, int(offset)), 40u);
                QCOMPARE(read32(*data, int(offset) + 8), quint32(sizes[i] * 2));
                const int maskStride = ((sizes[i] + 31) / 32) * 4;
                QCOMPARE(length, quint32(40 + sizes[i] * sizes[i] * 4 + maskStride * sizes[i]));
            }
            offset += length;
        }
        QCOMPARE(quint32(data->size()), offset);
    }

    // The reader of Qt reads the file back with the colors and the transparency of each image.
    void the_file_is_read_back() {
        QVERIFY2(QImageReader::supportedImageFormats().contains("ico"),
                 "The ICO plugin of Qt is required.");
        const auto images = imagesAt({16, 48, 256});
        QString error;
        auto data = IcoFormat::encode(images, error);
        QVERIFY2(data, qPrintable(error));

        QBuffer buffer(&*data);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer, "ico");
        QCOMPARE(reader.imageCount(), 3);
        for (int i = 0; i < 3; ++i) {
            QVERIFY(reader.jumpToImage(i));
            const auto image = reader.read().convertToFormat(QImage::Format_ARGB32);
            QCOMPARE(image.size(), images.at(i).size());
            const int size = image.width();
            QCOMPARE(qAlpha(image.pixel(0, 0)), 0);
            QCOMPARE(image.pixel(size / 2, size / 2), images.at(i).pixel(size / 2, size / 2));
            QCOMPARE(image.pixel(size / 4, size / 2), images.at(i).pixel(size / 4, size / 2));
        }
    }

    void an_image_that_an_ico_cannot_hold_is_rejected() {
        QString error;
        QVERIFY(!IcoFormat::encode(imagesAt({512}), error));
        QVERIFY(error.contains(QLatin1String("512")));
        QVERIFY(!IcoFormat::encode({QImage(16, 32, QImage::Format_ARGB32)}, error));
    }
};

QTEST_MAIN(test_IcoFormat)
#include "test_IcoFormat.moc"
