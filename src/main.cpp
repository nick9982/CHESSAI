#include "../include/chess_game/chess.hpp"

int main(int argc, char **argv)
{
    Chess chess;
    cout << chess.board_representation() << endl;
    try{
        chess.step(2, 262144);
        cout << chess.board_representation() << endl;

        chess.step(2048, 524288);
        cout << chess.board_representation() << endl;

        chess.step(262144, 2);
        cout << chess.board_representation() << endl;
    } catch(exception e)
    {
        cout << "exc4" <<  endl;
    }
    
    return 0;
}
