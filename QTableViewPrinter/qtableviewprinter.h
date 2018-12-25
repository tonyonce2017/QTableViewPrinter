#ifndef QTABLEVIEWPRINTER_H
#define QTABLEVIEWPRINTER_H

#include <QPen>
#include <QFont>
#include <QPrinter>
#include <QTableView>

class QPainter;
class QAbstractItemModel;

enum TitleFlag{
    FIRST_PAGE=0X1,
    EVERY_PAGE=0X2,
};

struct PrintColumn{
    QString name;
    int modelColumn;
    int columnWidth;
};

struct mergeColumn{
    QString text;
    QStringList columnList;
};

typedef QList<PrintColumn*> PrintColumnList;
typedef QHash<int,QList<mergeColumn*>> MergeColumnHash;


//标题
class PageTitle
{
public:
    PageTitle(QPainter *painter, QPrinter* printer);
    virtual ~PageTitle(){}
    void setPageTitle(const QString& title);
    void setTitleFont(QFont font);
    void setSideTitleFont(QFont font);
    void setSideTitle(const QStringList& sidetitle);
public:
    virtual void startDrawTitle();
private:
    QPainter* mpainter;
    QPrinter* mprinter;
    QFont titleFont;
    QFont sidetitleFont;
    QString mtitle;
    QStringList msideTitles;
};

//页眉
class PageHeader
{
public:
    PageHeader(QPainter *painter);
    virtual ~PageHeader(){}
    void setPageHeaderFont(QFont font);
public:
    virtual void startDrawHeader();
private:
    QPainter* mpainter;
    QFont pageHeaderFont;
};

//页脚
class PageFooter
{
public:
    PageFooter(QPainter *painter);
    ~PageFooter(){}
    virtual void startDrawFooter();
    void setCreater(const QString& name);
    void setPageFooterFont(QFont font);
    void resetPageNumber();
    void setViewFlag(bool zbr=true, bool ym=true, bool zbsj=true);
private:
    QPainter* mpainter;
    static int pageNumber;
    QString creater;
    QFont pageFooterFont;
    bool mzbr,mym, mzbsj;
};

//主体内容
class QTableViewPrinter
{
public:
    QTableViewPrinter(QPainter *painter, QPrinter *printer);
    bool printTable(QTableView* tableView, const QStringList &totalFields, const QStringList &visualFields);
    QString lastError();
    static int headerHeight;
    static int leftBlank;
    static int rightBlank;

    //设置单元格内间距
    void setCellMargin(int left = 10, int right = 5, int top = 5, int bottom = 5);
    //设置表格外边距
    void setPageMargin(int left = 50, int right = 20, int top = 20, int bottom = 20);
    //设置页面绘制画笔
    void setPen(QPen pen);
    //设置表格头字体
    void setHeadersFont(QFont font);
    //设置表格内容字体
    void setContentFont(QFont font);
    //设置表格头字体颜色
    void setHeaderColor(QColor color);
    //设置表格内容颜色
    void setContentColor(QColor color);
    //设置文本填充方式(超出部分换行或者不换行)
    void setTextWordWrap(Qt::TextFlag Wrap);
    //设置最大行高
    void setMaxRowHeight(int height);
    //设置文本对齐方式
    void setTextAlign(Qt::AlignmentFlag align);
    //设置标题显示方式(只在第一页显示或者每一页都显示标题)
    void setTitleFlag(const TitleFlag type);
    //设置表格头显示方式(只在第一页显示或者每一页都显示)
    void setHeaderFlag(bool flag);

    //获取标题实例
    void setPagerTitle(PageTitle *pagetitle);
    //获取页眉实例
    void setPageHeader(PageHeader *pageheader);
    //获取页脚实例
    void setPagerFooter(PageFooter *pagefooter);
private:
    QPainter *painter;
    QPrinter *printer;
    PageTitle *pageTitle;
    PageHeader *pageHeader;
    PageFooter *pageFooter;
    QTableView *mTableView;
    QAbstractItemModel *model;

    int topMargin;
    int bottomMargin;
    int leftMargin;
    int rightMargin;
    int bottomHeight;
    int maxRowHeight;
    int mheaderRowHeight;

    QPen pen;
    QFont headersFont;
    QFont contentFont;
    QColor headerColor;
    QColor contentColor;
    QStringList mHeaderTitles;
    Qt::TextFlag textLineFlag;
    Qt::AlignmentFlag textAlign;
    TitleFlag titleFlag;
    PrintColumnList mColumnList;
    MergeColumnHash _mergeColumnHash;
    bool mheaderFlag;
    QString error;

private:
    //获取画笔当前位置(测试用)
    void position(const QString& str);
    //解析表格头数据(区分隐藏列,需要合并的列以及计算列宽)
    bool setPrintColumnList(const QStringList &list);
    //主体绘制方法
    bool paintTable(int row=0, int column=0, bool headFlag=false);
};
#endif // QTABLEVIEWPRINTER_H
