#include <QtTest/QTest>

#include <qiconmaker/IcnsFormat.h>
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

    quint32 readBig32(const QByteArray &data, int at) {
        return quint32(quint8(data.at(at))) << 24 | quint32(quint8(data.at(at + 1))) << 16 |
               quint32(quint8(data.at(at + 2))) << 8 | quint32(quint8(data.at(at + 3)));
    }

    struct Element {
        QByteArray type;
        int width;
        int height;
    };

    // Returns the elements of data, with the size that the header of each PNG records, or an
    // empty list if the file is malformed.
    QList<Element> elementsOf(const QByteArray &data) {
        QList<Element> elements;
        if (!data.startsWith("icns") || readBig32(data, 4) != quint32(data.size())) {
            return {};
        }
        int at = 8;
        while (at < data.size()) {
            const auto length = int(readBig32(data, at + 4));
            const auto png = data.mid(at + 8, length - 8);
            if (length < 8 || !png.startsWith(QByteArray("\x89PNG\r\n\x1a\n", 8))) {
                return {};
            }
            // The header chunk IHDR follows the signature and records the width and the height.
            elements.push_back({data.mid(at, 4), int(readBig32(png, 16)), int(readBig32(png, 20))});
            at += length;
        }
        return at == data.size() ? elements : QList<Element>();
    }

}

class test_IcnsFormat : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Each image is stored under each type of its size, in the order in which iconutil writes
    // the types.
    void the_images_are_stored_under_every_type_of_their_size() {
        QString error;
        const auto data = IcnsFormat::encode(imagesAt(IcnsFormat::sizes()), error);
        QVERIFY2(data, qPrintable(error));
        const auto elements = elementsOf(*data);
        const QList<QPair<QByteArray, int>> expected = {
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
        QCOMPARE(elements.size(), expected.size());
        for (int i = 0; i < expected.size(); ++i) {
            QCOMPARE(elements.at(i).type, expected.at(i).first);
            QCOMPARE(elements.at(i).width, expected.at(i).second);
            QCOMPARE(elements.at(i).height, expected.at(i).second);
        }
    }

    void the_types_of_the_sizes_given_are_written() {
        QString error;
        const auto data = IcnsFormat::encode(imagesAt({32}), error);
        QVERIFY2(data, qPrintable(error));
        const auto elements = elementsOf(*data);
        QCOMPARE(elements.size(), 2);
        QCOMPARE(elements.at(0).type, QByteArray("icp5"));
        QCOMPARE(elements.at(1).type, QByteArray("ic11"));
    }

    void a_size_without_a_type_is_rejected() {
        QString error;
        QVERIFY(!IcnsFormat::encode(imagesAt({48}), error));
        QVERIFY(error.contains(QLatin1String("48")));
    }
};

QTEST_MAIN(test_IcnsFormat)
#include "test_IcnsFormat.moc"
