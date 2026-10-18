// qiconmaker writes an .ico or an .icns from a drawing in SVG or PNG and from optional sources of
// particular sizes. See README.md.

#include <cstdio>
#include <optional>

#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSaveFile>
#include <QtGui/QGuiApplication>

#include <qiconmaker/IcnsFormat.h>
#include <qiconmaker/IcoFormat.h>
#include <qiconmaker/IconSet.h>
#include <qiconmaker/IconSource.h>

using namespace qiconmaker;

namespace {

    int fail(const QString &message) {
        std::fprintf(stderr, "qiconmaker: %s\n", qUtf8Printable(message));
        return 1;
    }

    // Returns the sizes in text, a list of positive numbers separated by commas, or std::nullopt
    // if the text is not such a list.
    std::optional<QList<int>> sizesOf(const QString &text) {
        QList<int> sizes;
        for (const auto &part : text.split(QLatin1Char(','))) {
            bool ok = false;
            const int size = part.trimmed().toInt(&ok);
            if (!ok || size < 1) {
                return std::nullopt;
            }
            sizes.push_back(size);
        }
        return sizes;
    }

}

int main(int argc, char *argv[]) {
    // A build machine may have no display. The platform plugin is needed for the fonts of a
    // drawing with text.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("qiconmaker"));
    QCoreApplication::setApplicationVersion(QStringLiteral(QICONMAKER_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Writes an .ico or an .icns from a drawing in SVG or PNG. A size with a "
                       "source of its own takes that source, and every other size is rendered "
                       "from the drawing, or reduced from it if the drawing is a PNG."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption sizesOption(
        QStringLiteral("sizes"),
        QStringLiteral(
            "The sizes in pixels, separated by commas, in place of those of the format."),
        QStringLiteral("list"));
    const QCommandLineOption pngOption(
        QStringLiteral("png-dir"),
        QStringLiteral("Also writes each image as <size>.png into the directory, for inspection."),
        QStringLiteral("dir"));
    parser.addOption(sizesOption);
    parser.addOption(pngOption);
    parser.addPositionalArgument(QStringLiteral("output"),
                                 QStringLiteral("The file to write, .ico or .icns."));
    parser.addPositionalArgument(QStringLiteral("drawing"),
                                 QStringLiteral("The drawing, .svg, .svgz or .png."));
    parser.addPositionalArgument(QStringLiteral("size=file"),
                                 QStringLiteral("The source of the image of a size."),
                                 QStringLiteral("[size=file...]"));
    parser.process(app);

    const auto arguments = parser.positionalArguments();
    if (arguments.size() < 2) {
        return fail(QStringLiteral("An output and a drawing are required. See --help."));
    }

    const auto output = arguments.at(0);
    const auto suffix = QFileInfo(output).suffix().toLower();
    const bool ico = suffix == QLatin1String("ico");
    if (!ico && suffix != QLatin1String("icns")) {
        return fail(QStringLiteral("%1 is neither an .ico nor an .icns").arg(output));
    }

    auto sizes = ico ? IcoFormat::defaultSizes() : IcnsFormat::sizes();
    if (parser.isSet(sizesOption)) {
        const auto given = sizesOf(parser.value(sizesOption));
        if (!given) {
            return fail(QStringLiteral("%1 is no list of sizes").arg(parser.value(sizesOption)));
        }
        sizes = *given;
    }

    QString error;
    auto master = IconSource::read(arguments.at(1), error);
    if (!master) {
        return fail(error);
    }
    IconSet set(std::move(*master));
    for (int i = 2; i < arguments.size(); ++i) {
        const auto &argument = arguments.at(i);
        const int equals = argument.indexOf(QLatin1Char('='));
        bool ok = false;
        const int size = equals > 0 ? argument.left(equals).toInt(&ok) : 0;
        if (!ok || size < 1) {
            return fail(QStringLiteral("%1 is not of the form size=file").arg(argument));
        }
        auto source = IconSource::read(argument.mid(equals + 1), error);
        if (!source) {
            return fail(error);
        }
        set.setSource(size, std::move(*source));
    }

    const auto images = set.images(sizes, error);
    if (!images) {
        return fail(error);
    }
    const auto data = ico ? IcoFormat::encode(*images, error) : IcnsFormat::encode(*images, error);
    if (!data) {
        return fail(error);
    }

    // The directory of the output is created, as a build writes an icon into a directory of the
    // build tree that may not exist yet.
    const auto outputDirectory = QFileInfo(output).absolutePath();
    if (!QDir().mkpath(outputDirectory)) {
        return fail(QStringLiteral("%1 cannot be created").arg(outputDirectory));
    }

    // Written whole or not at all, so that a failed run leaves no truncated icon for the build
    QSaveFile file(output);
    if (!file.open(QIODevice::WriteOnly) || file.write(*data) != data->size() || !file.commit()) {
        return fail(QStringLiteral("%1 cannot be written: %2").arg(output, file.errorString()));
    }

    if (parser.isSet(pngOption)) {
        const QDir directory(parser.value(pngOption));
        if (!QDir().mkpath(directory.path())) {
            return fail(QStringLiteral("%1 cannot be created").arg(directory.path()));
        }
        for (int i = 0; i < sizes.size(); ++i) {
            const auto path = directory.filePath(QStringLiteral("%1.png").arg(sizes.at(i)));
            if (!images->at(i).save(path, "PNG")) {
                return fail(QStringLiteral("%1 cannot be written").arg(path));
            }
        }
    }
    return 0;
}
