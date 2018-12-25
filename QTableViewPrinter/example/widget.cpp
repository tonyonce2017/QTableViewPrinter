#include "widget.h"
#include "ui_widget.h"
#include "../qtableviewprinter.h"

#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

Widget::Widget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Widget) {
    ui->setupUi(this);
    initDb();
    model = new QSqlTableModel;
    model->setTable("exampleTable");
    model->select();
    ui->tableView->setModel(model);
}

Widget::~Widget() {
    delete ui;
    delete model;
}

void Widget::print(QPrinter *printer)
{
    //定义纸张属性
    printer->setPageSize(QPrinter::A4);
    //申明画笔
    QPainter painter;
    if(!painter.begin(printer)) {
        qWarning() << "开启打印失败!";
        return;
    }
    //申明tablePrinter
    QTableViewPrinter tablePrinter(&painter, printer);
    QVector<int> colWidth;
    for(int i=0;i<ui->tableView->model()->columnCount();++i){
        colWidth.push_back(ui->tableView->columnWidth(i));
    }

    //设定字体
    QFont headerFont;
    headerFont.setBold(true);
    QFont contentFont;
    contentFont.setPointSize(15);
    tablePrinter.setContentFont(contentFont);

    //设定标题
    PageTitle* title=new PageTitle(&painter,printer);
    title->setPageTitle("表格打印测试实例");
    tablePrinter.setPagerTitle(title);
    tablePrinter.setTitleFlag(TitleFlag::EVERY_PAGE);

    //设定页脚
    PageFooter* pagefooter=new PageFooter(&painter);
    pagefooter->setCreater("系统管理员");
    tablePrinter.setPagerFooter(pagefooter);

    QStringList headers = { "标题0" , "标题1|内容1" , "标题2|内容2" , "标题2|内容3"};
    QStringList headers2 = { "标题0" , "标题1|内容1" , "标题2|内容2" , "标题2|内容3"};
    if(!tablePrinter.printTable(ui->tableView, headers, headers2)) {
        qDebug() << tablePrinter.lastError();
    }
    painter.end();
}

void Widget::initDb() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("exampleDb");
    if(!db.open()) {
        qWarning() << "can't open db" << db.lastError().text();
    }
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS exampleTable(number int, b TEXT, c TEXT, d TEXT);");
    query.exec("DELETE FROM exampleTable;");
    QSqlDatabase::database().transaction();
    for(int i = 1; i < 21; i++) {
        query.exec(QString("INSERT INTO exampleTable VALUES (%1, '%2', '%3', '%4');")
                   .arg(i).arg("column2").arg("column3").arg("column4"));
    }
    QSqlDatabase::database().commit();
}

void Widget::on_pushButton_clicked()
{
    QPrintPreviewDialog dialog;
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    dialog.exec();
}
