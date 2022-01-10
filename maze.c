#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void barDefeat(int** maze, int row, int column);
int getDirection(int** maze, int row, int column);


int ** makeMaze(int row, int column){
    srand((unsigned)time(NULL));
    
    int i, j; //row: 行数 column: 列数
    int** maze; //迷路
    if(row % 2 == 0 || column % 2 == 0 || row < 2 || column < 2){
        fprintf(stderr, "The values should be odd and more than 1");
        exit(0);
    }
    maze = (int**)malloc(sizeof(int*) * row);
    
    for(i = 0; i < row; i++){
        maze[i] = (int*)malloc(sizeof(int) * column);
    }
    
    //初期化
    for(i = 0; i < row; i++){
        for(j = 0; j < column; j++){
            maze[i][j] = 0;
        }
    }
    
    /* 棒倒法 */
    //外壁の設定
    for(i = 0; i < column; i++){
        maze[0][i] = 1; // 最初の行
    }
    for(i = 0; i < column; i++){
        maze[row-1][i] = 1; // 最後の列
    }
    for(i = 0; i < row; i++){
        maze[i][0] = 1; // 最初の列
    }
    for(i = 0; i < row; i++){
        maze[i][column-1] = 1; // 最後の列
    }
    
    //奇数のXとY座標がともに非ゼロ整数である地点で棒倒し
    for(i = 1; i <= row/2-1; i++){
        for(j = 1; j <= column/2-1; j++){
            barDefeat(maze, i*2, j*2);
        }
    }
      
    return maze;
}

int getDirection(int** maze, int row, int column){
    //棒を倒す方向を返す (3: 上 2: 下 1: 左 0; 右)
    int status[4] = {0, 0, 0, 0};
    int street_num = 0; //上下左右に存在する壁の個数
    int random, direction;
    
    //上下左右の壁状況を確認する
    if(maze[row-1][column] == 0){
        //上壁なし
        status[3] = 1;
        street_num++;
    }
    if(maze[row+1][column] == 0){
        //下壁なし
        status[2] = 1;
        street_num++;
    }
    if(maze[row][column-1] == 0){
        //左壁なし
        status[1] = 1;
        street_num++;
    }
    if(maze[row][column+1] == 0){
        //右壁なし
        status[0] = 1;
        street_num++;
    }
    
    if(row != 2 && status[3] == 1){
        //最初の段以外は上方向には倒すことができないので上に壁がなくとも候補から除外する
        street_num--;
        status[3] = 0; //上方向には仮想的に壁を設定しておく
    }
    // 上下左右のいずれかの道に棒を倒す
    random = rand() % street_num;
    
    direction = 0;
    while(1){
        if(status[direction] == 1 && random == 0){
            //この道をふさぐ
            break;
        }
        else if(status[direction] == 1 && random != 0){
            //この道はふさがない
            direction++;
            random--;
        }
        else{
            //もともと壁なのでスキップ
            direction++;
        }
    }
    return direction;
    
}

void barDefeat(int** maze, int row, int column){
    
    int direction;
    //この座標を壁に設定
    maze[row][column] = 1;
    
    direction = getDirection(maze, row, column);
    switch (direction) {
        case 3:
            //ふさぐのは上の道
            maze[row-1][column] = 1;
            break;
        case 2:
            //ふさぐのは下の道
            maze[row+1][column] = 1;
            break;
        case 1:
            //ふさぐのは左の道
            maze[row][column-1] = 1;
            break;
        case 0:
            //ふさぐのは右の道
            maze[row][column+1] = 1;
            break;
    }
}
