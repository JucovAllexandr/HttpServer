#include "widget.h"
#include "ui_widget.h"

static inline QString host(const QHttpServerRequest &request)
{
    return request.headers()[QStringLiteral("Host")].toString();
}

static inline QString methodToString(QHttpServerRequest::Method method){
    QString str = "";
    switch (method) {
    case QHttpServerRequest::Method::Get: str = "GET"; break;
    case QHttpServerRequest::Method::Put: str = "PUT"; break;
    case QHttpServerRequest::Method::Post: str = "POST"; break;
    case QHttpServerRequest::Method::Delete: str = "DELETE"; break;
    }

    return str;
}


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    auto collection = db["testcollection"];
    mongocxx::cursor cursor = collection.find({});
    for(auto doc : cursor) {
        qDebug() << bsoncxx::to_json(doc).c_str() << "\n";
    }

    httpServer.route("/", [this](const QHttpServerRequest &request) {
        QString retVal;
        QJsonObject json = QJsonDocument::fromJson(request.body()).object();

        switch (request.method()) {
        case QHttpServerRequest::Method::Put: {
            auto collection = db[json.value("topic").toString().toStdString()];
            bsoncxx::builder::basic::document filter;
            filter.append(kvp(std::string("_id"), json.value("_id").toString().toStdString()));
            bsoncxx::stdx::optional<mongocxx::result::update> res;
            bsoncxx::builder::basic::document updateVal;

            for(QString key: json.keys()){
                if(key != "_id"){
                    updateVal.append(kvp(key.toStdString(), json.value(key).toString().toStdString()));
                }
            }
            collection.update_one(filter.view(), updateVal.view());

        } break;
        case QHttpServerRequest::Method::Post: {
            if(json.value("topic") == QJsonValue::Undefined){
                retVal = "Error topic not found";
            }else{
                QUuid id = QUuid::createUuid();
                QString topic = json.value("topic").toString();
                auto collection = db[json.value("topic").toString().toStdString()];
                auto doc = bsoncxx::builder::basic::document();
                doc.append(kvp(std::string("_id"), id.toString(QUuid::Id128).toStdString()));
                for(QString key: json.keys()){
                    if(key != "_id" || key != "topic"){
                        doc.append(kvp(key.toStdString(), json.value(key).toString().toStdString()));
                    }
                }
                collection.insert_one(doc.view());

                redis.hset(id.toString(QUuid::Id128).toStdString(), "topic", topic.toStdString());

                for(QString key: json.keys()){
                    if(key != "_id" || key != "topic"){
                        redis.hset(id.toString(QUuid::Id128).toStdString(), key.toStdString(), json.value(key).toString().toStdString());
                    }
                }

                retVal = id.toString(QUuid::Id128);
            }
        } break;
        case QHttpServerRequest::Method::Delete:  {
            retVal = "";

            if(json.isEmpty() || json.value("_id") == QJsonValue::Undefined || json.value("topic") == QJsonValue::Undefined){
                retVal = "Err _id not found";
            }else{
                auto collection = db[json.value("topic").toString().toStdString()];
                bsoncxx::builder::basic::document doc;
                doc.append(kvp(std::string("_id"), json.value("_id").toString().toStdString()));
                bsoncxx::stdx::optional<mongocxx::result::delete_result> delRes = collection.delete_one(doc.view());
                if(delRes){
                 retVal = "Deleted";
                 redis.del(json.value("_id").toString().toStdString());
                }else{
                    retVal = "Err delete";
                }
            }

        }break;
        }


        qDebug()<<request.body();
        return retVal;
    });

    httpServer.route("/<arg>/<arg>", [this](QString topic, QString id, const QHttpServerRequest &request) {
        QString retVal = "Not found";
        std::vector<std::pair<std::string, std::string>> hash_vec;
        redis.hgetall(id.toStdString(), std::back_inserter(hash_vec));
        if(hash_vec.size()){
            QJsonObject json;
            json.insert("_id", id);

            for(auto i = hash_vec.begin(); i != hash_vec.end(); ++i){
                json.insert(i->first.c_str(), i->second.c_str());
            }

            QJsonDocument doc;
            doc.setObject(json);
            retVal = doc.toJson();

        }else{
            auto collection = db[topic.toStdString()];
            bsoncxx::builder::basic::document doc;
            doc.append(kvp(std::string("_id"), id.toStdString()));
            bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
                    collection.find_one(doc.view());

            if(maybe_result) {
                QJsonDocument doc;
                retVal =  bsoncxx::to_json(*maybe_result).c_str();
                doc = QJsonDocument::fromJson(retVal.toStdString().c_str());
                QJsonObject obj = doc.object();
                QStringList keys = obj.keys();
                for(QString key: keys){
                    redis.hset(id.toStdString(), key.toStdString(), obj.value(key).toString().toStdString());
                }
            }
        }

        return retVal;
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::runServer()
{
    const auto port = httpServer.listen(QHostAddress::Any, ui->spinBox_server_port->value());

    if (port == -1) {
        qDebug() << QCoreApplication::translate(
                        "QHttpServerExample", "Could not run on http://127.0.0.1:%1/").arg(port);
        return;
    }

    qDebug() << QCoreApplication::translate(
                    "QHttpServerExample", "Running on http://127.0.0.1:%1/ ").arg(port);
}


void Widget::on_pushButton_start_clicked()
{
    runServer();
}
