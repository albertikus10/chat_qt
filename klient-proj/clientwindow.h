#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QMap>
#include <QStringList>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCloseEvent> // Для работы с событием закрытия окна
#include <QJsonArray>

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void readPendingDatagrams(); // Слот для чтения входящих датаграмм
    void on_sendButton_clicked(); // Слот для обработки нажатия кнопки отправки сообщения
    void closeEvent(QCloseEvent *event); // Слот для обработки события закрытия окна

private:
    Ui::ClientWindow *ui;
    QUdpSocket *udpSocket;               // Указатель на объект UDP сокета
    struct UserInfo {
        QString name;
        int id;
    } userInfo;  // Структура для хранения информации о пользователе
    QMap<int, QString> connectedUsers;  // Карта для хранения подключенных пользователей (id, имя)

    void processMessage(const QJsonObject &jsonObject); // Метод для обработки сообщений в формате JSON
    void updateConnectedUsers(const QStringList &connectedUsers);  // Метод для обновления списка подключенных пользователей
};

#endif // CLIENTWINDOW_H
