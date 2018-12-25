#include "qtableviewprinter.h"
#include <QAbstractItemModel>
#include <QPainter>
#include <QDebug>
#include <QDateTime>

//--------主体内容-------------
int QTableViewPrinter::headerHeight = 100;
int QTableViewPrinter::leftBlank = 40;
int QTableViewPrinter::rightBlank = 40;

QTableViewPrinter::QTableViewPrinter(QPainter* painter, QPrinter* printer) : painter(painter),printer(printer) {
    //设定默认值
    topMargin = 3;
    bottomMargin = 3;
    leftMargin = 3;
    rightMargin = 3;
    bottomHeight = 40;
    maxRowHeight = 1000;
    mheaderRowHeight = 0;
    pen = painter->pen();
    headersFont = QFont("宋体",9);
    contentFont = QFont("宋体",9);
    headerColor = painter->pen().color();
    contentColor = painter->pen().color();
    pageFooter = NULL;
    pageHeader = NULL;
    pageTitle = NULL;
    error = "无错误";
    mheaderFlag = true;
    textLineFlag=Qt::TextFlag::TextSingleLine;
    textAlign=Qt::AlignmentFlag::AlignCenter;
    titleFlag=TitleFlag::EVERY_PAGE;
}

bool QTableViewPrinter::printTable(QTableView *tableView, const QStringList &totalFields, const QStringList &visualFields)
{
    //初始化
    mTableView = tableView;
    mHeaderTitles = totalFields;
    model = tableView->model();
    //错误检查
    if(!printer->isValid()) {
        error = "打印指针有误!";
        return false;
    }
    if(!painter->isActive()) {
        error = "画笔指针有误!";
        return false;
    }
    if(!setPrintColumnList(visualFields))
        return false;
    //开始绘制
    if(!paintTable(0,0,true))
        return false;
    return true;
}

QString QTableViewPrinter::lastError() {
    return error;
}

void QTableViewPrinter::setCellMargin(int left, int right, int top, int bottom) {
    topMargin = top;
    bottomMargin = bottom;
    leftMargin = left;
    rightMargin = right;
}

void QTableViewPrinter::setPageMargin(int left, int right, int top, int bottom) {
    headerHeight = top;
    bottomHeight = bottom;
    leftBlank = left;
    rightBlank = right;
}

void QTableViewPrinter::setPageHeader(PageHeader *pageheader)
{
    pageHeader=pageheader;
}

void QTableViewPrinter::setPagerFooter(PageFooter *pagefooter)
{
    pageFooter = pagefooter;
}

void QTableViewPrinter::setPagerTitle(PageTitle *pagetitle)
{
    pageTitle = pagetitle;
}

void QTableViewPrinter::setPen(QPen p) {
    pen = p;
}

void QTableViewPrinter::setHeadersFont(QFont f) {
    headersFont = f;
}

void QTableViewPrinter::setContentFont(QFont f) {
    contentFont = f;
}

void PageHeader::setPageHeaderFont(QFont font)
{
    pageHeaderFont=font;
}

void PageFooter::setViewFlag(bool zbr, bool ym, bool zbsj)
{
    mzbr = zbr;
    mym = ym;
    mzbsj = zbsj;
}

void QTableViewPrinter::setTextWordWrap(Qt::TextFlag Wrap)
{
    textLineFlag=Wrap;
}

void QTableViewPrinter::setHeaderColor(QColor color) {
    headerColor = color;
}

void QTableViewPrinter::setContentColor(QColor color) {
    contentColor = color;
}

void QTableViewPrinter::setMaxRowHeight(int height) {
    maxRowHeight = height;
}

void QTableViewPrinter::setTextAlign(Qt::AlignmentFlag align)
{
    textAlign=align;
}

void QTableViewPrinter::setTitleFlag(const TitleFlag type)
{
    titleFlag = type;
}

void QTableViewPrinter::setHeaderFlag(bool flag)
{
    mheaderFlag = flag;
}

void QTableViewPrinter::position(const QString &str)
{
    qDebug()<<str<<" x:"<<painter->transform().dx()<<" y:"<<painter->transform().dy();
}

bool QTableViewPrinter::setPrintColumnList(const QStringList &list)
{
    if(list.size()==0 || mHeaderTitles.size()==0 || model->columnCount()==0)
        return false;
    mColumnList.clear();
    //先给行号赋值
    if(!list.contains("行号")){
        PrintColumn *item= new PrintColumn();
        item->name="行号";
        item->modelColumn=-1;
        item->columnWidth=50;
        mColumnList.push_back(item);
    }
    for(int i=0;i<model->columnCount();++i){
        if(list.contains(mHeaderTitles.at(i))){
            PrintColumn *item= new PrintColumn();
            item->name= mHeaderTitles.at(i);
            item->modelColumn = i;
            item->columnWidth = mTableView->columnWidth(i)*1.4*contentFont.pointSize()/12+leftMargin+rightMargin;
            if(item->name.contains("|")){//复杂表头
                QString temp=item->name;
                QStringList tempList=temp.split("|");
                mergeColumn* merge;
                for(int j=0;j<tempList.size();++j){
                    if(_mergeColumnHash[j].size()>0){
                        if(_mergeColumnHash[j].last()->text!=tempList.at(j)){
                            merge = new mergeColumn();
                            merge->text = tempList.at(j);
                            merge->columnList.push_back(QString::number(i));
                            _mergeColumnHash[j].push_back(merge);
                        }else{//需合并的列
                            if(j==0){
                                merge = _mergeColumnHash[j].last();
                                merge->columnList.push_back(QString::number(i));
                            }else{
                                if(_mergeColumnHash[j-1].last()->columnList.contains(QString::number(i)) && _mergeColumnHash[j-1].last()->columnList.contains(_mergeColumnHash[j].last()->columnList.last())){
                                    merge = _mergeColumnHash[j].last();
                                    merge->columnList.push_back(QString::number(i));
                                }else{
                                    merge = new mergeColumn();
                                    merge->text = tempList.at(j);
                                    merge->columnList.push_back(QString::number(i));
                                    _mergeColumnHash[j].push_back(merge);
                                }
                            }
                        }
                    }else{
                        merge = new mergeColumn();
                        merge->text = tempList.at(j);
                        merge->columnList.push_back(QString::number(i));
                        _mergeColumnHash[j].push_back(merge);
                    }
                }
            }
            mColumnList.push_back(item);
        }
    }
    return true;
}

bool QTableViewPrinter::paintTable(int row, int column, bool headFlag)
{
    //获取总行数,列数
    int rowCount = model->rowCount();
    int columnCount=mColumnList.size();
    //界面宽度,高度
    int viewWidth = painter->viewport().width();
    int viewHeight = painter->viewport().height();
    //表格宽度,高度
    int tableWidth = viewWidth - leftBlank - rightBlank;
    int tableHeight = viewHeight - headerHeight - bottomHeight;
    //绘制的最后一行,一列
    int lastRow =0 ,lastColumn = 0;
    //当前页实际总宽度及每一列宽度
    int realTableWidth = 0;
    bool ischild=false;
    QList<int> columnWidth;
    for(int i=column;i<columnCount;++i){
        //此计算为预估
        int width=mColumnList.at(i)->columnWidth;
        if(realTableWidth+width > tableWidth){
            lastColumn = i;
            break;
        }
        columnWidth.push_back(width);
        realTableWidth += width;
    }
    //当前绘制页的总列数
    int curColumnCount = columnWidth.size();
    //是否绘制表格头
    bool curheadflag=headFlag;
    //设置原点
    painter->translate(-painter->transform().dx(),-painter->transform().dy()+headerHeight);//(0,60)
    painter->save();//(0,60)

    //绘制页眉
    if(pageHeader){
        painter->save();
        pageHeader->startDrawHeader();
        painter->restore();
    }
    //绘制标题
    if(pageTitle){
        painter->save();
        pageTitle->startDrawTitle();
        painter->restore();
    }
    //绘制页脚
    if(pageFooter) {
        painter->save();
        painter->translate(-painter->transform().dx(), -painter->transform().dy());
        pageFooter->startDrawFooter();
        painter->restore();
    }

    //定位表格左顶点
    painter->translate(-painter->transform().dx() + leftBlank, 0);
    painter->save();//(40,60)

    painter->setPen(pen);
    //水平顶线
    painter->drawLine(0, 0, realTableWidth, 0);

    //先用一个测试painter获取最大行高
    QPainter testSize;
    QImage* image = new QImage(10, 10, QImage::Format_RGB32);
    image->setDotsPerMeterX(printer->logicalDpiX() * 100 / 2.54); // 2.54 cm = 1 inch
    image->setDotsPerMeterY(printer->logicalDpiY() * 100 / 2.54);
    testSize.begin(image);

    //获取行高
    int i;
    if(curheadflag)
        i = row-1;
    else
        i = row;
    testSize.setFont(headersFont);
    painter->setFont(headersFont);
    int currentPageRowCount=0;
    QHash<int, int> headrowheight;
    for(i; i<rowCount; ++i){
        int maxHeight = 0;
        for(int j=0;j<columnCount;++j){
            if(i==row){
                testSize.setFont(contentFont);
                painter->setFont(contentFont);
            }

            QString str;
            if(i<row){
                str = mColumnList.at(j)->name;
            }else{
                if(mColumnList.at(j)->modelColumn==-1){//行号
                    str = QString::number(i+1);
                }else
                    str = model->data(model->index(i,mColumnList.at(j)->modelColumn), Qt::DisplayRole).toString();
            }

            if(str.contains("|")){
                int height=0;
                QStringList list=str.split("|");
                for(int k=0;k<list.size();++k){
                    QRect rect(0, 0,  mColumnList.at(j)->columnWidth- rightMargin - leftMargin, maxRowHeight);
                    QRect realRect;
                    testSize.drawText(rect, textAlign | textLineFlag, list.at(k), &realRect);
                    if(headrowheight[k]<realRect.height())
                        headrowheight[k]=realRect.height();
                    height += realRect.height()+bottomMargin;
                }
                if(height > maxHeight && mColumnList.at(j)->columnWidth != 0){
                    height > maxRowHeight ? maxHeight = maxRowHeight : maxHeight = height;
                }
            }else{
                QRect rect(0, 0,  mColumnList.at(j)->columnWidth- rightMargin - leftMargin, maxRowHeight);
                QRect realRect;
                testSize.drawText(rect, textAlign | textLineFlag, str, &realRect);
                if (realRect.height() > maxHeight && mColumnList.at(j)->columnWidth != 0) {
                    realRect.height() > maxRowHeight ? maxHeight = maxRowHeight : maxHeight = realRect.height();
                }
            }
        }
        //表格头高度
        if(mheaderRowHeight==0){
            mheaderRowHeight = maxHeight;
        }
        //判断是否新增一页
        double curHeight=painter->transform().dy() + maxHeight + topMargin + bottomMargin;
        double topHeight=painter->viewport().height() -bottomHeight;
        if(curHeight > topHeight){
            lastRow = i;
            //-------绘制当前页垂直线----------
            int y = painter->transform().dy();
            painter->restore();//(40,60)
            painter->save();//(40,60)
            for(int j = column; j < column+curColumnCount; j++) {
                if(j==column){
                    painter->translate(0, -painter->transform().dy()+headerHeight);
                }else{
                    painter->translate(0,-painter->transform().dy()+headerHeight+mheaderRowHeight+topMargin+bottomMargin);
                }
                painter->drawLine(0, 0, 0, - painter->transform().dy() + y);
                painter->translate(mColumnList.at(j)->columnWidth, 0);
            }
            painter->translate(0, -painter->transform().dy()+headerHeight);
            painter->drawLine(0, 0, 0, - painter->transform().dy() + y);
            painter->restore();//(40,60)
            //------结束绘制当前页垂直线---------
            //新建子页
            printer->newPage();
            paintTable(lastRow,column,mheaderFlag);
            painter->save();//(40,60)
            ischild=true;
            break;
        }
        //-------------开始绘制表格数据----------------------
        painter->save(); //(40,60)
        i < row ? painter->setPen(QPen(headerColor)) : painter->setPen(QPen(contentColor));
        curheadflag ? painter->setFont(headersFont) : painter->setFont(contentFont);
        int firstc = mColumnList.at(column)->modelColumn;
        int lastc = mColumnList.at(column+curColumnCount-1)->modelColumn;
        //绘制数据
        for(int j = column; j < column+curColumnCount; j++) {
            QString str;
            if(curheadflag) {
                str = mColumnList.at(j)->name;
            } else {
                if(mColumnList.at(j)->modelColumn==-1){
                    str = QString::number(i+1);
                }else
                    str = model->data(model->index(i,mColumnList.at(j)->modelColumn), Qt::DisplayRole).toString();
            }

            if(str.contains("|")){
                QStringList templist = str.split("|");
                int realHeight=0;
                for(int a=0;a<templist.size();++a){
                    QString temp = templist.at(a);
                    //获取行高
                    realHeight = headrowheight[a];
                    //判断是否需要合并行
                    if(templist.size()<headrowheight.size() && a==templist.size()-1){
                        for(int b=a+1;b<headrowheight.size();++b){
                            realHeight += (headrowheight[b]+topMargin+bottomMargin);
                        }
                    }
                    //从容器取值, 查找是否存在当前列---
                    //当前列
                    int curc = mColumnList.at(j)->modelColumn;
                    int width=0;
                    //查找当前列, 计算列宽
                    for(int b=0;b<_mergeColumnHash[a].size();++b){
                        //取容器列
                        mergeColumn* item = _mergeColumnHash[a].at(b);
                        //判断容器列是否存在当前列
                        if(item->columnList.contains(QString::number(curc))){
                            //循环容器列, 找到列位置, 获取列宽
                            if(item->columnList.at(0).toInt()==curc || curc==firstc){
                                int d=0;
                                for(int c=0;c<item->columnList.size();++c){
                                    //要求容器列第一个列数值等于当前列或者容器列或者当前页第一列值在容器列中间
                                    if(item->columnList.at(c).toInt()<=lastc && item->columnList.at(c).toInt()>=curc){
                                        width += mColumnList.at(j+d)->columnWidth;
                                        d++;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    //判断列宽
                    if(width==0){//当前列所在的标题已经绘制过了, 无需再绘制
                        if(a==templist.size()-1){//换列
                            painter->translate(mColumnList.at(j)->columnWidth, -painter->transform().dy()+headerHeight);
                        }else{//不绘制换行
                            painter->translate(0, realHeight+bottomMargin+topMargin);
                        }
                    }else{//准备绘制
                        QRect rec(leftMargin, topMargin, width - rightMargin - leftMargin, realHeight);
                        painter->drawText(rec, textAlign | textLineFlag, temp);
                        //绘制横线
                        if(a!=templist.size()-1){//最后一行无需绘制
                            painter->drawLine(0,realHeight+bottomMargin+topMargin,width,realHeight+bottomMargin+topMargin);
                        }
                        //绘制竖线
                        painter->drawLine(width,0,width,realHeight+bottomMargin+topMargin);

                        if(a==templist.size()-1){//换列
                            painter->translate(mColumnList.at(j)->columnWidth, -painter->transform().dy()+headerHeight);
                        }else{//换行
                            painter->translate(0, realHeight+bottomMargin+topMargin);
                        }
                    }
                }
            }else{
                QRect rec(leftMargin, topMargin, mColumnList.at(j)->columnWidth - rightMargin - leftMargin, maxHeight);
                painter->drawText(rec, textAlign | textLineFlag, str);
                //绘制竖线
                painter->drawLine(mColumnList.at(j)->columnWidth,0,mColumnList.at(j)->columnWidth,maxHeight+bottomMargin+topMargin);
                painter->translate(mColumnList.at(j)->columnWidth, 0);
            }
        }
        if(curheadflag){
            curheadflag = false;
        }
        painter->restore(); //(40,60)
        //--------------结束绘制表格数据-----------------------
        //绘制当前行横线
        painter->drawLine(0, maxHeight + topMargin + bottomMargin, realTableWidth,maxHeight + topMargin + bottomMargin);
        //更新坐标
        painter->translate(0, maxHeight + topMargin + bottomMargin);
        currentPageRowCount++;
    }
    //-----------绘制垂直线--------------------
    int y = painter->transform().dy();
    painter->restore();//(40,60)
    if(!ischild){ //垂直线已在子页中绘制过, 无需重复再绘制
        for(int i = column; i < column+curColumnCount; i++) {
            if(i==column){
                painter->translate(0, -painter->transform().dy()+headerHeight);
            }else{
                painter->translate(0,-painter->transform().dy()+headerHeight+mheaderRowHeight+topMargin+bottomMargin);
            }
            painter->drawLine(0, 0, 0,- painter->transform().dy() + y);
            painter->translate(mColumnList.at(i)->columnWidth, 0);
        }
        painter->translate(0, -painter->transform().dy()+headerHeight);
        painter->drawLine(0, 0, 0, - painter->transform().dy() + y);
    }
    //-----------垂直线绘制完毕----------------
    testSize.end();
    delete image;

    //恢复原点(0,60)
    painter->restore();

    //分页(只有第一页的数据允许分页)
    if(column+curColumnCount<columnCount && row==0){
        printer->newPage();
        paintTable(row,lastColumn,mheaderFlag);
    }
    return true;
}
//--------主体内容结束-------------

//--------页眉类----------------
PageHeader::PageHeader(QPainter *painter)
{
    mpainter = painter;
    pageHeaderFont = QFont("Microsoft YaHei",9);
}

void PageHeader::startDrawHeader()
{

}
//--------页眉类结束-------------


//--------页脚类-------------
int PageFooter::pageNumber=1;

PageFooter::PageFooter(QPainter *painter)
{
    mpainter = painter;
    creater = "无";
    pageFooterFont = QFont("宋体",9);
    mzbr = true;
    mzbsj = true;
    mym = true;
}

void PageFooter::setPageFooterFont(QFont font)
{
    pageFooterFont=font;
}

void PageFooter::resetPageNumber()
{
    pageNumber=1;
}

void PageFooter::startDrawFooter()
{
    //保存当前字体
    QFont font=mpainter->font();
    //设置页脚字体
    mpainter->setFont(pageFooterFont);
    //绘制制表人
    if(mzbr){
        mpainter->translate(-mpainter->transform().dx()+40, -mpainter->transform().dy()+mpainter->viewport().height() - 20);
        mpainter->drawText(0, 0, QString("制表人: %1").arg(creater));
    }
    //绘制页码
    if(mym){
        mpainter->translate(-mpainter->transform().dx()+mpainter->viewport().width()/2-15, -mpainter->transform().dy()+mpainter->viewport().height() - 20);
        mpainter->drawText(0, 0, QString("第 %1 页").arg(pageNumber));
        pageNumber += 1;
    }
    //绘制制表日期
    if(mzbsj){
        mpainter->translate(-mpainter->transform().dx()+mpainter->viewport().width()-170, -mpainter->transform().dy()+mpainter->viewport().height() - 20);
        mpainter->drawText(0, 0, QString("制表日期: %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
    }
    //恢复之前字体
    mpainter->setFont(font);
}

void PageFooter::setCreater(const QString &name)
{
    creater=name;
}

//--------页脚类结束-------------

//--------标题类-------------
PageTitle::PageTitle(QPainter *painter, QPrinter* printer)
{
    mpainter=painter;
    mprinter = printer;
    QFont font = QFont("宋体",20);
    QFont sidefont = QFont("宋体",9);
    font.setBold(true);
    titleFont=font;
    sidetitleFont = sidefont;
}

void PageTitle::setTitleFont(QFont font)
{
    titleFont=font;
}

void PageTitle::setSideTitleFont(QFont font)
{
    sidetitleFont=font;
}

void PageTitle::setPageTitle(const QString &title)
{
    mtitle=title;
}

void PageTitle::setSideTitle(const QStringList &sidetitle)
{
    msideTitles = sidetitle;
}

void PageTitle::startDrawTitle()
{
    //回到原点
    mpainter->translate(-mpainter->transform().dx(),-mpainter->transform().dy());
    //设置字体
    QFont font=mpainter->font();
    mpainter->setFont(titleFont);
    //获取标题宽度
    QPainter testSize;
    QImage* image = new QImage(10, 10, QImage::Format_RGB32);
    image->setDotsPerMeterX(mprinter->logicalDpiX() * 100 / 2.54); // 2.54 cm = 1 inch
    image->setDotsPerMeterY(mprinter->logicalDpiY() * 100 / 2.54);
    testSize.begin(image);
    testSize.setFont(titleFont);
    QRect rect(0, 0,  1000, 1000);
    QRect realRect;
    testSize.drawText(rect, Qt::AlignmentFlag::AlignCenter | Qt::TextFlag::TextSingleLine, mtitle, &realRect);
    int left= (mpainter->viewport().width()-realRect.width())*0.5;
    //绘制标题
    mpainter->drawText(left,QTableViewPrinter::headerHeight/2+realRect.height()/2,mtitle);
    //绘制副标题(均布绘制在表格上方, 标题下方)
    if(msideTitles.size()>0){
        mpainter->setFont(sidetitleFont);
        testSize.setFont(sidetitleFont);
        QList<int> widthList;
        int totalwidth=0;
        int height =0;
        for(int i=0;i<msideTitles.size();++i){
            testSize.drawText(rect, Qt::AlignmentFlag::AlignCenter | Qt::TextFlag::TextSingleLine, msideTitles.at(i), &realRect);
            widthList.push_back(realRect.width());
            totalwidth+=realRect.width();
            if(i==0)
                height = realRect.height();
        }
        int midWidth = 0;
        if(msideTitles.size()>1)
            midWidth=(mpainter->viewport().width()-(QTableViewPrinter::leftBlank+QTableViewPrinter::rightBlank+totalwidth))/(msideTitles.size()-1);
        for(int i=0;i<msideTitles.size();++i){
            int tempWidth=0;
            for(int j=0;j<i;++j){
                tempWidth+=widthList.at(j)+midWidth;
            }
            mpainter->drawText(QTableViewPrinter::leftBlank+tempWidth,QTableViewPrinter::headerHeight-height/2,msideTitles.at(i));
        }
    }
    mpainter->setFont(font);
}
//--------标题类结束-------------
