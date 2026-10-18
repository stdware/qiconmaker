#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

#include <qiconmaker/IconSet.h>

using namespace qiconmaker;

namespace {

    QString dataFile(const char *name) {
        return QStringLiteral(QICONMAKER_TEST_DATA "/") + QLatin1String(name);
    }

    IconSource sourceOf(const QString &file) {
        QString error;
        auto source = IconSource::read(file, error);
        if (!source) {
            qFatal("%s", qPrintable(error));
        }
        return std::move(*source);
    }

    // Writes a PNG of size pixels square into dir, in one color
    QString plain(const QTemporaryDir &dir, int size) {
        QImage image(size, size, QImage::Format_ARGB32);
        image.fill(qRgb(0, 128, 0));
        const auto path = dir.filePath(QStringLiteral("plain%1.png").arg(size));
        image.save(path, "PNG");
        return path;
    }

}

class test_IconSet : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The size 16 takes its own source, red, and the other sizes take the drawing, white in the
    // middle.
    void a_size_with_a_source_takes_it() {
        IconSet set(sourceOf(dataFile("drawing.svg")));
        set.setSource(16, sourceOf(dataFile("drawing16.svg")));
        QString error;
        const auto images = set.images({32, 16, 64}, error);
        QVERIFY2(images, qPrintable(error));
        QCOMPARE(images->size(), 3);
        QCOMPARE(images->at(0).size(), QSize(32, 32));
        QCOMPARE(images->at(0).pixelColor(16, 16), QColor(Qt::white));
        QCOMPARE(images->at(1).size(), QSize(16, 16));
        QCOMPARE(images->at(1).pixelColor(8, 8), QColor(Qt::red));
        QCOMPARE(images->at(2).pixelColor(32, 32), QColor(Qt::white));
    }

    void a_png_of_a_size_takes_the_size() {
        QTemporaryDir dir;
        IconSet set(sourceOf(dataFile("drawing.svg")));
        set.setSource(16, sourceOf(plain(dir, 16)));
        QString error;
        const auto images = set.images({16}, error);
        QVERIFY2(images, qPrintable(error));
        QCOMPARE(images->at(0).pixelColor(8, 8), QColor(0, 128, 0));
    }

    // A source of a size is drawn for that size, so a PNG of another size is refused rather
    // than scaled.
    void a_png_of_another_size_is_rejected() {
        QTemporaryDir dir;
        IconSet set(sourceOf(dataFile("drawing.svg")));
        set.setSource(16, sourceOf(plain(dir, 32)));
        QString error;
        QVERIFY(!set.images({16}, error));
        QVERIFY(error.contains(QLatin1String("not 16")));
    }

    // A PNG master is reduced to the smaller sizes and never enlarged.
    void a_png_master_serves_only_smaller_sizes() {
        QTemporaryDir dir;
        IconSet set(sourceOf(plain(dir, 64)));
        QString error;
        const auto images = set.images({16, 64}, error);
        QVERIFY2(images, qPrintable(error));
        QCOMPARE(images->at(0).size(), QSize(16, 16));
        QCOMPARE(images->at(1).size(), QSize(64, 64));
        QVERIFY(!set.images({128}, error));
        QVERIFY(error.contains(QLatin1String("smaller than 128")));
    }
};

QTEST_MAIN(test_IconSet)
#include "test_IconSet.moc"
