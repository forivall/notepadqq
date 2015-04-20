#include "include/Extensions/extension.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTime>
#include <QDebug>

namespace Extensions {

    Extension::Extension(QString path, QString serverSocketPath) : QObject(0)
    {
        m_extensionId = path + "-" + QTime::currentTime().msec() + "-" + QString::number(qrand());

        QFile fManifest(path + "/manifest.json");
        if (fManifest.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&fManifest);
            QString content = in.readAll();
            fManifest.close();

            QJsonParseError err;
            QJsonDocument manifestDoc = QJsonDocument::fromJson(content.toUtf8(), &err);

            if (err.error != QJsonParseError::NoError) {
                failedToLoadExtension(path, "manifest.json: " + err.errorString());
                return;
            }

            QJsonObject manifest = manifestDoc.object();
            m_name = manifest.value("name").toString();

            if (m_name.isEmpty()) {
                failedToLoadExtension(path, "name missing or invalid");
                return;
            }

            QProcess *process = new QProcess(this);
            process->setProcessChannelMode(QProcess::ForwardedChannels);
            process->setWorkingDirectory(path);
            QStringList args;
            args << serverSocketPath;
            args << m_extensionId;
            process->start("./start.sh", args); // Are we sure it's ".sh"?

        } else {
            failedToLoadExtension(path, "manifest.json missing");
            return;
        }

        // FIXME Load editor parts
        /*QFile fUi(path + "/ui.js");
        if (fUi.open(QFile::ReadOnly | QFile::Text)) {


        } else {
            failedToLoadExtension(path, "ui.js missing");
            return;
        }*/
    }

    Extension::~Extension()
    {

    }

    void Extension::failedToLoadExtension(QString path, QString reason)
    {
        // FIXME Mark extension as broken
        qWarning() << QString("Failed to load %1: %2").arg(path).arg(reason).toStdString().c_str();
    }

    QString Extension::id() const
    {
        return m_extensionId;
    }

    QString Extension::name() const
    {
        return m_name;
    }

}
