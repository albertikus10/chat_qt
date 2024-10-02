#include "clientwindow.h"  // Включаем заголовочный файл для класса ClientWindow

#include <QApplication>  // Включаем заголовочный файл для QApplication, который управляет приложением Qt

int main(int argc, char *argv[])  // Определение функции main, точка входа в приложение
{
    QApplication a(argc, argv);  // Создаем объект QApplication для управления приложением
    ClientWindow w;  // Создаем объект класса ClientWindow, который представляет главное окно приложения
    w.show();  // Отображаем главное окно приложения
    return a.exec();  // Запускаем основной цикл обработки событий приложения и возвращаем код выхода по его завершении
}
