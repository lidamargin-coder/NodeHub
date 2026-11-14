#include "AppController.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QStandardPaths>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include <QIcon>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QTimer>
#include <QImage>
#include <QSharedPointer>
#include <functional>

class DiskCacheNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory {
public:
    QNetworkAccessManager* create(QObject* parent) override {
        QNetworkAccessManager* nam = new QNetworkAccessManager(parent);
        QNetworkDiskCache* diskCache = new QNetworkDiskCache(nam);
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/nodehub_images";
        QDir().mkpath(cacheDir);
        diskCache->setCacheDirectory(cacheDir);
        diskCache->setMaximumCacheSize(500 * 1024 * 1024); // 500 MB disk cache
        nam->setCache(diskCache);
        return nam;
    }
};

static QFile g_logFile;

static void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    if (!g_logFile.isOpen()) {
        g_logFile.setFileName(QDir::tempPath() + "/nodehub_debug.log");
        g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }
    QTextStream out(&g_logFile);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    switch (type) {
    case QtDebugMsg:    out << "[DEBUG] "; break;
    case QtInfoMsg:     out << "[INFO]  "; break;
    case QtWarningMsg:  out << "[WARN]  "; break;
    case QtCriticalMsg: out << "[CRIT]  "; break;
    case QtFatalMsg:    out << "[FATAL] "; break;
    }
    out << msg << "\n";
    out.flush();
    g_logFile.flush();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("NodeHub");
    app.setApplicationName("NodeHub");
    app.setApplicationDisplayName("NodeHub");
    app.setWindowIcon(QIcon(":/NodeHub/logos/app_icon_256.png"));

    qInstallMessageHandler(customLogHandler);
    qDebug() << "NodeHub starting up...";

    // Register modern custom fonts from resources
    QFontDatabase::addApplicationFont(":/NodeHub/fonts/Inter-Regular.ttf");
    QFontDatabase::addApplicationFont(":/NodeHub/fonts/Inter-Medium.ttf");
    QFontDatabase::addApplicationFont(":/NodeHub/fonts/Inter-Bold.ttf");
    QFontDatabase::addApplicationFont(":/NodeHub/fonts/JetBrainsMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/NodeHub/fonts/JetBrainsMono-Bold.ttf");

    QFont defaultFont("Inter", 10);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(defaultFont);

    QQuickStyle::setStyle("Basic");

    AppController controller;

    QQmlApplicationEngine engine;
    engine.setNetworkAccessManagerFactory(new DiskCacheNetworkAccessManagerFactory());

    engine.rootContext()->setContextProperty("app", &controller);
    engine.rootContext()->setContextProperty("modModel", controller.modModel());
    engine.rootContext()->setContextProperty("settingsMgr", controller.settings());
    engine.rootContext()->setContextProperty("downloadManager", controller.downloadManager());
    engine.rootContext()->setContextProperty("downloader", controller.downloadManager()); // alias
    engine.rootContext()->setContextProperty("installedMods", controller.installedMods());

    const QUrl url(QStringLiteral("qrc:/NodeHub/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [](const QUrl &failedUrl) {
            qCritical() << "Object creation failed for URL:" << failedUrl;
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Engine root objects is empty!";
    } else {
        qDebug() << "Root object loaded successfully!";
    }

    controller.initialize();

    const QStringList args = app.arguments();
    const int captureIdx = args.indexOf(QStringLiteral("--capture-screenshots"));
    if (captureIdx >= 0 && captureIdx + 1 < args.size() && !engine.rootObjects().isEmpty()) {
        const QString outDir = args.at(captureIdx + 1);
        QDir().mkpath(outDir);
        QQuickWindow* win = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
        if (win) {
            auto saveShot = [win, outDir](const QString& fileName, std::function<void()> next) {
                const QString path = QDir(outDir).filePath(fileName);
                QImage img = win->grabWindow();
                if (!img.isNull() && img.width() > 0 && img.height() > 0) {
                    img.save(path, "PNG");
                    qDebug() << "Saved grabWindow screenshot:" << path << img.size();
                    if (next) next();
                    return;
                }
                if (win->contentItem()) {
                    QSharedPointer<QQuickItemGrabResult> res = win->contentItem()->grabToImage();
                    if (res) {
                        QObject::connect(res.data(), &QQuickItemGrabResult::ready, win, [res, path, next]() {
                            res->saveToFile(path);
                            qDebug() << "Saved grabToImage screenshot:" << path << res->image().size();
                            if (next) next();
                        });
                        return;
                    }
                }
                qWarning() << "Failed to capture screenshot:" << path;
                if (next) next();
            };

            QTimer::singleShot(7000, win, [&controller, &app, saveShot, win, outDir]() {
                saveShot(QStringLiteral("hero_preview.png"), [&controller, &app, saveShot, win, outDir]() {
                    // Duplicate hero_preview as screen_catalog for the gallery switcher
                    const QString heroPath = QDir(outDir).filePath(QStringLiteral("hero_preview.png"));
                    const QString catPath = QDir(outDir).filePath(QStringLiteral("screen_catalog.png"));
                    QFile::remove(catPath);
                    QFile::copy(heroPath, catPath);

                    controller.selectCategory(QStringLiteral("installed"));
                    QTimer::singleShot(2000, win, [&controller, &app, saveShot, win]() {
                        saveShot(QStringLiteral("screen_installed.png"), [&controller, &app, saveShot, win]() {
                            controller.selectCategory(QStringLiteral("maps"));
                            QTimer::singleShot(6000, win, [&app, saveShot]() {
                                saveShot(QStringLiteral("screen_maps.png"), [&app]() {
                                    app.quit();
                                });
                            });
                        });
                    });
                });
            });
        }
    }

    return app.exec();
}
