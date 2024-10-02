#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QMap>
#include <QStringList>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void readPendingDatagrams(); // Слот для чтения входящих датаграмм
    void broadcastUserList();    // Слот для рассылки списка пользователей

private:
    Ui::ServerWindow *ui;
    QUdpSocket *serverSocket;               // Указатель на объект UDP сокета
    QMap<int, QPair<QHostAddress, quint16>> users; // Карта для хранения пользователей (id, адрес, порт)
    QMap<int, QString> userNames; // Карта для хранения имен пользователей

    void processMessage(const QJsonObject &jsonObject, const QHostAddress &sender, quint16 senderPort); // Метод для обработки сообщений в формате JSON
};

#endif // SERVERWINDOW_H
