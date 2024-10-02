#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QInputDialog>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8888

ClientWindow::ClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow),
    udpSocket(new QUdpSocket(this))
{
    ui->setupUi(this);

    connect(udpSocket, &QUdpSocket::readyRead, this, &ClientWindow::readPendingDatagrams);

    // Генерируем случайное id для пользователя
    userInfo.id = QRandomGenerator::global()->generate() % 1000; // Используем QRandomGenerator

    // Запрос имени пользователя с использованием QInputDialog
    bool ok;
    QString name = QInputDialog::getText(this, tr("Введите ваше имя"),
                                         tr("Имя пользователя:"), QLineEdit::Normal,
                                         QString(), &ok);
    if (!ok || name.isEmpty()) {
        // Если пользователь отменил ввод или не ввел имя, закрываем окно
        QMessageBox::critical(this, tr("Ошибка"), tr("Вы должны ввести имя пользователя."));
        close();
    } else {
        // Добавляем символ "@" и ID в конец имени пользователя
        userInfo.name = name + "@" + QString::number(userInfo.id);
    }

    // Отправляем серверу информацию о новом пользователе
    QJsonObject jsonObject;
    jsonObject["type"] = "newUser";
    jsonObject["name"] = userInfo.name;
    jsonObject["id"] = userInfo.id;

    QByteArray datagram = QJsonDocument(jsonObject).toJson();
    udpSocket->writeDatagram(datagram, QHostAddress(SERVER_ADDRESS), SERVER_PORT);
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

void ClientWindow::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

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
        processMessage(jsonObject);
    }
}

void ClientWindow::on_sendButton_clicked()
{
    QString message = ui->messageLineEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // Формируем JSON-сообщение для отправки
    QJsonObject jsonObject;
    jsonObject["type"] = "message";
    jsonObject["username"] = userInfo.name;
    jsonObject["id"] = userInfo.id; // Передаем id пользователя
    jsonObject["message"] = message;

    // Отображаем сообщение в собственном интерфейсе
    ui->chatTextEdit->append(/*userInfo.name*/ "Вы: " + message);

    QByteArray datagram = QJsonDocument(jsonObject).toJson();
    udpSocket->writeDatagram(datagram, QHostAddress(SERVER_ADDRESS), SERVER_PORT);

    ui->messageLineEdit->clear();
}

void ClientWindow::closeEvent(QCloseEvent *event)
{
    // Показываем предупреждение пользователю
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, tr("Предупреждение"),
                                 tr("Вы действительно хотите закрыть окно?"),
                                 QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Если пользователь подтвердил закрытие окна

        // Отправляем серверу сообщение о том, что пользователь отключается
        QJsonObject jsonObject;
        jsonObject["type"] = "userDisconnected";
        jsonObject["username"] = userInfo.name;
        jsonObject["id"] = userInfo.id; // Передаем id пользователя

        QByteArray datagram = QJsonDocument(jsonObject).toJson();
        udpSocket->writeDatagram(datagram, QHostAddress(SERVER_ADDRESS), SERVER_PORT);

        event->accept();
    } else {
        // Если пользователь отменил закрытие окна
        event->ignore();
    }
}


void ClientWindow::processMessage(const QJsonObject &jsonObject)
{
    if (jsonObject.contains("type") && jsonObject["type"].isString()) {
        QString type = jsonObject["type"].toString();
        if (type == "userList") {
            if (jsonObject.contains("users") && jsonObject["users"].isArray()) {
                QJsonArray usersArray = jsonObject["users"].toArray();
                QStringList connectedUsers;
                for (int i = 0; i < usersArray.size(); ++i) {
                    QJsonObject userObject = usersArray[i].toObject();
                    if (userObject.contains("id") && userObject["id"].isDouble() &&
                        userObject.contains("name") && userObject["name"].isString()) {
                        connectedUsers.append(userObject["name"].toString());
                    }
                }
                // Обновляем список подключенных пользователей в интерфейсе
                updateConnectedUsers(connectedUsers);
            }
        } else if (type == "message") {
            // Обрабатываем полученное сообщение и отображаем его
            if (jsonObject.contains("username") && jsonObject["username"].isString() &&
                jsonObject.contains("message") && jsonObject["message"].isString()) {
                QString username = jsonObject["username"].toString();
                QString message = jsonObject["message"].toString();
                ui->chatTextEdit->append(username + ": " + message);
            }
        }
    }
}

void ClientWindow::updateConnectedUsers(const QStringList &connectedUsers)
{
    // Обновляем список подключенных пользователей в интерфейсе
    ui->connectedUsersTextEdit->clear();
    for (const QString &user : connectedUsers) {
        ui->connectedUsersTextEdit->append(user);
    }
}
