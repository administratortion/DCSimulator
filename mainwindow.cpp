#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "allwidget.h"
#include "wire.h"
#include "mainwindowsetcomponentparameter.h"
#include "DrawPlot/mainwindowdraw.h"

#include <QLabel>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QLineEdit>
#include <QInputDialog>
#include <QProcess>
#include <QLineEdit>
#include <QFileDialog>
#include <QDateTime>
#include <QStack>
#include <QScrollArea>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    listLineNet=creatNet(netStartPoint,netEndPoint,widthOfNet);
    //    QColor color = QColor(Qt::white);
    //    QPalette p = this->palette();
    //    p.setColor(QPalette::Window,color);
    //    this->setPalette(p);// 设置背景色为白色，用样式表设置方式的话，我画直线就显示不出来

    m_allWidget = new AllWidget(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);  // 注：由于主窗体表面有个centralWidget，所以得这样设置两个鼠标跟踪
    setAcceptDrops(true);                      // 设置窗口部件可以接收拖入

    pointMenuBar = QPoint(0,ui->menubar->height());  // 因为经过反复测试发现菜单栏会占据一个高度，但是绘图没有体现出来
    int cntTmp=0;
    m_allWidget->addWidget("R",ui->centralwidget,QPoint(0,20));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "R";
    m_allWidget->addWidget("C",ui->centralwidget,QPoint(0,100));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "C";
    m_allWidget->addWidget("FC",ui->centralwidget,QPoint(0,180));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "FC";
    m_allWidget->addWidget("E",ui->centralwidget,QPoint(0,260));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "E";
    m_allWidget->addWidget("I",ui->centralwidget,QPoint(0,340));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "I";
    m_allWidget->addWidget("L",ui->centralwidget,QPoint(0,420));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "L";
    m_allWidget->addWidget("FL",ui->centralwidget,QPoint(0,500));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "FL";
    m_allWidget->addWidget("T",ui->centralwidget,QPoint(120,20));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "T";


    m_allWidget->addWidget("CS",ui->centralwidget,QPoint(120,100));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "CS";

    m_allWidget->addWidget("VS",ui->centralwidget,QPoint(120,180));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "VS";

    m_allWidget->addWidget("K",ui->centralwidget,QPoint(120,260));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "K";//大写的K

    m_allWidget->addWidget("Scope",ui->centralwidget,QPoint(120,340));
    m_allWidget->arrayNameLeftComponent[cntTmp++] = "Scope";


    m_allWidget->numComponentExistLeft = m_allWidget->numComponent;
    QString widNameTmp=m_allWidget->listStructWidget.at(0)->currWidgObjName;
    QWidget *childTmp = this->findChild<QWidget *>(widNameTmp);
    //    childTmp->setWindowOpacity(0.5);

    //    ui->pushButton->setWindowOpacity(0.5);
    //    ui->pushButton_2->setWindowOpacity(0.5);
    //    ui->widgetTm->setWindowOpacity(0.5);
}
QWidget *MainWindow::getUiCentralWidget()
{
    return ui->centralwidget;
}
QMenuBar *MainWindow::getMenuBar()
{
    return ui->menubar;
}

void MainWindow::mousePressEvent(QMouseEvent *event)   // 鼠标按下事件,注：左键点击了连线标签，不能马上进行update()
{                                                      // 操作，因为这时候直线绘制的终点坐标还没有确定，系统默认为0,0
    if(event->button() == Qt::LeftButton)              // 所以会出现一瞬间绘制到原点的直线的情况
    {

        QPoint posMouse = event->pos();
        QWidget *child = static_cast<QWidget*>(childAt(posMouse));
        // 第二步：自定义MIME类型
        QByteArray itemData;                                     // 创建字节数组
        QDataStream dataStream(&itemData, QIODevice::WriteOnly); // 创建数据流
        // 将图片信息，位置信息输入到字节数组中
        // 说明移动的时候点击的不是元件的空白处（widget），而是标签了，但是我们传过去坐标一定得是
        // 元件空白处（widget）的坐标才行，因为后面就是用这个坐标来移动元件的，不然就是出现坐标偏差问题（我是调试了好久才发现这个问题的！！！)
        QWidget *childTmp;//这个变量的目的是不想影响child的值
        if(m_allWidget->isLabelLogoOrName(child->objectName()) == true)
        {
            QLabel *lab = this->findChild<QLabel *>(child->objectName());
            childTmp = lab->parentWidget();    // change to label's parent widget,because need move widget
        }
        else
        {
            childTmp = this->findChild<QWidget *>(child->objectName());
        }// 现在childTmp已经是元件空白处（widget）的指针了，而child还是原来的指针
        QPoint positionWidgetComponent = childTmp->pos();//////////这个地方有问题！！！
        QPoint offset = m_allWidget->getOffset(posMouse);
        // 控件的左上角位置，offset：鼠标和左上角距离差
        QString childName = child->objectName();
        dataStream << positionWidgetComponent<<offset<<childName;  // 注：这里只能首先输入坐标数据，因为后面也是按照顺序拿的数据
        //传过去的东西如下：拖动元件的widget坐标，鼠标位置和元件的widget坐标差，拖动时候点击的对象名（可能是标签的对象名）
        // 还有个getIdWidget(childName)
        /***************************************************************************/
        /* 单击了控件内部任何位置 */
        /***************************************************************************/
        if(m_allWidget->isWidgetInnerOrOuter(posMouse) == true)    //
        {
            /***************************************************************************/
            /* 左侧的控件 */
            /***************************************************************************/
            if(m_allWidget->isWidgetRight(posMouse) == false)  //
            {

                actionMoveOrCopy = Qt::CopyAction;
                dataStream<<m_allWidget->getIdWidget(childName);   // if
                // 第三步：将数据放入QMimeData中
                QMimeData *mimeData = new QMimeData;  // 创建QMimeData用来存放要移动的数据
                // 将字节数组放入QMimeData中，这里的MIME类型是我们自己定义的
                mimeData->setData("myimage/png", itemData);

                // 第四步：将QMimeData数据放入QDrag中
                QDrag *drag = new QDrag(this);      // 创建QDrag，它用来移动数据
                drag->setMimeData(mimeData);
                drag->setHotSpot(posMouse - child->pos()); // 拖动时鼠标指针的位置不变
                // 第六步：执行拖放操作   这里我把它改为复制操作了
                if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction)== Qt::MoveAction)  // 设置拖放可以是移动和复制操作，默认是复制操作
                {
                    // child->close();         // 如果是移动操作，那么拖放完成后关闭原标签
                }
                else
                {
                    child->show();            // 如果是复制操作，那么拖放完成后显示标签
                }

            }
            /***************************************************************************/
            /* 右侧控件 */
            /***************************************************************************/
            else if(m_allWidget->isWidgetRight(posMouse) == true) //
            {
                /***************************************************************************/
                /* 点击的是控件上面的端口标签位置 */
                /***************************************************************************/
                if(m_allWidget->isWidgetInnerOrOuter(posMouse) == true&&m_allWidget->isPortLabel(posMouse) == true)
                {
                    //之前没有连端口且直线,现在连了端口
                    if((m_allWidget->lastObjectName=="")&&(m_allWidget->lastLineClicked==-1))
                    {
                        m_allWidget->lastObjectName = childName;
                        QPoint posTmp1 = child->parentWidget()->pos();
                        QPoint posTmp2 = child->pos()+QPoint(child->width()/2,child->height()/2);//线的起点是端口的中心位置
                        QPoint posTmp = posTmp1+posTmp2;
                        qDebug()<<"点击的端口名称"<<child->objectName()<<"\n";
                        lastMousePos = posTmp;
                        posMouseCurrent = lastMousePos; // attention: here must set posMouseCurrent = lastMousePos,if not, will a line exist
                        flagMouseLineDrawing = true;    // 鼠标动态画线开始
                    }
                    qDebug()<<"m_allWidget->lastObjectName:"<<m_allWidget->lastObjectName<<"\n";
                    //之前连了端口或者直线,现在连了端口
                    if((m_allWidget->lastObjectName!= child->objectName())||(m_allWidget->lastLineClicked!=-1))    // 线连接成功，需要绘制直线了
                    {
                        QWidget *wid2;
                        QLabel *label2 = this->findChild<QLabel *>(child->objectName());
                        QString label2Name=label2->objectName();
                        qDebug()<<"label2Name:"<<label2Name<<"\n";

                        wid2 = label2->parentWidget();
                        QPoint pointEndTmp = label2->pos()+QPoint(label2->width()/2,label2->height()/2)+wid2->pos();

                        flagMouseLineDrawing = false;   // 鼠标动态画线结束
                        qDebug()<<pointEndTmp<<"\n";


                        structLine*tmpLine = new structLine;
                        tmpLine->currLineName = "line_"
                                + QString::number(m_allWidget->cntAddLine);
                        tmpLine->pointStart = lastMousePos;
                        tmpLine->pointEnd = pointEndTmp ;
                        // 直线的起点和终点一定得刚好在端口的左上角，这样最后的搜索才准确
                        m_allWidget->cntAddLine++;
                        m_allWidget->listStructLine.append(tmpLine);  // 记录直线

                        update();
                        // 等到算法连接成功后，才更新界面连线
                        m_allWidget->lastObjectName = "";
                        m_allWidget->lastLineClicked=-1;
                    }
                }
                /***************************************************************************/
                /* 点击的是控件的空位置（不是任何标签和编辑框的位置） */
                /***************************************************************************/
                else if(m_allWidget->isBlankOfWidget(posMouse) ==true || m_allWidget->isLabelLogoOrName(posMouse) == true)      //
                {
                    qDebug()<<"单击了控件"<<"\n";
                    //说明鼠标left键在控件位置,select this widget
                    // child here should is widget not label,so we should judge it whether switch to widget

                    if(m_allWidget->isLabelLogoOrName(child->objectName()) == true)
                    {
                        QLabel *labelLogo = this->findChild<QLabel *>(child->objectName());
                        child = labelLogo->parentWidget();    // change to label's parent widget,because need move widget
                    }
                    else
                    {
                        child = this->findChild<QWidget *>(child->objectName());
                    }

                    QList<int> portListTmp=m_allWidget->findPortListByWidget(child->objectName());
                    for (int j = 0; j < portListTmp.size(); ++j) {
                        //                        qDebug()<<"此端口坐标"<<
                        int indexTmp=portListTmp.at(j);
                        if(m_allWidget->atWhichLine(m_allWidget->listStructPort.at(indexTmp)->posPort)>=0)
                        {
                            qDebug()<<"单击了控件,但是此控件有线连着，所有不能移动，本函数结束"<<"\n";
                            QMessageBox::warning(this,"警告","元件连着导线，所以不能拖动",QMessageBox::Abort);

                            return;// 这样写法还是不错的
                        }
                    }//单击了控件,发现此元件的端口中有个是有线连着，所有不能移动此元件，本函数结束，停止继续往下运行


                    int w = -1;
                    for (int i = 0; i < m_allWidget->listStructWidget.size(); ++i) {
                        if(m_allWidget->listStructWidget.at(i)->currWidgObjName == child->objectName())
                        {
                            w=i;
                            break;
                        }
                    }
                    /**********************************************************************/

                    actionMoveOrCopy = Qt::MoveAction;
                    dataStream<<m_allWidget->getIdWidget(childName);   // if
                    // 第三步：将数据放入QMimeData中
                    QMimeData *mimeData = new QMimeData;  // 创建QMimeData用来存放要移动的数据
                    // 将字节数组放入QMimeData中，这里的MIME类型是我们自己定义的
                    mimeData->setData("myimage/png", itemData);

                    // 第四步：将QMimeData数据放入QDrag中
                    QDrag *drag = new QDrag(this);      // 创建QDrag，它用来移动数据
                    drag->setMimeData(mimeData);
                    drag->setHotSpot(posMouse - child->pos()); // 拖动时鼠标指针的位置不变
                    // 第六步：执行拖放操作   这里我把它改为复制操作了
                    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction)== Qt::MoveAction)  // 设置拖放可以是移动和复制操作，默认是复制操作
                    {
                        // child->close();         // 如果是移动操作，那么拖放完成后关闭原标签
                    }
                    else
                    {
                        child->show();            // 如果是复制操作，那么拖放完成后显示标签
                    }

                }
            }
        }
        /***************************************************************************/
        /* 控件外部 */
        /***************************************************************************/
        else if(m_allWidget->isWidgetInnerOrOuter(posMouse) == false)   //
        {
            qDebug()<<"单击了空白"<<"\n";

            /***************************************************************************/
            /* 单击在直线上 */
            /***************************************************************************/
            if(m_allWidget->isAtLine(posMouse) == true)   //
            {
                //前面没有连接端口和直线,现在点了直线
                if((m_allWidget->lastLineClicked==-1)&&(m_allWidget->lastObjectName==""))//点击在直线上，重新开始新的一条直线的条件是，之前没有点击端口且直线
                {
                    m_allWidget->lastLineClicked = m_allWidget->atWhichLine(posMouse);
                    lastMousePos = getNearestPointOfNet(posMouse);
                    posMouseCurrent = lastMousePos; // attention: here must set posMouseCurrent = lastMousePos,if not, will a line exist
                    flagMouseLineDrawing = true;    // 鼠标动态画线开始
                }
                //前面连了端口或者直线，现在点了直线
                else if((m_allWidget->atWhichLine(posMouse)!=m_allWidget->lastLineClicked)||(m_allWidget->lastObjectName!=""))
                {
                    flagMouseLineDrawing = false;   // 鼠标动态画线结束
                    structLine*tmpLine = new structLine;
                    tmpLine->currLineName = "line_" + QString::number(m_allWidget->cntAddLine);
                    tmpLine->pointStart = lastMousePos;
                    tmpLine->pointEnd = getNearestPointOfNet(posMouse) ;
                    // 直线的起点和终点一定得刚好在端口的左上角，这样最后的搜索才准确
                    m_allWidget->cntAddLine++;
                    m_allWidget->listStructLine.append(tmpLine);  // 记录直线
                    update();
                    // 等到算法连接成功后，才更新界面连线
                    m_allWidget->lastLineClicked=-1;
                    m_allWidget->lastObjectName="";
                }
            }
            /***************************************************************************/
            /* 单击在直线外 */
            /***************************************************************************/
            else if(m_allWidget->isAtLine(posMouse) == false)  //
            {
                if(m_allWidget->lastObjectName!= ""||m_allWidget->lastLineClicked!=-1)    // 说明端口或者直线上正在引出了线，需要绘制直线了
                {
                    if(m_allWidget->isRoundOfPointP1(posMouseCurrent,posMouse,5))//说明此时鼠标点了出来的线的头头，这样可以让这条线存在,即添加直线
                    {
                        structLine*tmpLine = new structLine;
                        tmpLine->currLineName = "line_" + QString::number(m_allWidget->cntAddLine);
                        tmpLine->pointStart = lastMousePos;
                        tmpLine->pointEnd = posMouseCurrent ;
                        // 直线的起点和终点一定得刚好在端口的左上角，这样最后的搜索才准确
                        m_allWidget->cntAddLine++;
                        m_allWidget->listStructLine.append(tmpLine);  // 记录直线
                        // 等到算法连接成功后，才更新界面连线
                    }

                    flagMouseLineDrawing =false;
                    m_allWidget->lastObjectName= "";
                    m_allWidget->lastLineClicked=-1;
                    update();
                }


            }
        }
        /*----------------------------@ 这是一个分隔符 @-----------------------------*/
        /***************************************************************************/
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // 如果有我们定义的MIME类型数据，则进行移动操作
    if (event->mimeData()->hasFormat("myimage/png")) {
        event->setDropAction(actionMoveOrCopy);
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("myimage/png")) {
        event->setDropAction(actionMoveOrCopy);
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("myimage/png")) {

        QByteArray itemData = event->mimeData()->data("myimage/png");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPoint offset,positionLastOfWidgetComponent;//这是被拖到元件原来的底widget的坐标
        QString childName;
        QWidget *child;
        QLabel *labelLogo;
        int i = -1;
        // 使用数据流将字节数组中的数据读入到QPixmap和QPoint变量中
        dataStream >> positionLastOfWidgetComponent>>offset>>childName>>i;
        qDebug()<<"childName:"<<childName<<"\n";
        // child here should is widget not label,so we should judge it whether switch to widget
        if(m_allWidget->isLabelLogoOrName(childName) == true)
        {
            labelLogo = this->findChild<QLabel *>(childName);
            child = labelLogo->parentWidget();    // change to label's parent widget,because need move widget
        }
        else
        {
            child = this->findChild<QWidget *>(childName);
        }
        QPoint positionCurrent = event->pos() - offset;
        positionCurrent=getNearestPointOfNet(positionCurrent);//只能停靠在网格上
        if(positionCurrent == positionLastOfWidgetComponent)
        {
            qDebug()<<"鼠标上次位置和这次位置一样，不能拖动，返回"<<"\n";
            return;
        }
        QString operat = childName;
        if(operat.contains("_"))
        {
            int index = operat.indexOf("_");
            operat.truncate(index);        // 由于operate是add_8这样的，这里需要开始调用toolbox了，需要去掉_8
        }
        if(m_allWidget->isComponentExistLeft(childName)) // childName已经是去掉后缀id的名字了
        {
            if(i>=m_allWidget->numComponentExistLeft)   // 如果是移动的话，这里的i是被移动的控件的id，会大于左边候选控件的个数的
            {
                child->move(positionCurrent.x(),positionCurrent.y());   // 先移动控件，再移动对应链接的直线
                m_allWidget->listStructWidget.at(m_allWidget->getIndexFromListStructWidget(child->objectName()))->point=QPoint(positionCurrent.x(),positionCurrent.y());
                QList<int> portListTmp=m_allWidget->findPortListByWidget(child->objectName());
                for (int j = 0; j < portListTmp.size(); ++j) {
                    m_allWidget->listStructPort.at(portListTmp.at(j))->posPort=m_allWidget->listStructPort.at(portListTmp.at(j))->posPort+positionCurrent-positionLastOfWidgetComponent;
                }//更新完毕在listStructPort中此元件的所有端口的坐标
                child->show();
                update();

            }
            else//是拖动也就是复制左边的元件的话
            {
                m_allWidget->addWidget(operat,ui->centralwidget,positionCurrent);
                update();
            }

        }
        qDebug()<<actionMoveOrCopy<<"\n";
        event->setDropAction(actionMoveOrCopy);
        event->accept();
    }

    else
    {
        event->ignore();
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if((m_allWidget->lastObjectName != "")||(m_allWidget->lastLineClicked!=-1))
    {
        if(abs(event->pos().x()-lastMousePos.x())>=abs(event->pos().y()-lastMousePos.y()))//鼠标在在右侧
        {
            posMouseCurrent=getNearestPointOfNet(QPoint(event->pos().x(),lastMousePos.y()));
        }
        else
        {
            posMouseCurrent=getNearestPointOfNet(QPoint(lastMousePos.x(),event->pos().y()));
        }
        qDebug()<<"posMouseCurrent"<<posMouseCurrent<<"event->pos()"<<event->pos()<<"\n";
        update();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{

}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QPoint posMouse = event->pos() ;    // 鼠标当前坐标
    QMenu *menuLine = new QMenu(this);

    if(m_allWidget->isBlankOfWidget(posMouse) ==true || m_allWidget->isLabelLogoOrName(posMouse) == true)      //
    {
        QWidget *child = static_cast<QWidget*>(childAt(event->pos()));
        // child here should is widget not label,so we should judge it whether switch to widget
        if(m_allWidget->isLabelLogoOrName(child->objectName()) == true)
        {
            QLabel *labelLogo = this->findChild<QLabel *>(child->objectName());
            child = labelLogo->parentWidget();    // change to label's parent widget,because need move widget
        }
        else
        {
            child = this->findChild<QWidget *>(child->objectName());
        }
        int w = -1;
        for (int i = 0; i < m_allWidget->listStructWidget.size(); ++i) {
            if(m_allWidget->listStructWidget.at(i)->currWidgObjName == child->objectName())
            {
                w=i;
                break;
            }
        }
        tmpWidgetDelet=child->objectName();
        menuLine->addAction(ui->actionDelet);//删除元件的
        //说明元件还未旋转过，而且不是示波器元件（示波器不支持旋转，太丑了），则可以进行旋转
        if((m_allWidget->listStructWidget.at(w)->state==0) && (!m_allWidget->listStructWidget.at(w)->name.contains("Scope")))
            menuLine->addAction(ui->actionRotateLeft90);

        tmpIndexSetParameter=w;
        menuLine->addAction(ui->actionSetParameter);
        if(m_allWidget->listStructWidget.at(w)->name.contains("Scope"))
            menuLine->addAction(ui->actionOpenWindow);

        menuLine->move(cursor().pos());
        menuLine->show();
    }
    else if(m_allWidget->atWhichNode(posMouse)>=0)
    {
        tmpNodeDelet=m_allWidget->listStructNode.at(m_allWidget->atWhichNode(posMouse))->currNodeObjName;
        menuLine->addAction(ui->actionEditNode);
        menuLine->addAction(ui->actionDeletNode);//删除节点的
        menuLine->move(cursor().pos());
        menuLine->show();
    }
    else if(m_allWidget->isAtLine(posMouse))//右键在直线上，说明想添加节点
    {
        tmpPosAddNode=posMouse;
        tmpLineDelet=m_allWidget->listStructLine.at(m_allWidget->atWhichLine(posMouse))->currLineName;
        menuLine->addAction(ui->actionDeletLine);//删除直线的
        menuLine->addAction(ui->actionAddNode);
        menuLine->move(cursor().pos());
        menuLine->show();
    }

}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if(flagTurnOnNet)
        paintAllLineNet(listLineNet);
    paintAllLine(m_allWidget->listStructLine);

    if(flagMouseLineDrawing == true)
    {
        if((m_allWidget->lastObjectName != "")||(m_allWidget->lastLineClicked!=-1))
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing,true);
            QPen pen = painter.pen();
            pen.setWidth(m_allWidget->lineWidth);
            painter.setPen(pen);
            painter.drawLine(lastMousePos,posMouseCurrent);    // 后面为什么不用加上菜单栏高度呢，因为这个是用实时鼠标坐标，是正确的
        }
    }
}
void MainWindow::paintAllLine(QList <structLine*> listStructLine)
{
    QPainter painterPara(this);
    painterPara.setRenderHint(QPainter::Antialiasing,true);
    QPen pen = painterPara.pen();
    pen.setWidth(m_allWidget->lineWidth);

    for (int i = 0; i < listStructLine.size(); ++i) {
        pen.setColor(listStructLine.at(i)->lineColor);
        painterPara.setPen(pen);
        painterPara.drawLine(listStructLine.at(i)->pointStart,listStructLine.at(i)->pointEnd);
    }
}

void MainWindow::paintAllLineNet(QList<structLineNet> listStructLineNet)
{
    QPainter painterPara(this);
    painterPara.setRenderHint(QPainter::Antialiasing,true);
    QPen pen = painterPara.pen();
    pen.setWidth(2);

    for (int i = 0; i < listStructLineNet.size(); ++i) {
        pen.setColor(QColor(220,220,220));
        painterPara.setPen(pen);
        painterPara.drawLine(listStructLineNet.at(i).pointStart,listStructLineNet.at(i).pointEnd);
    }
}

QPoint MainWindow::getNearestPointOfNet(QPoint pp)
{
    QPoint res;
    int n1=(pp.y()-netStartPoint.y())/widthOfNet;
    int v=netStartPoint.y()+n1*widthOfNet;//得到鼠标y坐标的上面一条线的那个y
    int c=pp.y()-v;
    if(c<=(widthOfNet/2))
        res.setY(v);//如果鼠标的y更加靠近上面一条直线的y，那就选这条直线的y坐标作为停靠坐标
    else
        res.setY(netStartPoint.y()+(n1+1)*widthOfNet);//如果鼠标的y更加靠近下面一条直线的y，那就选这条直线的y坐标作为停靠坐标
    //现在设置x停靠
    n1=(pp.x()-netStartPoint.x())/widthOfNet;
    v=netStartPoint.x()+n1*widthOfNet;//得到鼠标y坐标的上面一条线的那个y
    c=pp.x()-v;
    if(c<=(widthOfNet/2))
        res.setX(v);//如果鼠标的y更加靠近上面一条直线的y，那就选这条直线的y坐标作为停靠坐标
    else
        res.setX(netStartPoint.x()+(n1+1)*widthOfNet);//如果鼠标的y更加靠近下面一条直线的y，那就选这条直线的y坐标作为停靠坐标
    return res;
}

void MainWindow::_DFS(QList<structLine *> listStructLineTmp, int n)
{
    visited[n]=true;
    for (int i = 0; i < listStructLineTmp.size(); ++i) {
        if((visited[i]==false)&&way(listStructLineTmp,n,i))//必须是未访问过的，不然会一直循环
            _DFS(listStructLineTmp,i);
    }
}

QList<int> MainWindow::DFS(QList<structLine *> listStructLineTmp, int n)
{
    for (int i = 0; i < listStructLineTmp.size(); ++i) {
        visited[i]=false;//得清空这个全局数组才行，不然后面的节点的遍历会受影响
    }
    _DFS(listStructLineTmp,n);
    QList<int> lines;
    for (int i = 0; i < listStructLineTmp.size(); ++i) {
        if(visited[i]==true)
            lines.append(i);
    }
    return lines;
}

bool MainWindow::way(QList<structLine *> listStructLineTmp, int i, int j)
{
    if(i==j)
        return false;
    else if(m_allWidget->atWhichLine(listStructLineTmp.at(i)->pointStart)==j)
        return true;
    else if(m_allWidget->atWhichLine(listStructLineTmp.at(i)->pointEnd)==j)
        return true;
    else if(m_allWidget->atWhichLine(listStructLineTmp.at(j)->pointStart)==i)
        return true;
    else if(m_allWidget->atWhichLine(listStructLineTmp.at(j)->pointEnd)==i)
        return true;
    else
        return false;
}

void MainWindow::setComponentPatameterByOtherMainwindow(structWidget *tmp)
{
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->name=tmp->name;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->value=tmp->value;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->starValue=tmp->starValue;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->nOrder=tmp->nOrder;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->fz=tmp->fz;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->fre=tmp->fre;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->cxw=tmp->cxw;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->NADD=tmp->NADD;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->NCUT=tmp->NCUT;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->k1=tmp->k1;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->G=tmp->G;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->orderLeft=tmp->orderLeft;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->orderRight=tmp->orderRight;
    m_allWidget->listStructWidget.at(tmpIndexSetParameter)->hgxs=tmp->hgxs;





    delete tmp;//记得释放掉不用的对象，否则内存泄漏，好像我前面的list的使用不知道是不是也得释放，不然造成内存不足
    //根据备选元件的对象名找到此widget的地址，然后找到它包含的子控件中的name标签即可
    QWidget *wid = findChild<QWidget *>(m_allWidget->listStructWidget.at(tmpIndexSetParameter)->currWidgObjName);
    QList<QLabel *> lab = wid->findChildren<QLabel*>();
    for (int i = 0; i < lab.size(); ++i) {
        if(lab.at(i)->objectName().contains("name"))//说明找到了元件中的name标签，更新他的显示了
            lab.at(i)->setText(m_allWidget->listStructWidget.at(tmpIndexSetParameter)->name);
    }
}

QList<structLineNet> MainWindow::creatNet(QPoint p1, QPoint p2, int widt)
{
    QList<structLineNet> res;
    int n1=(p2.x()-p1.x())/widt+1;//有多少条竖线
    int n2=(p2.y()-p1.y())/widt+1;//有多少横线
    structLineNet net;
    for (int i = 0; i < n1; ++i) {//先画竖线
        net.pointStart=QPoint(p1.x()+i*widt,p1.y());
        net.pointEnd=QPoint(p1.x()+i*widt,p2.y());
        res.append(net);
    }
    for (int i = 0; i < n2; ++i) {//画横线
        net.pointStart=QPoint(p1.x(),p1.y()+i*widt);
        net.pointEnd=QPoint(p2.x(),p1.y()+i*widt);
        res.append(net);
    }
    return res;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    for (int i = 0; i < m_allWidget->listStructNode.size(); ++i) {
        m_allWidget->listStructNode.at(i)->indexLine=m_allWidget->atWhichLine(m_allWidget->listStructNode.at(i)->posNode);
    }// 更新填入完毕所有节点所在的直线序号
    for (int i = 0; i < m_allWidget->listStructNode.size(); ++i) {
        QList<int> lines=DFS(m_allWidget->listStructLine,m_allWidget->listStructNode.at(i)->indexLine);
        for (int j = 0; j < lines.size(); ++j) {
            m_allWidget->listStructLine.at(lines.at(j))->nodeNameLinked=m_allWidget->listStructNode.at(i)->name;
        }
    }// 更新完毕和存在的节点相连接的直线都标上了连接的node

    for (int i = 0; i < m_allWidget->listStructPort.size(); ++i) {
        int nn=m_allWidget->atWhichLine(m_allWidget->listStructPort.at(i)->posPort);
        if(nn<0)
        {}//说明这个端口没有连接到任何直线
        else
        {
            m_allWidget->listStructPort.at(i)->nodeNameLinked=m_allWidget->listStructLine.at(nn)->nodeNameLinked;
        }
    }//更新完毕所有端口都标上了连接的node
    for (int i = 0; i < m_allWidget->listStructPort.size(); ++i) {
        qDebug()<<QString::number(i)+":"<< m_allWidget->listStructPort.at(i)->nodeNameLinked<<"\n";
    }
    QString path=QCoreApplication::applicationDirPath() + "/info.txt";
    QFile file1(path);
    if (file1.open(QIODevice::WriteOnly  | QIODevice::Text))
    {
        //如果文件打开成功
        QTextStream out(&file1);
        for (int i = m_allWidget->numComponentExistLeft; i < m_allWidget->listStructWidget.size(); ++i) {

            QString objNameOfComponent=m_allWidget->listStructWidget.at(i)->currWidgObjName;

            QString name=m_allWidget->listStructWidget.at(i)->name;             // 元件名称
            QString value=m_allWidget->listStructWidget.at(i)->value;           // 元件值
            QString starValue=m_allWidget->listStructWidget.at(i)->starValue;   // 初值
            QString nOrder=m_allWidget->listStructWidget.at(i)->nOrder;         // 阶次
            QString fz=m_allWidget->listStructWidget.at(i)->fz;                 // 幅值
            QString fre=m_allWidget->listStructWidget.at(i)->fre;               // 频率
            QString cxw=m_allWidget->listStructWidget.at(i)->cxw;               // 初相位
            QString NADD=m_allWidget->listStructWidget.at(i)->NADD;             // N+控制
            QString NCUT=m_allWidget->listStructWidget.at(i)->NCUT;             // N-控制
            QString k1=m_allWidget->listStructWidget.at(i)->k1;                 // k1变比系数
            QString G=m_allWidget->listStructWidget.at(i)->G;                   // G回转器系数
            QString orderLeft=m_allWidget->listStructWidget.at(i)->orderLeft;   // 阶次（一次侧）
            QString orderRight=m_allWidget->listStructWidget.at(i)->orderRight; // 阶次（二次侧）
            QString hgxs=m_allWidget->listStructWidget.at(i)->hgxs;             // 互感系数

            QString nodeAdd=m_allWidget->findPortLinkedNodeNameByComponentName("port1",name);
            QString nodeCut=m_allWidget->findPortLinkedNodeNameByComponentName("port0",name);

            if(objNameOfComponent.contains("R"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"\n";
            else if(objNameOfComponent.contains("C"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"            "<<starValue<<"\n";
            else if(objNameOfComponent.contains("FC"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"            "<<nOrder<<"            "<<starValue<<"\n";
            else if(objNameOfComponent.contains("L"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"            "<<starValue<<"\n";
            else if(objNameOfComponent.contains("FL"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"            "<<nOrder<<"            "<<starValue<<"\n";
            else if(objNameOfComponent.contains("E"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<fz<<"            "<<fre<<"            "<<cxw<<"\n";
            else if(objNameOfComponent.contains("I"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<fz<<"            "<<fre<<"            "<<cxw<<"\n";
            else if(objNameOfComponent.contains("CS"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<NADD<<"            "<<NCUT<<"            "<<value<<"\n";
            else if(objNameOfComponent.contains("VS"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<NADD<<"            "<<NCUT<<"            "<<value<<"\n";

            else if(objNameOfComponent.contains("T"))//注意变压器有4个端
            {
                QString nodeAddLeft=m_allWidget->findPortLinkedNodeNameByComponentName("port0",name);
                QString nodeCutLeft=m_allWidget->findPortLinkedNodeNameByComponentName("port1",name);
                QString nodeAddRight=m_allWidget->findPortLinkedNodeNameByComponentName("port3",name);
                QString nodeCutRight=m_allWidget->findPortLinkedNodeNameByComponentName("port2",name);
                out<<name<<"         "<<nodeAddLeft<<"            "<<nodeCutLeft<<"            "<<nodeAddRight<<"            "<<nodeCutRight<<"            "<<k1<<"\n";
            }
            else if(objNameOfComponent.contains("H"))
                out<<name<<"         "<<nodeAdd<<"            "<<nodeCut<<"            "<<value<<"\n";
            else if(objNameOfComponent.contains("K"))
            {
                QString nodeAddLeft=m_allWidget->findPortLinkedNodeNameByComponentName("port0",name);
                QString nodeCutLeft=m_allWidget->findPortLinkedNodeNameByComponentName("port1",name);
                QString nodeAddRight=m_allWidget->findPortLinkedNodeNameByComponentName("port3",name);
                QString nodeCutRight=m_allWidget->findPortLinkedNodeNameByComponentName("port2",name);
                out<<name<<"         "<<nodeAddLeft<<"            "<<nodeCutLeft<<"            "<<orderLeft<<"            "<<nodeAddRight<<"            "<<nodeCutRight<<"            "<<orderRight<<"            "<<hgxs<<"\n";

            }
        }
        file1.close();
    }

}

void MainWindow::on_actionDelet_triggered()
{
    //tmpWidgetDelet这是临时传递右键选中的元件底widget对象名
    QList<int> portListTmp=m_allWidget->findPortListByWidget(tmpWidgetDelet);
    for (int j = 0; j < portListTmp.size(); ++j) {
        int indexTmp=portListTmp.at(j);
        if(m_allWidget->atWhichLine(m_allWidget->listStructPort.at(indexTmp)->posPort)>=0)
        {
            qDebug()<<"右键单击了控件,但是此控件有线连着，所以不能删除，本函数结束"<<"\n";
            QMessageBox::warning(this,"警告","元件连着导线，不能删除该元件，得先删除连着的导线",QMessageBox::Abort);
            return;// 本函数结束，这样写法还是不错的
        }
    }
    m_allWidget->deletWidget(tmpWidgetDelet);
    update();
}

void MainWindow::on_actionRotateLeft90_triggered()
{
    //tmpWidgetDelet这是临时传递右键选中的元件底widget对象名
    QList<int> portListTmp=m_allWidget->findPortListByWidget(tmpWidgetDelet);
    for (int j = 0; j < portListTmp.size(); ++j) {
        int indexTmp=portListTmp.at(j);
        if(m_allWidget->atWhichLine(m_allWidget->listStructPort.at(indexTmp)->posPort)>=0)
        {
            qDebug()<<"右键单击了控件,但是此控件有线连着，所有不能旋转，本函数结束"<<"\n";
            QMessageBox::warning(this,"警告","元件连着导线，不能旋转",QMessageBox::Abort);
            return;// 本函数结束，这样写法还是不错的
        }
    }
    structWidget *tt=m_allWidget->findStructWidget(tmpWidgetDelet);
    if(tt->state==1)//如果这个元件已经是旋转了90度，就不允许再旋转了，因为我没做其它角度的旋转，函数结束
        return;
    structWidget *tmp = new structWidget;
    tmp->currWidgObjName = tt->currWidgObjName;
    tmp->state=tt->state;
    tmp->point=tt->point;

    tmp->name=tt->name;
    tmp->value=tt->value;
    tmp->starValue=tt->starValue;
    tmp->nOrder=tt->nOrder;
    tmp->fz=tt->fz;
    tmp->fre=tt->fre;
    tmp->cxw=tt->cxw;
    tmp->NADD=tt->NADD;
    tmp->NCUT=tt->NCUT;
    tmp->k1=tt->k1;
    tmp->G=tt->G;
    tmp->orderLeft=tt->orderLeft;
    tmp->orderRight=tt->orderRight;
    tmp->hgxs=tt->hgxs;

    //元件复制完成
    m_allWidget->deletWidget(tmpWidgetDelet);
    m_allWidget->copyComponentRotate(tmp);
}

void MainWindow::on_actionAddNode_triggered()
{
    bool ok;
    QString str=QInputDialog::getText(this,"set node name","node name",QLineEdit::Normal,QString::number(m_allWidget->cntAddNode),&ok);
    if(ok)
        m_allWidget->addNode(ui->centralwidget,tmpPosAddNode,str);
}

void MainWindow::on_actionSetParameter_triggered()
{
    MainWindowSetComponentParameter *mmc = new MainWindowSetComponentParameter(m_allWidget->listStructWidget.at(tmpIndexSetParameter),this);
    mmc->show();

}

void MainWindow::on_actionDeletLine_triggered()
{
    if(m_allWidget->isHaveNodeOnThisLine(tmpLineDelet))
    {
        QMessageBox::warning(this,"警告","导线上有节点，不能删除，只能先删除节点",QMessageBox::Abort);
        return;
    }
    m_allWidget->deletLine(tmpLineDelet);
    update();
}

void MainWindow::on_actionDeletNode_triggered()
{
    m_allWidget->deletNode(tmpNodeDelet);
    update();
}

void MainWindow::on_actionEditNode_triggered()
{
    structNode *tmp=m_allWidget->findStructNode(tmpNodeDelet);
    bool ok;
    QString str=QInputDialog::getText(this,"set node name","node name",QLineEdit::Normal,tmp->name,&ok);
    if(ok)
        tmp->name=str;
    else
        return;
    QWidget *wid = findChild<QWidget *>(tmpNodeDelet);
    QList<QLabel *> lab = wid->findChildren<QLabel*>();
    for (int i = 0; i < lab.size(); ++i) {
        if(lab.at(i)->objectName().contains("content"))//说明找到了元件中的content标签，更新他的显示了
            lab.at(i)->setText(tmp->name);
    }

}

void MainWindow::on_pushButtonHindNet_clicked()
{
    if(ui->pushButtonHindNet->text()=="隐藏网格")
    {
        ui->pushButtonHindNet->setText("显示网格");
        flagTurnOnNet=false;
    }
    else
    {
        ui->pushButtonHindNet->setText("隐藏网格");
        flagTurnOnNet=true;
    }
    update();
}

void MainWindow::on_actionOpenWindow_triggered()
{
    MainWindowDraw* scope = new MainWindowDraw(this);
    scope->setWindowFlags(scope->windowFlags()|Qt::Dialog);
    scope->show();
}
