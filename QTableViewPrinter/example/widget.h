﻿#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class QPrinter;
class QSqlTableModel;

namespace Ui {
class Widget;
}

class Widget : public QWidget {
  Q_OBJECT

public:
  explicit Widget(QWidget* parent = 0);
  ~Widget();

private:
  Ui::Widget* ui;
  void initDb();
  QSqlTableModel *model;
private slots:
  void print(QPrinter *printer);
  void on_pushButton_clicked();
};

#endif // WIDGET_H
