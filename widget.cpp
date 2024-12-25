#include "widget.h"
#include "ui_widget.h"
#include <QPainter> // 添加此行
#include <QTimer>
#include "gamemap.h"
#include <vector>
#include <QMouseEvent>
#include <utility>  // 含pair
#include <QDebug>

// 地图格子
std::vector<std::vector<Space*>> spaces;

// 爆炸位置
std::vector<std::vector<bool>> toBomb;

// 消融位置
std::vector<std::vector<bool>> toMelt;

// 画板距离地图边界的距离
const int i_off = 80;
const int j_off = 50;

//方块大小
const int block_size = 50;

// 地图大小
const int row = 8;
const int col = 8;

// 重绘时间间隔
const int duration = 11;

// 选中方块坐标
std::vector<std::pair<int, int>> selected_points;

// 交换位置的坐标
std::vector<int> swaping_points;
// 交换成功标志
int swap_state = 0;

// 提示宝石位置坐标
std::vector<int> tip_points;

// 有关地图状态的参数

int score = 0;
bool map_swaping = false;
bool map_falling = false;
bool map_bombing = false;

// 交换速度
const int swaping_speed = 5;

// 有关道具及鼠标选择的布尔值、范围
bool prop_bomb = false;
bool prop_refresh = false;
bool prop_tip = false;
bool prop_hammer = false;

int bomb_num = 3;
int refresh_num = 3;
int tip_num = 3;
int hammer_num = 3;


// 有关地图相关操作的函数
void mapInitial();
void SwapStone(int i1, int j1, int i2, int j2);
void performBomb();
void setDownAndFill();
bool checkMap();
bool simpleCheckMap();
bool checkOne(int i, int j);
void performMelt();
void returnSwap();
bool updateGemstonePositions();
bool judgeContinue();
void Tip();
void freshMap();

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , timer(new QTimer(this))
{


    // 加载图片
    picInitial();

    ui->setupUi(this);

    // 地图初始化
    toBomb.assign(row, std::vector<bool>(col, false));
    toMelt.assign(row, std::vector<bool>(col, false));
    mapInitial();
    swaping_points = {-1,-1,-1,-1};
    tip_points = {-1,-1,-1,-1};

    // 连接 QTimer 的 timeout 信号到槽函数
    connect(timer, &QTimer::timeout, this, &Widget::updateWidget);

    // 启动定时器，每隔一段时间重绘一次
    timer->start(duration);
}


Widget::~Widget()
{
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            delete spaces[i][j];
        }
    }

    delete ui;
}


// 实现paintEvent方法
void Widget::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);


    // 绘制格子背景
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            QColor color(30+i*17,200, 30+j*17, 250); // 设置格子背景渐变颜色
            painter.fillRect(j_off + j*block_size,i_off + i*block_size,block_size,block_size,color);
        }
    }

    // 绘制宝石及地形
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            int space_type = spaces[i][j]->GetType();
            int type = spaces[i][j]->GetGemstone()->GetType();
            Gemstone *g = spaces[i][j]->GetGemstone();
            if(toBomb[i][j] == false & g!=nullptr){
                painter.drawPixmap(g->currentX,g->currentY,pic_fruits[type]);


            } else{
                Gemstone *g = spaces[i][j]->GetGemstone();
                if(g->bomb_life >= 20){
                    painter.drawPixmap(j_off+j*block_size,i_off+i*block_size,pic_destroy1);
                }
                if(g->bomb_life >= 10){
                    painter.drawPixmap(j_off+j*block_size,i_off+i*block_size,pic_destroy2);
                }
                if(g->bomb_life >= 0){
                    painter.drawPixmap(j_off+j*block_size,i_off+i*block_size,pic_destroy3);
                }
            }
            if(space_type > 0){

                if(space_type == 1)
                    painter.setOpacity(0.50);
                if(space_type == 2)
                    painter.setOpacity(0.74);
                if(space_type == 3)
                    painter.setOpacity(0.84);

                painter.drawPixmap(j_off+j*block_size,i_off+i*block_size,pic_ices[space_type-1]);
                painter.setOpacity(1.0);

            }

            if(space_type == -1){
                QColor color(0,0,0); // 设置格子背景渐变颜色
                painter.fillRect(j_off + j*block_size,i_off + i*block_size,block_size,block_size,color);
            }
        }
    }

    // 刷新得分
    ui->scoreLabel->setText(QString::number(score));

    // 绘制道具
    painter.drawPixmap(50, 490, pic_bomb.scaled( 50, 50, Qt::KeepAspectRatio));
    painter.drawPixmap(140, 490, pic_refresh.scaled( 50, 50, Qt::KeepAspectRatio));
    painter.drawPixmap(230, 490, pic_hammer.scaled( 50, 50, Qt::KeepAspectRatio));
    painter.drawPixmap(320, 490, pic_tip.scaled( 50, 50, Qt::KeepAspectRatio));

    // 绘制选中效果
    if(!selected_points.empty()){

        // 创建颜色对象，设置透明度
        QColor color(100, 100, 100, 150); // 150：透明度为约 59%
        int i = selected_points[0].first;
        int j = selected_points[0].second;
        ui->label_2->setText(QString::number(i)+","+QString::number(j));
        painter.fillRect(j_off + j*block_size,i_off + i*block_size,block_size,block_size,color);
    }

    // 绘制提示效果
    if(tip_points[0]!=-1){
        // 创建颜色对象，设置透明度
        QColor color(20, 20, 250, 150); // 150：透明度为约 59%
        int i1 = tip_points[0];
        int j1 = tip_points[1];
        int i2 = tip_points[2];
        int j2 = tip_points[3];
        painter.fillRect(j_off + j1*block_size,i_off + i1*block_size,block_size,block_size,color);
        painter.fillRect(j_off + j2*block_size,i_off + i2*block_size,block_size,block_size,color);
    }



    if(updateGemstonePositions())   //若在交换或下落，则返回
        return;

    if(judgeContinue()){
        qDebug()<<"游戏可继续";
    } else{
        qDebug()<<"游戏不可继续";
        freshMap();
    }

    checkMap();
    performBomb();
    performMelt();
    setDownAndFill();

}


// 实现mousePressEvent方法
void Widget::mousePressEvent(QMouseEvent *event)
{
    // 判断是否执行交换
    if(swap_state != 0)
        return;

    // 消除提示信息
    if(tip_points[0] != -1){
        tip_points = {-1,-1,-1,-1};
        prop_tip = false;
    }

    int click_x = event->pos().x(); // 获取 x 坐标
    int click_y = event->pos().y(); // 获取 y 坐标

    ui->label->setText(QString::number(click_x)+","+QString::number(click_y));

    int click_i = (click_y-i_off)/block_size; // 获取当前i
    int click_j = (click_x-j_off)/block_size; // 获取当前j

    // 判断两次选择是否重复
    bool is_repeat = !selected_points.empty() && click_i == selected_points[0].first
                     && click_j == selected_points[0].second;

    // 判断是否已选择道具
    bool is_select_prop = prop_bomb || prop_refresh || prop_hammer || prop_tip;

    // 判断是否点击了不可交换地形
    bool is_select_terrain = false;
    if(click_i>=0 && click_i<row && click_j>=0 && click_j<col)
        is_select_terrain = (spaces[click_i][click_j]->GetType() != 0);

    if(click_i>=0 && click_i<row && click_j>=0 && click_j<col
        && !is_repeat && !is_select_prop && !is_select_terrain){   // 判断i,j是否合法及其他合法情况
        selected_points.push_back(std::make_pair(click_i,click_j));

        // 若已经选择两个方块
        if(selected_points.size() == 2){
            int i1 = selected_points[0].first;
            int j1 = selected_points[0].second;
            int i2 = selected_points[1].first;
            int j2 = selected_points[1].second;
            SwapStone(i1,j1,i2,j2);
            selected_points.clear();
        }
    }

    // 若选择炸弹
    if(prop_bomb){

        if(click_i>=0 && click_i<row && click_j>=0 && click_j<col){
            toBomb[click_i][click_j] = true;

            if(click_j-1>=0)
                toBomb[click_i][click_j-1] = true;
            if(click_j+1<col)
                toBomb[click_i][click_j+1] = true;

            if(click_i-1>=0){
                toBomb[click_i-1][click_j] = true;
                if(click_j-1>=0)
                    toBomb[click_i-1][click_j-1] = true;
                if(click_j+1<col)
                    toBomb[click_i-1][click_j+1] = true;
            }

            if(click_i+1<row){
                toBomb[click_i+1][click_j] = true;
                if(click_j-1>=0)
                    toBomb[click_i+1][click_j-1] = true;
                if(click_j+1<col)
                    toBomb[click_i+1][click_j+1] = true;
            }

            prop_bomb = false;
        }
    }

    // 若选择刷新
    if(prop_refresh && click_i>=0 && click_i<row && click_j>=0 && click_j<col){
        for(int i=0; i<row;i++){
            for(int j=0;j<col;j++){
                Gemstone *g = spaces[i][j]->GetGemstone();
                spaces[i][j]->SetGemstone(nullptr);
                delete g;
            }
        }
        setDownAndFill();
        prop_refresh = false;
    }


    // 若选择锤子
    if(prop_hammer && click_i>=0 && click_i<row && click_j>=0 && click_j<col){
        int type = spaces[click_i][click_j]->GetGemstone()->GetType();
        for(int i=0;i<row;i++){
            for(int j=0;j<col;j++){
                Gemstone *g = spaces[i][j]->GetGemstone();
                if(type == g->GetType()){
                    toBomb[i][j] = true;
                }
            }
        }
        prop_hammer = false;
    }



    // 判断是否点击了道具
    if(click_y >= 490 && click_y <= 490+50){
        // 炸弹道具区
        if(click_x >= 50 && click_x <= 50+50){
            selected_points.clear();
            prop_bomb = true;
            prop_hammer = false;
            prop_refresh = false;
            prop_tip = false;
        }
        // 刷新道具区
        if(click_x >= 140 && click_x <= 140+50){
            selected_points.clear();
            prop_bomb = false;
            prop_hammer = false;
            prop_refresh = true;
            prop_tip = false;
        }
        // 锤子道具区
        if(click_x >= 230 && click_x <= 230+50){
            selected_points.clear();
            prop_bomb = false;
            prop_hammer = true;
            prop_refresh = false;
            prop_tip = false;
        }
        // 提示道具区
        if(click_x >= 320 && click_x <= 320+50){
            selected_points.clear();
            prop_bomb = false;
            prop_hammer = false;
            prop_refresh = false;
            prop_tip = true;
            Tip();
        }
    }
}

void Widget::updateWidget(){
    update(); // 请求重绘窗口
}


// 图片加载
void Widget::picInitial(){

    // 宝石图片的加载
    pic_mine1.load(":/material/picture/mine/mine1.png");
    pic_mine2.load(":/material/picture/mine/mine2.png");
    pic_mine3.load(":/material/picture/mine/mine3.png");
    pic_mine4.load(":/material/picture/mine/mine4.png");
    pic_mine5.load(":/material/picture/mine/mine5.png");
    pic_mine6.load(":/material/picture/mine/mine6.png");
    pic_mine7.load(":/material/picture/mine/mine7.png");
    pic_mine8.load(":/material/picture/mine/mine8.png");

    pic_mines = {pic_mine1,pic_mine2,pic_mine3,pic_mine4,pic_mine5,pic_mine6,pic_mine7,pic_mine8};

    pic_fruit1.load(":/material/picture/mine/fruit1.png");
    pic_fruit2.load(":/material/picture/mine/fruit2.png");
    pic_fruit3.load(":/material/picture/mine/fruit3.png");
    pic_fruit4.load(":/material/picture/mine/fruit4.png");
    pic_fruit5.load(":/material/picture/mine/fruit5.png");
    pic_fruit6.load(":/material/picture/mine/fruit6.png");
    pic_fruit7.load(":/material/picture/mine/fruit7.png");
    pic_fruit8.load(":/material/picture/mine/fruit8.png");

    pic_fruits = {pic_fruit1,pic_fruit2,pic_fruit3,pic_fruit4,pic_fruit5,pic_fruit6,pic_fruit7,pic_fruit8};

    pic_ice1.load(":/material/picture/mine/ice1.png");
    pic_ice2.load(":/material/picture/mine/ice2.png");
    pic_ice3.load(":/material/picture/mine/ice3.png");

    pic_ices = {pic_ice1,pic_ice2,pic_ice3};

    pic_destroy1.load(":/material/picture/mine/destroy1.png");
    pic_destroy2.load(":/material/picture/mine/destroy2.png");
    pic_destroy3.load(":/material/picture/mine/destroy3.png");

    pic_bomb.load(":/material/picture/tools/bomb.png");
    pic_refresh.load(":/material/picture/tools/refresh.png");
    pic_hammer.load(":/material/picture/tools/hammer.png");
    pic_tip.load(":/material/picture/tools/tip.jpg");

}


bool updateGemstonePositions() {


    // 判断交换动画--成功
    if (swap_state == 1) {

            qDebug()<<"交换一帧(成功)";

        int i1 = swaping_points[0];
        int j1 = swaping_points[1];
        int i2 = swaping_points[2];
        int j2 = swaping_points[3];
        Gemstone* g1 = spaces[i1][j1]->GetGemstone();
        Gemstone* g2 = spaces[i2][j2]->GetGemstone();




        // 判断方向，处理动画
        if (i1 == i2) { // 水平方向交换
            if (j1 < j2) {
                // 1在左 2在右
                int targetX1 = j_off + j1 * block_size;
                int targetX2 = j_off + j2 * block_size;

                if (g1->currentX > targetX1) {
                    g1->currentX -= swaping_speed;
                    if (g1->currentX <= targetX1) {
                        g1->currentX = targetX1;

                    }
                }
                if (g2->currentX < targetX2) {
                    g2->currentX += swaping_speed;
                    if (g2->currentX >= targetX2) {
                        g2->currentX = targetX2;

                    }
                }
                if(g1->currentX == targetX1 && g2->currentX == targetX2)
                    swap_state = 0;

            } else {
                // 1在右 2在左
                int targetX1 = j_off + j1 * block_size;
                int targetX2 = j_off + j2 * block_size;

                if (g1->currentX < targetX1) {
                    g1->currentX += swaping_speed;
                    if (g1->currentX >= targetX1) {
                        g1->currentX = targetX1;

                    }
                }
                if (g2->currentX > targetX2) {
                    g2->currentX -= swaping_speed;
                    if (g2->currentX <= targetX2) {
                        g2->currentX = targetX2;

                    }
                }
                if(g1->currentX == targetX1 && g2->currentX == targetX2)
                    swap_state = 0;
            }
        } else { // 垂直方向交换
            if (i1 < i2) {
                // 1在上 2在下
                int targetY1 = i_off + i1 * block_size;
                int targetY2 = i_off + i2 * block_size;

                if (g1->currentY > targetY1) {
                    g1->currentY -= swaping_speed;
                    if (g1->currentY <= targetY1) {
                        g1->currentY = targetY1;

                    }
                }
                if (g2->currentY < targetY2) {
                    g2->currentY += swaping_speed;
                    if (g2->currentY >= targetY2) {
                        g2->currentY = targetY2;

                    }
                }
                if(g1->currentY == targetY1 && g2->currentY == targetY2)
                    swap_state = 0;
            } else {
                // 1在下 2在上
                int targetY1 = i_off + i1 * block_size;
                int targetY2 = i_off + i2 * block_size;
                if (g1->currentY < targetY1) {
                    g1->currentY += swaping_speed;
                    if (g1->currentY >= targetY1) {
                        g1->currentY = targetY1;

                    }
                }
                if (g2->currentY > targetY2) {
                    g2->currentY -= swaping_speed;
                    if (g2->currentY <= targetY2) {
                        g2->currentY = targetY2;

                    }
                }
                if(g1->currentY == targetY1 && g2->currentY == targetY2)
                    swap_state = 0;
            }
        }
    }

    // 判断交换动画--失败
    if (swap_state == -1) {

        qDebug()<<"交换一帧(失败)";

        int i1 = swaping_points[0];
        int j1 = swaping_points[1];
        int i2 = swaping_points[2];
        int j2 = swaping_points[3];
        Gemstone* g1 = spaces[i1][j1]->GetGemstone();
        Gemstone* g2 = spaces[i2][j2]->GetGemstone();


        // 判断方向，处理动画
        if (i1 == i2) { // 水平方向交换
            if (j1 < j2) {
                // 1在左 2在右
                int targetX1 = j_off + j1 * block_size;
                int targetX2 = j_off + j2 * block_size;

                if (g1->currentX > targetX1) {
                    g1->currentX -= swaping_speed;
                    if (g1->currentX <= targetX1) {
                        g1->currentX = targetX1;

                    }
                }
                if (g2->currentX < targetX2) {
                    g2->currentX += swaping_speed;
                    if (g2->currentX >= targetX2) {
                        g2->currentX = targetX2;
                    }
                }
                if(g1->currentX == targetX1 && g2->currentX == targetX2)
                    returnSwap();

            } else {

                // 1在右 2在左
                int targetX1 = j_off + j1 * block_size;
                int targetX2 = j_off + j2 * block_size;

                if (g1->currentX < targetX1) {
                    g1->currentX += swaping_speed;
                    if (g1->currentX >= targetX1) {
                        g1->currentX = targetX1;
                    }
                }
                if (g2->currentX > targetX2) {
                    g2->currentX -= swaping_speed;
                    if (g2->currentX <= targetX2) {
                        g2->currentX = targetX2;
                    }
                }
                if(g1->currentX == targetX1 && g2->currentX == targetX2)
                    returnSwap();
            }
        } else { // 垂直方向交换
            if (i1 < i2) {
                // 1在上 2在下
                int targetY1 = i_off + i1 * block_size;
                int targetY2 = i_off + i2 * block_size;

                if (g1->currentY > targetY1) {
                    g1->currentY -= swaping_speed;
                    if (g1->currentY <= targetY1) {
                        g1->currentY = targetY1;
                    }
                }
                if (g2->currentY < targetY2) {
                    g2->currentY += swaping_speed;
                    if (g2->currentY >= targetY2) {
                        g2->currentY = targetY2;
                    }
                }
                if(g1->currentY == targetY1 && g2->currentY == targetY2)
                    returnSwap();
            } else {
                // 1在下 2在上
                int targetY1 = i_off + i1 * block_size;
                int targetY2 = i_off + i2 * block_size;
                if (g1->currentY < targetY1) {
                    g1->currentY += swaping_speed;
                    if (g1->currentY >= targetY1) {
                        g1->currentY = targetY1;
                    }
                }
                if (g2->currentY > targetY2) {
                    g2->currentY -= swaping_speed;
                    if (g2->currentY <= targetY2) {
                        g2->currentY = targetY2;
                    }
                }
                if(g1->currentY == targetY1 && g2->currentY == targetY2)
                    returnSwap();
            }
        }
    }



    // 判断下落动画
    bool falling = false;
    const int falling_speed = 6; // 控制下落速度
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            Gemstone* g = spaces[i][j]->GetGemstone();
            if (g != nullptr) {
                int targetY = i_off + i * block_size;
                if (g->currentY < targetY) {
                    g->currentY += falling_speed;
                    falling = true;

                    // 防止越过目标位置
                    if (g->currentY > targetY) {
                        g->currentY = targetY;
                    }
                }
            }
        }
    }



    return falling || (swap_state!=0);
}


// 地图初始化
void mapInitial() {
    spaces.resize(row, std::vector<Space*>(col));

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            spaces[i][j] = new Space();
            spaces[i][j]->SetGemstone(nullptr);
        }
    }

    spaces[3][2]->SetType(-1);
    spaces[3][3]->SetType(-1);
    spaces[3][4]->SetType(-1);
    spaces[3][5]->SetType(-1);
    spaces[3][6]->SetType(-1);
    spaces[4][2]->SetType(-1);
    spaces[4][3]->SetType(-1);
    spaces[4][4]->SetType(-1);
    spaces[4][5]->SetType(-1);
    spaces[4][6]->SetType(-1);

    spaces[7][0]->SetType(-1);
    spaces[6][1]->SetType(-1);
    spaces[7][7]->SetType(-1);
    spaces[6][6]->SetType(-1);
    spaces[5][5]->SetType(-1);
    spaces[5][2]->SetType(-1);

    setDownAndFill();

}


// 交换格子的宝石
void SwapStone(int i1, int j1, int i2, int j2) {
    // 检查边界和邻接情况
    if (i1 < 0 || i1 >= row || j1 < 0 || j1 >= col ||
        i2 < 0 || i2 >= row || j2 < 0 || j2 >= col)
        return;

    // 两个格子必须相邻
    if (std::abs(i1 - i2) + std::abs(j1 - j2) != 1)
        return;

    Gemstone* g1 = spaces[i1][j1]->GetGemstone();
    Gemstone* g2 = spaces[i2][j2]->GetGemstone();

    spaces[i1][j1]->SetGemstone(g2);
    spaces[i2][j2]->SetGemstone(g1);

    swaping_points = {i1,j1,i2,j2};

    if (simpleCheckMap()) {
        swap_state = 1;

    } else {
        swap_state = -1;

    }
}

void returnSwap(){

    int i1 = swaping_points[0];
    int j1 = swaping_points[1];
    int i2 = swaping_points[2];
    int j2 = swaping_points[3];

    Gemstone *g1 = spaces[i1][j1]->GetGemstone();
    Gemstone *g2 = spaces[i2][j2]->GetGemstone();

    spaces[i2][j2]->SetGemstone(g1);
    spaces[i1][j1]->SetGemstone(g2);

    swap_state = 1;

}

// 根据toBomb位置执行消融操作
void performMelt() {

    // 根据消融位置执行消融操作
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            int space_type = spaces[i][j]->GetType();
            if(toMelt[i][j] && space_type >0){
                spaces[i][j]->SetType(space_type-1);
                toMelt[i][j] = false;
            }
        }
    }

}



// 执行爆炸
void performBomb() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (toBomb[i][j]) {
                Gemstone *g = spaces[i][j]->GetGemstone();
                if(g->bomb_life > 0)
                {

                    g->bomb_life--;

                }else{

                    delete spaces[i][j]->GetGemstone();
                    spaces[i][j]->SetGemstone(nullptr);
                    toBomb[i][j] = false;
                    score += 50;

                    // 让指定位置融化
                    toMelt[i][j] = true;
                    if(j-1>=0)
                        toMelt[i][j-1] = true;
                    if(j+1<col)
                        toMelt[i][j+1] = true;
                    if(i-1>=0){
                        toMelt[i-1][j] = true;
                    }
                    if(i+1<row){
                        toMelt[i+1][j] = true;
                    }

                }
            }
        }
    }
}



// 设置下落和填充
void setDownAndFill() {
    std::vector<int> lack_num(col,0);

    for(int i=0 ; i<row;i++){
        for(int j=0;j<col;j++){
            if(spaces[i][j]->GetGemstone() == nullptr)
                lack_num[j] = lack_num[j]+1;
        }
    }

    // 设置下落方块至所有方块落地
    for (int i = row - 1; i >= 0; i--) {
        for (int j = col - 1; j >= 0; j--) {
            if (spaces[i][j]->GetGemstone() == nullptr) {
                int temp_i = i - 1;
                while (temp_i >= 0) {
                    if (spaces[temp_i][j]->GetGemstone() != nullptr) {

                        Gemstone* g = spaces[temp_i][j]->GetGemstone();
                        spaces[i][j]->SetGemstone(g);
                        spaces[temp_i][j]->SetGemstone(nullptr);
                        break;
                    }
                    temp_i--;
                }
            }
        }
    }


    // 填充方块，使其从地图外进入
    for(int i= row - 1 ; i>=0; i--) {
        for(int j=0;j<col;j++){
            if (spaces[i][j]->GetGemstone() == nullptr) {
                Gemstone *g = new Gemstone(6);

                g->currentX = j_off + j*block_size;
                g->currentY = i_off + (i-lack_num[j])*block_size;

                spaces[i][j]->SetGemstone(g);
            }
        }
    }
}


// 判断地图是否有可消除部分
bool checkMap() {
    bool couldBomb = false;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (spaces[i][j]->GetGemstone() == nullptr) continue;

            if (j < col - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i][j + 1]->GetGemstone()->GetType() &&
                spaces[i][j + 1]->GetGemstone()->GetType() == spaces[i][j + 2]->GetGemstone()->GetType()) {

                couldBomb = true;
                toBomb[i][j] = true;
                toBomb[i][j + 1] = true;
                toBomb[i][j + 2] = true;
            }

            if (i < row - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i + 1][j]->GetGemstone()->GetType() &&
                spaces[i + 1][j]->GetGemstone()->GetType() == spaces[i + 2][j]->GetGemstone()->GetType()) {

                couldBomb = true;
                toBomb[i][j] = true;
                toBomb[i + 1][j] = true;
                toBomb[i + 2][j] = true;
            }
        }
    }
    return couldBomb;
}

// 判断地图是否有可消除部分
bool simpleCheckMap() {
    bool couldBomb = false;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (spaces[i][j]->GetGemstone() == nullptr) continue;
            if (j < col - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i][j + 1]->GetGemstone()->GetType() &&
                spaces[i][j + 1]->GetGemstone()->GetType() == spaces[i][j + 2]->GetGemstone()->GetType()) {
                couldBomb = true;

            }

            if (i < row - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i + 1][j]->GetGemstone()->GetType() &&
                spaces[i + 1][j]->GetGemstone()->GetType() == spaces[i + 2][j]->GetGemstone()->GetType()) {

                couldBomb = true;

            }
        }
    }
    return couldBomb;
}


// 提示道具
void Tip(){
    for(int i=row-1;i>=0;i--){
        for(int j=col-1;j>=0;j--){
            // 判断向上是否可交换
            if(i>0 && spaces[i][j]->GetType()==0 && spaces[i-1][j]->GetType()==0){
                Gemstone *g1 = spaces[i][j]->GetGemstone();
                Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                spaces[i][j]->SetGemstone(g2);
                spaces[i-1][j]->SetGemstone(g1);
                if(!simpleCheckMap()){  //交换后不可消除，换回
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i-1][j]->SetGemstone(g1);
                    continue;
                } else{
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i-1][j]->SetGemstone(g1);
                    tip_points = {i,j,i-1,j};
                }
            }
            // 判断向左是否可交换
            if(j>0 && spaces[i][j]->GetType()==0 && spaces[i][j-1]->GetType()==0){
                Gemstone *g1 = spaces[i][j]->GetGemstone();
                Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                spaces[i][j]->SetGemstone(g2);
                spaces[i][j-1]->SetGemstone(g1);
                if(!simpleCheckMap()){  //交换后不可消除，换回
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i][j-1]->SetGemstone(g1);
                    continue;
                } else{
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i][j-1]->SetGemstone(g1);
                    tip_points = {i,j,i,j-1};
                }
            }
        }
    }


}

// 判断游戏是否可继续(是否存在可消除部分)
bool judgeContinue(){

    for(int i=row-1;i>=0;i--){
        for(int j=col-1;j>=0;j--){
            // 判断向上是否可交换
            if(i>0 && spaces[i][j]->GetType()==0 && spaces[i-1][j]->GetType()==0){
                Gemstone *g1 = spaces[i][j]->GetGemstone();
                Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                spaces[i][j]->SetGemstone(g2);
                spaces[i-1][j]->SetGemstone(g1);
                if(!simpleCheckMap()){  //交换后不可消除，换回
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i-1][j]->SetGemstone(g1);
                    continue;
                } else{
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i-1][j]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i-1][j]->SetGemstone(g1);
                    return true;
                }
            }
            // 判断向左是否可交换
            if(j>0 && spaces[i][j]->GetType()==0 && spaces[i][j-1]->GetType()==0){
                Gemstone *g1 = spaces[i][j]->GetGemstone();
                Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                spaces[i][j]->SetGemstone(g2);
                spaces[i][j-1]->SetGemstone(g1);
                if(!simpleCheckMap()){  //交换后不可消除，换回
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i][j-1]->SetGemstone(g1);
                    continue;
                } else{
                    Gemstone *g1 = spaces[i][j]->GetGemstone();
                    Gemstone *g2 = spaces[i][j-1]->GetGemstone();
                    spaces[i][j]->SetGemstone(g2);
                    spaces[i][j-1]->SetGemstone(g1);
                    return true;
                }
            }
        }
    }
    return false;
}

// 刷新地图
void freshMap(){
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            Gemstone * g =spaces[i][j]->GetGemstone();
            spaces[i][j]->SetGemstone(nullptr);
            delete g;
        }
    }
}
