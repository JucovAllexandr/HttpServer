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

    httpServer.route("/", [](const QHttpServerRequest &request) {
        return request.body();
    });

    const auto port = httpServer.listen(QHostAddress::Any);

    if (port == -1) {
        qDebug() << QCoreApplication::translate(
                        "QHttpServerExample", "Could not run on http://127.0.0.1:%1/").arg(port);
        return;
    }

    qDebug() << QCoreApplication::translate(
                    "QHttpServerExample", "Running on http://127.0.0.1:%1/ ").arg(port);
}

Widget::~Widget()
{
    delete ui;
}

