#include <iostream>
#include <vector>
#include <bitset>
#include <stdint.h>
#include <string>
#include <math.h>
#include <stdio.h>
#include <stack>
#include <unordered_map>

using namespace std;

class Chess
{
    //bitboard 0 - white king, 1 - white queen, 2 - white rook, 3 - white bishop,
    //4 - white knight, 5 - white pawn, 6 - black king, 7 - black queen,
    //8 - black rook, 9 - black bishop, 10 - black knight, 11 black pawn

    //bool color white-0, black-1

    vector<uint64_t> bitboard;
    vector<uint64_t> legal_moves_white;
    vector<uint64_t> legal_moves_black;
    vector<uint64_t> legalMoves;

    unordered_map<string, uint64_t> moves;
    /*Will be a set of diagonal and straight lines shooting from king position
    to edge of board or for diagonal(bishop/queen) and straight(rook/queen) it
    locks friendly pieces that are in the region from leaving the path its on*/
    vector<uint64_t> king_safety_lines_black;
    vector<uint64_t> king_safety_lines_white;
    /*All positions the black/white king can not move*/
    //for kingless zones every move we need to count how
    //many pieces affect that square so when we remove one,
    //we know whether or not we need to recalculate the field.
    uint64_t kingless_zones_black;
    uint64_t kingless_zones_white;
    /*The following store which cells need to be recalculated on each move.
    The vector idx is which diagonal or which straight. move clockwise*/
    vector<uint64_t> blocks_diagonal;
    //0 upper left 
    uint64_t blocks_knight;
    //use a mask of knight moves to find knight
    vector<uint64_t> blocks_straight;
    // 0 upper
    vector<uint64_t> affects_pawn_diagonal;
    // 0 upper left corner
    uint64_t blocks_king;
    //a mask of king moves to find king
    vector<int> kingSftyCntWhite;
    vector<int> kingSftyCntBlack;

    bool wCanCastle;
    bool wCanQCastle;
    bool bCanCastle;
    bool bCanQCastle;

    //the turn
    bool wturn = true;

    public:
        Chess();
        
        string to_bin(uint64_t);

        uint64_t pieces();

        uint64_t white_pieces();

        uint64_t black_pieces();

        uint64_t knight_legal_moves(uint64_t knight_pos, bool color);

        uint64_t bishop_legal_moves(uint64_t bishop_pos, bool color);

        uint64_t king_legal_moves(uint64_t king_pos, bool color);

        uint64_t queen_legal_moves(uint64_t queen_pos, bool color);

        uint64_t rook_legal_moves(uint64_t rook_pos, bool color);

        uint64_t pawn_legal_moves(uint64_t pawn_pos, bool color);

        vector<uint64_t> update_vec(vector<uint64_t> tou, int idx, uint64_t val);

        void check360(uint64_t to);

        void shootDiags(int diag, uint64_t coord);

        void shootStraights(int straight, uint64_t coord);

        void shootKnights(uint64_t coord);

        void shootPawns(int diag, uint64_t coord);

        void legal_moves();

        string bin_board(uint64_t bin);

        string board_representation();

        void turn();

        void board_test();

        vector<uint64_t> gen_legal_moves(uint64_t pieces);

        bool step(uint64_t from, uint64_t to);

        void startConsole();

        string cypherMove(uint64_t move);

        void init_move_map();
};

class Num2let
{
    public:
        vector<string> let;

        Num2let();

        string get(int x);
};