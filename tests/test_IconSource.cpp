#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

#include <qiconmaker/IconSource.h>

using namespace qiconmaker;

namespace {

    QString dataFile(const char *name) {
        return QStringLiteral(QICONMAKER_TEST_DATA "/") + QLatin1String(name);
    }

    // Writes a PNG of size pixels square into dir, black and white in alternating pixels
    QString checkerboard(const QTemporaryDir &dir, int size) {
        QImage image(size, size, QImage::Format_ARGB32);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                image.setPixel(x, y, (x + y) % 2 ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
            }
        }
        const auto path = dir.filePath(QStringLiteral("checkerboard%1.png").arg(size));
        image.save(path, "PNG");
        return path;
    }

}

class test_IconSource : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // An SVG is rendered at each size: transparent in the margin, blue on the plate and white in
    // the middle.
    void an_svg_is_rendered_at_any_size() {
        QString error;
        const auto source = IconSource::read(dataFile("drawing.svg"), error);
        QVERIFY2(source, qPrintable(error));
        QVERIFY(source->isVector());
        for (const int size : {16, 48, 256}) {
            const auto image = source->image(size);
            QVERIFY(image);
            QCOMPARE(image->size(), QSize(size, size));
            QCOMPARE(image->format(), QImage::Format_ARGB32);
            QCOMPARE(qAlpha(image->pixel(0, 0)), 0);
            QCOMPARE(image->pixelColor(size / 2, size / 2), QColor(Qt::white));
            QCOMPARE(image->pixelColor(size / 4, size / 2), QColor(0x33, 0x70, 0xff));
        }
    }

    void a_drawing_that_is_not_square_is_rejected() {
        QString error;
        QVERIFY(!IconSource::read(dataFile("wide.svg"), error));
        QVERIFY(error.contains(QLatin1String("not square")));
    }

    void an_unreadable_file_is_reported() {
        QString error;
        QVERIFY(!IconSource::read(dataFile("missing.png"), error));
        QVERIFY(error.contains(QLatin1String("missing.png")));
    }

    void a_png_of_the_size_is_returned_as_it_is() {
        QTemporaryDir dir;
        QString error;
        const auto source = IconSource::read(checkerboard(dir, 32), error);
        QVERIFY2(source, qPrintable(error));
        QVERIFY(!source->isVector());
        QCOMPARE(source->pixelSize(), 32);
        const auto image = source->image(32);
        QVERIFY(image);
        QCOMPARE(*image, QImage(checkerboard(dir, 32)).convertToFormat(QImage::Format_ARGB32));
    }

    // A larger PNG is reduced by averaging, so that a checkerboard of single pixels becomes an
    // even gray rather than the black or the white of the pixels that a single scaling samples.
    void a_larger_png_is_reduced_by_averaging() {
        QTemporaryDir dir;
        QString error;
        const auto source = IconSource::read(checkerboard(dir, 256), error);
        QVERIFY2(source, qPrintable(error));
        const auto image = source->image(16);
        QVERIFY(image);
        QCOMPARE(image->size(), QSize(16, 16));
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                const int gray = qGray(image->pixel(x, y));
                QVERIFY2(gray > 112 && gray < 144,
                         qPrintable(QStringLiteral("%1 at %2, %3").arg(gray).arg(x).arg(y)));
            }
        }
    }

    void a_smaller_png_is_not_enlarged() {
        QTemporaryDir dir;
        QString error;
        const auto source = IconSource::read(checkerboard(dir, 16), error);
        QVERIFY2(source, qPrintable(error));
        QVERIFY(!source->image(32));
    }
};

QTEST_MAIN(test_IconSource)
#include "test_IconSource.moc"
