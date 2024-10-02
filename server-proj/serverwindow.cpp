#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define PORT_NUMBER 8888

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow),
    serverSocket(new QUdpSocket(this))
{
    ui->setupUi(this);

    connect(serverSocket, &QUdpSocket::readyRead, this, &ServerWindow::readPendingDatagrams);

    if (!serverSocket->bind(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось привязать сокет к порту %1. Проверьте, что порт не занят другим приложением.").arg(PORT_NUMBER));
        close();
    }
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::readPendingDatagrams()
{
    while (serverSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(serverSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        serverSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram, &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Ошибка разбора JSON:" << error.errorString();
            continue;
        }

        if (!jsonDoc.isObject()) {
            qWarning() << "Полученные данные не являются объектом JSON";
            continue;
        }

        QJsonObject jsonObject = jsonDoc.object();
        processMessage(jsonObject, sender, senderPort);
    }
}

void ServerWindow::processMessage(const QJsonObject &jsonObject, const QHostAddress &sender, quint16 senderPort)
{
    if (jsonObject.contains("type") && jsonObject["type"].isString()) {
        QString type = jsonObject["type"].toString();
        if (type == "userDisconnected") {
            if (jsonObject.contains("id") && jsonObject["id"].isDouble()) {
                int userId = jsonObject["id"].toInt();
                ui->textEdit->append("Пользователь с id " + QString::number(userId) + " отключился.");

                if (users.contains(userId)) {
                    users.remove(userId);
                }
                broadcastUserList();
            }
        } else if (type == "message") {
            if (jsonObject.contains("id") && jsonObject["id"].isDouble() &&
                jsonObject.contains("message") && jsonObject["message"].isString()) {
                int userId = jsonObject["id"].toInt();
                QString message = jsonObject["message"].toString();

                ui->textEdit->append("Получено от пользователя с id " + QString::number(userId) + ": " + message);

                QByteArray datagram = QJsonDocument(jsonObject).toJson();
                for (int id : users.keys()) {
                    if (id != userId) {
                        serverSocket->writeDatagram(datagram, users[id].first, users[id].second);
                    }
                }
            }
        } else if (type == "newUser") {
            if (jsonObject.contains("name") && jsonObject["name"].isString() &&
                jsonObject.contains("id") && jsonObject["id"].isDouble()) {
                QString username = jsonObject["name"].toString();
                int userId = jsonObject["id"].toInt();

                users[userId] = qMakePair(sender, senderPort);
                userNames[userId] = username; // сохраняем имя пользователя

                ui->textEdit->append("Новый пользователь подключился: " + username + " с id " + QString::number(userId));

                broadcastUserList();
            }
        }
    }
}

void ServerWindow::broadcastUserList()
{
    QJsonArray usersArray;
    for (int userId : users.keys()) {
        QJsonObject userObject;
        userObject["id"] = userId;
        userObject["name"] = userNames[userId]; // используем сохраненное имя пользователя
        usersArray.append(userObject);
    }

    QJsonObject jsonObject;
    jsonObject["type"] = "userList";
    jsonObject["users"] = usersArray;

    QByteArray datagram = QJsonDocument(jsonObject).toJson();
    for (int id : users.keys()) {
        serverSocket->writeDatagram(datagram, users[id].first, users[id].second);
    }
}
