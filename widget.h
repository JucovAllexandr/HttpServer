#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <sw/redis++/redis++.h>
#include <QUuid>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

using namespace sw::redis;
using namespace bsoncxx::builder::basic;


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
    QHttpServer httpServer;
    mongocxx::instance inst;
    mongocxx::client conn{mongocxx::uri{"mongodb+srv://sania:starwars133@cluster0-wacd3.mongodb.net/test?retryWrites=true&w=majority"}};
    mongocxx::database db = conn["testdb"];
    Redis redis = Redis("tcp://127.0.0.1:6379");

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_start_clicked();

private:
    Ui::Widget *ui;
    void runServer();
};
#endif // WIDGET_H
