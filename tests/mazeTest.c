#include <stdio.h>
#include "../maze.h"

char WALL='#';
char STREET=' ';

void display(int** maze, int row, int column){
    
    int i, j;
    for(i = 0; i < row; i++){
        for(j = 0; j < column; j++){
            if(maze[i][j] == 1){
                printf("%c", WALL);
            }
            else{
                printf("%c", STREET);
            }
        }
        printf("\n");
    }
}


int main(int argc, char *argv[])
{
  int **maze = makeMaze(11, 21);
  display(maze, 11, 21);

  int **maze2 = makeMaze(31, 51);
  display(maze2, 31, 51);

}
