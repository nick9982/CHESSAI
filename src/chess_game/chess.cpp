#include "../../include/chess_game/chess.hpp"

Chess::Chess()
{
    //w
    this->bitboard.push_back(8); //king
    this->bitboard.push_back(16); //queen
    this->bitboard.push_back(129); //rooks
    this->bitboard.push_back(36); //bishops
    this->bitboard.push_back(66); //knights
    this->bitboard.push_back(65280); //pawns
    //b
    this->bitboard.push_back(576460752303423488); //king
    this->bitboard.push_back(1152921504606846976); //queen
    this->bitboard.push_back(9295429630892703744); //rook
    this->bitboard.push_back(2594073385365405696); //bishop
    this->bitboard.push_back(4755801206503243776); //knights
    this->bitboard.push_back(71776119061217280); //pawns

    for(uint i = 0; i < 8; i++)
    {
        this->king_safety_lines_white.push_back(0);
        this->king_safety_lines_black.push_back(0);
    }
    for(uint i = 0; i < 64; i++)
    {
        this->kingSftyCntBlack.push_back(0);
        this->kingSftyCntWhite.push_back(0);
    }

    for(uint i = 0;  i < 4; i++)
    {
        this->blocks_diagonal.push_back(0);
        this->blocks_straight.push_back(0);
        this->affects_pawn_diagonal.push_back(0);
    }
    this->blocks_knight = 0;
    this->blocks_king = 0;

    this->wCanCastle = true;
    this->wCanQCastle = true;
    this->bCanCastle = true;
    this->bCanQCastle = true;

    this->init_move_map();

    this->legal_moves();
    
}

uint64_t Chess::pieces()
{
    uint64_t board = 0;
    for(uint i = 0; i < this->bitboard.size(); i++)
    {
        board |= this->bitboard[i];
    }

    return board;
}

uint64_t Chess::white_pieces()
{
    uint64_t board = 0;
    for(uint i = 0; i < 6; i++)
    {
        board |= this->bitboard[i];
    }

    return board;
}

uint64_t Chess::black_pieces()
{
    uint64_t board = 0;
    for(uint i = 0; i < 6; i++)
    {
        board |= this->bitboard[i+6];
    }

    return board;
}

void Chess::board_test()
{
    this->bitboard[0] = 0; //king
    this->bitboard[1] = 0; //queen
    this->bitboard[2] = 0; //rooks
    this->bitboard[3] = 0; //bishops
    this->bitboard[4] = 1; //knights
    this->bitboard[5] = 0; //pawns
    //b
    this->bitboard[6] = 0; //king
    this->bitboard[7] = 0; //queen
    this->bitboard[8] = 0; //rook
    this->bitboard[9] = 0; //bishop
    this->bitboard[10] = 0; //knights
    this->bitboard[11] = 0; //pawns
}

uint64_t Chess::knight_legal_moves(uint64_t knight_pos, bool color)
{
    uint64_t res = 0;
    if((!color && (this->bitboard[4] & knight_pos) != res) 
    || (color && (this->bitboard[10] & knight_pos) != res))
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(knight_pos == pow(2, cnt)) found = true;
            cnt++;
        }
        int x = (cnt%8)-1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }
        stack<int> idxs;
        if(x > 1 && y < 7) idxs.push((y+1)*8+x-2);
        if(x > 0 && y < 6) idxs.push((y+2)*8+x-1);
        if(x < 7 && y < 6) idxs.push((y+2)*8+x+1);
        if(x < 6 && y < 7) idxs.push((y+1)*8+x+2);
        if(x < 6 && y > 0) idxs.push((y-1)*8+x+2);
        if(x < 7 && y > 1) idxs.push((y-2)*8+x+1);
        if(x > 0 && y > 1) idxs.push((y-2)*8+x-1);
        if(x > 1 && y > 0) idxs.push((y-1)*8+x-2);

        uint64_t allMoves = 0;
        while(!idxs.empty())
        {
            uint64_t move = pow(2, idxs.top());
            allMoves |= move;
            if(!color)
            {
                this->kingless_zones_black |= move;
                this->kingSftyCntBlack[idxs.top()]++;
            }
            else
            {
                this->kingless_zones_white |= move;
                this->kingSftyCntWhite[idxs.top()]++;
            }
            idxs.pop();
        }

        uint64_t pieces = 0;
        if(!color) pieces = this->white_pieces();
        else pieces = this->black_pieces();
        uint64_t tmp = allMoves ^ pieces;
        uint64_t blocked = allMoves & pieces;
        this->blocks_knight |= blocked;
        allMoves &= tmp;
        res = allMoves;
    }
    return res;
}

uint64_t Chess::bishop_legal_moves(uint64_t bishop_pos, bool color)
{
    uint64_t allMoves = 0;
    if(!color && (this->bitboard[3] & bishop_pos) != 0
    || color && (this->bitboard[9] & bishop_pos) != 0)
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(bishop_pos == pow(2, cnt)) found = true;
            cnt++;
        }

        int x = (cnt % 8) - 1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }

        stack<uint64_t> idxs;
        uint64_t white_pieces = this->white_pieces();
        uint64_t black_pieces = this->black_pieces();
        bitset<4> paths = 15;
        bitset<4> com = 0;
        int depth = 1;
        if(!color)
        {
            while((paths | com) != com)
            {
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y+depth)*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[2] |= r;
                                bitset<4> npath = 7;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[2] |= r;
                            bitset<4> npath = 7;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 7;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y+depth)*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[3] |= r;
                                bitset<4> npath = 11;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[3] |= r;
                            bitset<4> npath = 11;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 11;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y>=depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[1] |= r;
                                bitset<4> npath = 13;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[1] |= r;
                            bitset<4> npath = 13;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 13;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[0] |= r;
                                bitset<4> npath = 14;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[0] |= r;
                            bitset<4> npath = 14;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 14;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }
        }
        else
        {
            while((paths | com) != com)
            {
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[2] |= r;
                                bitset<4> npath = 7;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[2] |= r;
                            bitset<4> npath = 7;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 7;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[3] |= r;
                                bitset<4> npath = 11;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[3] |= r;
                            bitset<4> npath = 11;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 11;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y>=depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[1] |= r;
                                bitset<4> npath = 13;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[1] |= r;
                            bitset<4> npath = 13;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 13;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                                this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[0] |= r;
                                bitset<4> npath = 14;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[0] |= r;
                            bitset<4> npath = 14;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 14;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }
        }

        while(!idxs.empty())
        {
            allMoves |= idxs.top();
            idxs.pop();
        }

    }
    return allMoves;
}

uint64_t Chess::king_legal_moves(uint64_t king_pos, bool color)
{
    if(!color)
    {
        for(uint i = 0; i < 8; i++)
        {
            this->king_safety_lines_white[i] = 0;
        }
    }
    else
    {
        for(uint i = 0; i < 8; i++)
        {
            this->king_safety_lines_black[i] = 0;
        }
    }
    uint64_t allMoves = 0;
    if(!color && (this->bitboard[0] & king_pos) != 0
    || color && (this->bitboard[6] & king_pos) != 0)
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(king_pos == pow(2, 64)) found = true;
            cnt++;
        }

        int x = (cnt%8)-1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }
        stack<uint64_t> newMoves;
        if(x > 0 && y > 0) newMoves.push(pow(2, (y-1)*8+x-1));
        if(x > 0) newMoves.push(pow(2, y*8+x-1));
        if(y > 0) newMoves.push(pow(2, (y-1)*8+x));
        if(x < 7) newMoves.push(pow(2, y*8+x+1));
        if(y < 7) newMoves.push(pow(2, (y+1)*8+x));
        if(x < 7 && y > 0) newMoves.push(pow(2, (y-1)*8+x+1));
        if(x > 0 && y < 7) newMoves.push(pow(2, (y+1)*8+x-1));
        if(x < 7 && y < 7) newMoves.push(pow(2, (y+1)*8+x+1));

        while(!newMoves.empty())
        {
            allMoves |= newMoves.top();
            if(!color)
            {
                this->kingless_zones_black |= newMoves.top();
                this->kingSftyCntBlack[log2(newMoves.top())]++;
            }
            else
            {
                this->kingless_zones_white |= newMoves.top();
                this->kingSftyCntWhite[log2(newMoves.top())]++;               
            }
            newMoves.pop();
        }

        uint64_t pieces = 0;
        if(!color) pieces = this->white_pieces();
        else pieces = this->black_pieces();
        uint64_t tmp = allMoves ^ pieces;
        uint64_t blocked = allMoves & pieces;
        this->blocks_king |= blocked;
        allMoves &= tmp;

        stack<uint64_t> idxs;
        uint64_t kingSafetyLine = 0;
        uint64_t white_pieces = this->white_pieces();
        uint64_t black_pieces = this->black_pieces();
        u_char paths = 255;
        u_char com = 0;
        vector<int> counts(8, 0);
        vector<stack<uint64_t>> prevPoints(8);
        int depth = 1;
        if(!color)
        {
            while((paths | com) != 0)
            {
                com = 128;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[0] == 0)
                            {
                                counts[0]++;
                                prevPoints[0].push(r);
                            }
                            else
                            {
                                u_char npath = 127;
                                paths &= npath;
                                while(!prevPoints[0].empty())
                                {
                                    prevPoints[0].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[0] == 1)
                            {
                                u_char npath = 127;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[0].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[0].empty())
                                    {
                                        prevPoints[0].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[0].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 127;
                        paths &= npath;
                        while(!prevPoints[0].empty())
                        {
                            prevPoints[0].pop();
                        }
                    }
                }
                com = 64;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[1] == 0)
                            {
                                counts[1]++;
                                prevPoints[1].push(r);
                            }
                            else
                            {
                                u_char npath = 191;
                                paths &= npath;
                                while(!prevPoints[1].empty())
                                {
                                    prevPoints[1].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[1] == 1)
                            {
                                u_char npath = 191;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[1].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[1].empty())
                                    {
                                        prevPoints[1].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[1].push(r);
                        }                        
                    }
                    else
                    {
                        u_char npath = 191;
                        paths &= npath;
                        while(!prevPoints[1].empty())
                        {
                            prevPoints[1].pop();
                        }
                    }
                }
                com  = 32;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[2] == 0)
                            {
                                counts[2]++;
                                prevPoints[2].push(r);
                            }
                            else
                            {
                                u_char npath = 223;
                                paths &= npath;
                                while(!prevPoints[2].empty())
                                {
                                    prevPoints[2].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[2] == 1)
                            {
                                u_char npath = 223;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[2].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[2].empty())
                                    {
                                        prevPoints[2].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[2].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 223;
                        paths &= npath;
                        while(!prevPoints[2].empty())
                        {
                            prevPoints[2].pop();
                        }
                    }
                }
                com = 16;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[3] == 0)
                            {
                                counts[3]++;
                                prevPoints[3].push(r);
                            }
                            else
                            {
                                u_char npath = 239;
                                paths &= npath;
                                while(!prevPoints[3].empty())
                                {
                                    prevPoints[3].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[3] == 1)
                            {
                                u_char npath = 239;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[3].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[3].empty())
                                    {
                                        prevPoints[3].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[3].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 239;
                        paths &= npath;
                        while(!prevPoints[3].empty())
                        {
                            prevPoints[3].pop();
                        }
                    }
                }
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[4] == 0)
                            {
                                counts[4]++;
                                prevPoints[4].push(r);
                            }
                            else
                            {
                                u_char npath = 247;
                                paths &= npath;
                                while(!prevPoints[4].empty())
                                {
                                    prevPoints[4].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[4] == 1)
                            {
                                u_char npath = 247;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[4].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[4].empty())
                                    {
                                        prevPoints[4].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[4].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 247;
                        paths &= npath;
                        while(!prevPoints[4].empty())
                        {
                            prevPoints[4].pop();
                        }
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[5] == 0)
                            {
                                counts[5]++;
                                prevPoints[5].push(r);
                            }
                            else
                            {
                                u_char npath = 251;
                                paths &= npath;
                                while(!prevPoints[5].empty())
                                {
                                    prevPoints[5].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[5] == 1)
                            {
                                u_char npath = 251;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[5].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[5].empty())
                                    {
                                        prevPoints[5].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[5].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 251;
                        paths &= npath;
                        while(!prevPoints[5].empty())
                        {
                            prevPoints[5].pop();
                        }
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[6] == 0)
                            {
                                counts[6]++;
                                prevPoints[6].push(r);
                            }
                            else
                            {
                                u_char npath = 253;
                                paths &= npath;
                                while(!prevPoints[6].empty())
                                {
                                    prevPoints[6].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[6] == 1)
                            {
                                u_char npath = 253;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[6].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[6].empty())
                                    {
                                        prevPoints[6].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[6].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 253;
                        paths &= npath;
                        while(!prevPoints[6].empty())
                        {
                            prevPoints[6].pop();
                        }
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y >= depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & white_pieces) != 0)
                        {
                            if(counts[7] == 0)
                            {
                                counts[7]++;
                                prevPoints[7].push(r);
                            }
                            else
                            {
                                u_char npath = 254;
                                paths &= npath;
                                while(!prevPoints[7].empty())
                                {
                                    prevPoints[7].pop();
                                }
                            }
                        }
                        else if((r & black_pieces) != 0)
                        {
                            if(counts[7] == 1)
                            {
                                u_char npath = 254;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[7].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[7].empty())
                                    {
                                        prevPoints[7].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[7].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 254;
                        paths &= npath;
                        while(!prevPoints[7].empty())
                        {
                            prevPoints[7].pop();
                        }
                    }
                }
                com = 0;
                depth++;
            }
        }
        else
        {
            while((paths | com) != 0)
            {
                com = 128;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[0] == 0)
                            {
                                counts[0]++;
                                prevPoints[0].push(r);
                            }
                            else
                            {
                                u_char npath = 127;
                                paths &= npath;
                                while(!prevPoints[0].empty())
                                {
                                    prevPoints[0].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[0] == 1)
                            {
                                u_char npath = 127;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[0].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[0].empty())
                                    {
                                        prevPoints[0].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[0].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 127;
                        paths &= npath;
                        while(!prevPoints[0].empty())
                        {
                            prevPoints[0].pop();
                        }
                    }
                }
                com = 64;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[1] == 0)
                            {
                                counts[1]++;
                                prevPoints[1].push(r);
                            }
                            else
                            {
                                u_char npath = 191;
                                paths &= npath;
                                while(!prevPoints[1].empty())
                                {
                                    prevPoints[1].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[1] == 1)
                            {
                                u_char npath = 191;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[1].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[1].empty())
                                    {
                                        prevPoints[1].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[1].push(r);
                        }                        
                    }
                    else
                    {
                        u_char npath = 191;
                        paths &= npath;
                        while(!prevPoints[1].empty())
                        {
                            prevPoints[1].pop();
                        }
                    }
                }
                com  = 32;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[2] == 0)
                            {
                                counts[2]++;
                                prevPoints[2].push(r);
                            }
                            else
                            {
                                u_char npath = 223;
                                paths &= npath;
                                while(!prevPoints[2].empty())
                                {
                                    prevPoints[2].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[2] == 1)
                            {
                                u_char npath = 223;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[2].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[2].empty())
                                    {
                                        prevPoints[2].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[2].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 223;
                        paths &= npath;
                        while(!prevPoints[2].empty())
                        {
                            prevPoints[2].pop();
                        }
                    }
                }
                com = 16;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[3] == 0)
                            {
                                counts[3]++;
                                prevPoints[3].push(r);
                            }
                            else
                            {
                                u_char npath = 239;
                                paths &= npath;
                                while(!prevPoints[3].empty())
                                {
                                    prevPoints[3].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[3] == 1)
                            {
                                u_char npath = 239;
                                paths &= npath;
                                if(((r & this->bitboard[9]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[3].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[3].empty())
                                    {
                                        prevPoints[3].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[3].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 239;
                        paths &= npath;
                        while(!prevPoints[3].empty())
                        {
                            prevPoints[3].pop();
                        }
                    }
                }
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[4] == 0)
                            {
                                counts[4]++;
                                prevPoints[4].push(r);
                            }
                            else
                            {
                                u_char npath = 247;
                                paths &= npath;
                                while(!prevPoints[4].empty())
                                {
                                    prevPoints[4].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[4] == 1)
                            {
                                u_char npath = 247;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[4].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[4].empty())
                                    {
                                        prevPoints[4].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[4].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 247;
                        paths &= npath;
                        while(!prevPoints[4].empty())
                        {
                            prevPoints[4].pop();
                        }
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[5] == 0)
                            {
                                counts[5]++;
                                prevPoints[5].push(r);
                            }
                            else
                            {
                                u_char npath = 251;
                                paths &= npath;
                                while(!prevPoints[5].empty())
                                {
                                    prevPoints[5].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[5] == 1)
                            {
                                u_char npath = 251;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[5].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[5].empty())
                                    {
                                        prevPoints[5].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[5].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 251;
                        paths &= npath;
                        while(!prevPoints[5].empty())
                        {
                            prevPoints[5].pop();
                        }
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[6] == 0)
                            {
                                counts[6]++;
                                prevPoints[6].push(r);
                            }
                            else
                            {
                                u_char npath = 253;
                                paths &= npath;
                                while(!prevPoints[6].empty())
                                {
                                    prevPoints[6].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[6] == 1)
                            {
                                u_char npath = 253;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[6].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[6].empty())
                                    {
                                        prevPoints[6].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[6].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 253;
                        paths &= npath;
                        while(!prevPoints[6].empty())
                        {
                            prevPoints[6].pop();
                        }
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y >= depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & black_pieces) != 0)
                        {
                            if(counts[7] == 0)
                            {
                                counts[7]++;
                                prevPoints[7].push(r);
                            }
                            else
                            {
                                u_char npath = 254;
                                paths &= npath;
                                while(!prevPoints[7].empty())
                                {
                                    prevPoints[7].pop();
                                }
                            }
                        }
                        else if((r & white_pieces) != 0)
                        {
                            if(counts[7] == 1)
                            {
                                u_char npath = 254;
                                paths &= npath;
                                if(((r & this->bitboard[8]) != 0) || (r & this->bitboard[7]) != 0)
                                {
                                    prevPoints[7].push(r);
                                }
                                else
                                {
                                    while(!prevPoints[7].empty())
                                    {
                                        prevPoints[7].pop();
                                    }
                                }
                            }
                        }
                        else
                        {
                            prevPoints[7].push(r);
                        }
                    }
                    else
                    {
                        u_char npath = 254;
                        paths &= npath;
                        while(!prevPoints[7].empty())
                        {
                            prevPoints[7].pop();
                        }
                    }
                }
                com = 0;
                depth++;
            }
        }

        for(uint i = 0; i < prevPoints.size(); i++)
        {
            if(!color)
            {
                while(!prevPoints[i].empty())
                {
                    this->king_safety_lines_white[i] |= prevPoints[i].top();
                    prevPoints[i].pop();
                }   
            }
            else
            {
                while(!prevPoints[i].empty())
                {
                    this->king_safety_lines_black[i] |= prevPoints[i].top();
                    prevPoints[i].pop();
                }
            }
        }
    }

    if(!color) allMoves ^= kingless_zones_white;
    else allMoves ^= kingless_zones_black;

    if(!color)
    {
        if(this->wCanCastle)
        {
            if(((this->pieces() & 6) == 0) && ((this->kingless_zones_white & 6) == 0))
                allMoves |= 2;
        }
        if(this->wCanQCastle)
        {
            if(((this->pieces() & 112) == 0) && ((this->kingless_zones_white & 112) == 0))
                allMoves |= 32;
        }
    }
    else
    {
        if(this->bCanCastle)
        {
            if(((this->pieces() & 432345564227567616) == 0) && ((this->kingless_zones_black & 432345564227567616) == 0))
                allMoves |= 144115188075855872;
        }
        if(this->bCanQCastle)
        {
            if(((this->pieces() & 8070450532247928832) == 0) && ((this->kingless_zones_black & 8070450532247928832) == 0))
                allMoves |= 2305843009213693952;
        }
    }

    return allMoves;
}

uint64_t Chess::rook_legal_moves(uint64_t rook_pos, bool color)
{
    uint64_t allMoves = 0;
    if(!color && (this->bitboard[2] & rook_pos) != 0
    || color && (this->bitboard[8] & rook_pos) != 0)
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(rook_pos == pow(2, cnt)) found = true;
            cnt++;
        }

        int x = (cnt%8)-1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }
        stack<uint64_t> idxs;
        uint64_t white_pieces = this->white_pieces();
        uint64_t black_pieces = this->black_pieces();
        bitset<4> paths = 15;
        bitset<4> com = 0;
        int depth = 1;
        if(!color)
        {
            while((paths | com) != com)
            {
                com = 8;
                if((paths & com) != 0)
                {
                    if(x+depth <= 7)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[y*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[1] |= r;
                                bitset<4> npath = 7;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[1] |= r;
                            bitset<4> npath = 7;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 7;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x-depth >= 0)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[y*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[3] |= r;
                                bitset<4> npath = 11;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[3] |= r;
                            bitset<4> npath = 11;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 11;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(y+depth <= 7)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y+depth)*8+x]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[2] |= r;
                                bitset<4> npath = 13;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[2] |= r;
                            bitset<4> npath = 13;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 13;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y-depth >= 0)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[0] |= r;
                                bitset<4> npath = 14;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[0] |= r;
                            bitset<4> npath = 14;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 14;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }
        }
        else
        {
            while((paths | com) != com)
            {
                com = 8;
                if((paths & com) != 0)
                {
                    if(x+depth <= 7)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[y*8+x+depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[1] |= r;
                                bitset<4> npath = 7;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[1] |= r;
                            bitset<4> npath = 7;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 7;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x-depth >= 0)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[y*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[3] |= r;
                                bitset<4> npath = 11;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[3] |= r;
                            bitset<4> npath = 11;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 11;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(y+depth <= 7)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y+depth)*8+x]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[2] |= r;
                                bitset<4> npath = 13;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[2] |= r;
                            bitset<4> npath = 13;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 13;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y-depth >= 0)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[0] |= r;
                                bitset<4> npath = 14;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[0] |= r;
                            bitset<4> npath = 14;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        bitset<4> npath = 14;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }            
        }
        while(!idxs.empty())
        {
            allMoves |= idxs.top();
            idxs.pop();
        }
    }

    return allMoves;
}

uint64_t Chess::pawn_legal_moves(uint64_t pawn_pos, bool color)
{
    uint64_t allMoves = 0;
    if(!color && (this->bitboard[5] & pawn_pos) != 0
    || color && (this->bitboard[11] & pawn_pos) != 0)
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(pawn_pos == pow(2, cnt)) found = true;
            cnt++;
        }

        int x = (cnt%8)-1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }
        uint64_t white_pieces = this->white_pieces();
        uint64_t black_pieces = this->black_pieces();
        stack<uint64_t> idxs;
        if(!color)
        {
            if(y < 7)
            {
                uint64_t in_front = pow(2, (y+1)*8+x);
                uint64_t upper_left = pow(2, (y+1)*8+x-1);
                uint64_t upper_right = pow(2, (y+1)*8+x+1);
                if((in_front & (black_pieces | white_pieces)) == 0)
                {
                    this->blocks_straight[2] |= in_front;
                    idxs.push(in_front);
                }
                if((upper_left & black_pieces) != 0)
                {
                    this->affects_pawn_diagonal[2] |= upper_left;
                    idxs.push(upper_left);
                }
                if((upper_right & black_pieces) != 0)
                {
                    this->affects_pawn_diagonal[3] |= upper_right;
                    idxs.push(upper_right);
                }
                if(x > 0)
                {
                    this->kingless_zones_black |= upper_left;
                    this->kingSftyCntBlack[(y-1)*8+x-1]++;
                }
                if(x < 7)
                {
                    this->kingless_zones_black |= upper_right;
                    this->kingSftyCntBlack[(y-1)*8+x+1]++;
                }
            }
        }
        else
        {
            if(y > 0)
            {
                uint64_t in_front = pow(2, (y-1)*8+x);
                uint64_t upper_left = pow(2, (y-1)*8+x-1);
                uint64_t upper_right = pow(2, (y-1)*8+x+1);
                if((in_front & (white_pieces | black_pieces)) == 0)
                {
                    this->blocks_straight[0] |= in_front;
                    idxs.push(in_front);
                }
                if((upper_left & white_pieces) != 0)
                {
                    this->affects_pawn_diagonal[0] |= upper_left;
                    idxs.push(upper_left);
                }
                if((upper_right & white_pieces) != 0)
                {
                    this->affects_pawn_diagonal[1] |= upper_left;
                    idxs.push(upper_right);
                }
                if(x > 0)
                {
                    this->kingless_zones_white |= upper_left;
                    this->kingSftyCntWhite[(y-1)*8+x-1]++;
                }
                if(x < 7)
                {
                    this->kingless_zones_white |= upper_right;
                    this->kingSftyCntWhite[(y-1)*8+x+1]++;
                }
            }
        }

        while(!idxs.empty())
        {
            allMoves |= idxs.top();
            idxs.pop();
        }
    }
    return allMoves;
}

uint64_t Chess::queen_legal_moves(uint64_t queen_pos, bool color)
{
    uint64_t allMoves = 0;
    if(!color && (this->bitboard[1] & queen_pos) != 0
    || color && (this->bitboard[7] & queen_pos) != 0)
    {
        bool found = false;
        int cnt = 0;
        while(!found && cnt < 64)
        {
            if(queen_pos == pow(2, cnt)) found = true;
            cnt++;
        }

        int x = (cnt % 8) - 1;
        int y = floor(cnt/8);
        if(cnt%8 == 0)
        {
            y--;
            x = 7;
        }
        stack<uint64_t> idxs;
        uint64_t white_pieces = this->white_pieces();
        uint64_t black_pieces = this->black_pieces();
        u_char paths = 255;
        u_char com = 0;
        int depth = 1;
        if(!color)
        {
            while((paths | com) != com)
            {
                com = 128;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y+depth)*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[2] |= r;
                                u_char npath = 127;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[2] |= r;
                            u_char npath = 127;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 127;
                        paths &= npath;
                    }
                }
                com = 64;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y+depth)*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[3] |= r;
                                u_char npath = 191;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[3] |= r;
                            u_char npath = 191;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 191;
                        paths &= npath;
                    }
                }
                com = 32;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y>=depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[1] |= r;
                                u_char npath = 223;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[1] |= r;
                            u_char npath = 223;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 223;
                        paths &= npath;
                    }
                }
                com = 16;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_diagonal[0] |= r;
                                u_char npath = 239;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[0] |= r;
                            u_char npath = 239;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 239;
                        paths &= npath;
                    }
                }
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x+depth]++;
                            if((r & black_pieces) != 0)
                            {
                                u_char npath = 247;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            u_char npath = 247;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 247;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[y*8+x-depth]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[3] |= r;
                                u_char npath = 251;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[3] |= r;
                            u_char npath = 251;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 251;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(y <= 7-depth)
                    {
                        uint64_t r = pow(2, ((y+depth)*8+x));
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[((y+depth)*8+x)]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[2] |= r;
                                u_char npath = 253;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[2] |= r;
                            u_char npath = 253;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 253;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x);
                        if((r & white_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_black |= r;
                            this->kingSftyCntBlack[(y-depth)*8+x]++;
                            if((r & black_pieces) != 0)
                            {
                                this->blocks_straight[0] |= r;
                                u_char npath = 254;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[0] |= r;
                            u_char npath = 254;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 254;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }
        }
        else
        {
            while((paths | com) != com)
            {
                com = 128;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y+depth)*8+x+depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[2] |= r;
                                u_char npath = 127;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[2] |= r;
                            u_char npath = 127;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 127;
                        paths &= npath;
                    }
                }
                com = 64;
                if((paths & com) != 0)
                {
                    if(x >= depth && y <= 7-depth)
                    {
                        uint64_t r = pow(2, (y+depth)*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y+depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[3] |= r;
                                u_char npath = 191;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[3] |= r;
                            u_char npath = 191;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 191;
                        paths &= npath;
                    }
                }
                com = 32;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth && y>=depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x+depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[1] |= r;
                                u_char npath = 223;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[1] |= r;
                            u_char npath = 223;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 223;
                        paths &= npath;
                    }
                }
                com = 16;
                if((paths & com) != 0)
                {
                    if(x >= depth && y >= depth)
                    {
                        int64_t r = pow(2, (y-depth)*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_diagonal[0] |= r;
                                u_char npath = 239;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_diagonal[0] |= r;
                            u_char npath = 239;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 239;
                        paths &= npath;
                    }
                }
                com = 8;
                if((paths & com) != 0)
                {
                    if(x <= 7-depth)
                    {
                        uint64_t r = pow(2, y*8+x+depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[y*8+x+depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[1] |= r;
                                u_char npath = 247;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[1] |= r;
                            u_char npath = 247;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 247;
                        paths &= npath;
                    }
                }
                com = 4;
                if((paths & com) != 0)
                {
                    if(x >= depth)
                    {
                        uint64_t r = pow(2, y*8+x-depth);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[y*8+x-depth]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[3] |= r;
                                u_char npath = 251;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[3] |= r;
                            u_char npath = 251;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 251;
                        paths &= npath;
                    }
                }
                com = 2;
                if((paths & com) != 0)
                {
                    if(y <= 7-depth)
                    {
                        uint64_t r = pow(2, ((y+depth)*8+x));
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[((y+depth)*8+x)]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[2] |= r;
                                u_char npath = 253;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[2] |= r;
                            u_char npath = 253;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 253;
                        paths &= npath;
                    }
                }
                com = 1;
                if((paths & com) != 0)
                {
                    if(y >= depth)
                    {
                        uint64_t r = pow(2, (y-depth)*8+x);
                        if((r & black_pieces) == 0)
                        {
                            idxs.push(r);
                            this->kingless_zones_white |= r;
                            this->kingSftyCntWhite[(y-depth)*8+x]++;
                            if((r & white_pieces) != 0)
                            {
                                this->blocks_straight[0] |= r;
                                u_char npath = 254;
                                paths &= npath;
                            }
                        }
                        else
                        {
                            this->blocks_straight[0] |= r;
                            u_char npath = 254;
                            paths &= npath;
                        }
                    }
                    else
                    {
                        u_char npath = 254;
                        paths &= npath;
                    }
                }
                com = 0;
                depth++;
            }
        }

        while(!idxs.empty())
        {
            allMoves |= idxs.top();
            idxs.pop();
        }
    }

    return allMoves;
}

void Chess::legal_moves()
{
    uint64_t white_pieces = this->white_pieces();
    uint64_t black_pieces = this->black_pieces();

    vector<uint64_t> white_moves = this->gen_legal_moves(white_pieces);
    vector<uint64_t> black_moves = this->gen_legal_moves(black_pieces);

    this->legal_moves_white = white_moves;
    this->legal_moves_black = black_moves;

    this->legalMoves.insert(this->legalMoves.begin(), this->legal_moves_white.begin(), this->legal_moves_white.end());
    this->legalMoves.insert(this->legalMoves.end(), this->legal_moves_black.begin(), this->legal_moves_black.end());
}

vector<uint64_t> Chess::gen_legal_moves(uint64_t pieces)
{
    vector<uint64_t> result;
    int king_pos = -1;
    uint64_t king = 0;
    int king_color = 0;
    uint king_position = 0;
    while(pieces != 0)
    {
        uint64_t m = 9223372036854775808;
        uint64_t temp = pieces;
        uint64_t res = 9223372036854775808;
        int cnt = 0;
        while(res >= 9223372036854775808)
        {
            res = (m ^ temp);
            temp <<= 1;
            cnt++;
        }
        if(cnt == 64) pieces = 0;
        else
        {
            pieces <<= cnt;
            pieces >>= cnt;
        }
        king_pos++;
        uint64_t idx = pow(2, 64-cnt);
        for(uint i = 0; i < 12; i++)
        {
            uint64_t matches = this->bitboard[i] & idx;
            if(matches != 0)
            {
                int x = 1;
                switch(i)
                {
                    case 0:
                        x = 0;
                    case 6:
                        king_position = king_pos;
                        king = idx;
                        king_color = x;
                        break;
                    case 1:
                        x=0;
                    case 7:
                        result.push_back(this->queen_legal_moves(idx, x));
                        break;
                    case 2:
                        x=0;
                    case 8:
                        result.push_back(this->rook_legal_moves(idx, x));
                        break;
                    case 3:
                        x = 0;
                    case 9:
                        result.push_back(this->bishop_legal_moves(idx, x));
                        break;
                    case 4:
                        x = 0;
                    case 10:
                        result.push_back(this->knight_legal_moves(idx, x));
                        break;
                    case 5:
                        x = 0;
                    case 11:
                        result.push_back(this->pawn_legal_moves(idx, x));
                        break;
                }
            }
        }
    }
    result = this->update_vec(result, king_position, this->king_legal_moves(king, king_color));
    return result;
}

vector<uint64_t> Chess::update_vec(vector<uint64_t> tou, int idx, uint64_t val)
{
    vector<uint64_t> result;
    if(idx < tou.size())
    {
        for(uint i = 0; i < idx; i++) result.push_back(tou[i]);
        result.push_back(val);
        for(uint i = idx; i < tou.size(); i++) result.push_back(tou[i]);
        return result;
    }
    else return result;
}

string Chess::to_bin(uint64_t n)
{
    vector<bool> c(64, 0);
    uint64_t tmp = n;

    for(int i = c.size()-1; i >= 0; i--)
    {
        c[i] = tmp%2 == 1;
        if(c[i]) tmp--;
        tmp = tmp / 2;
    }

    string result = "";
    for(int i = 0; i < c.size(); i++)
    {
        result.append(to_string(c[i]));
    }

    return result;
}

string Chess::board_representation()
{
    string board = "";
    int character_counter = 0;

    uint64_t full_board = this->pieces();
    int last_cnt = 0;
    int end_idx = 64;
    while(full_board != 0)
    {
        uint64_t m = 9223372036854775808;
        uint64_t temp = full_board;
        uint64_t res = 9223372036854775808;
        int cnt = 0;
        while(res >= 9223372036854775808)
        {
            res = (m ^ temp);
            temp <<= 1;
            cnt++;
        }
        for(uint i = 0; i < cnt-last_cnt-1; i++)
        {
            board.append("*");
            character_counter++;
            if(character_counter%8 == 0) board.append("\n");
            else board.append(" ");
            end_idx--;
        }

        if(cnt == 64) full_board = 0;
        else
        {
            full_board <<= cnt;
            full_board >>= cnt;
        }

        uint64_t idx = pow(2, 64-cnt);
        for(uint i = 0; i < 12; i++)
        {
            uint64_t matches = this->bitboard[i] & idx;
            if(matches != 0)
            {
                end_idx--;
                character_counter++;
                switch(i)
                {
                    case 0:
                        board.append("K");
                        break;
                    case 1:
                        board.append("Q");
                        break;
                    case 2:
                        board.append("R");
                        break;
                    case 3:
                        board.append("B");
                        break;
                    case 4:
                        board.append("N");
                        break;
                    case 5:
                        board.append("P");
                        break;
                    case 6:
                        board.append("k");
                        break;
                    case 7:
                        board.append("q");
                        break;
                    case 8:
                        board.append("r");
                        break;
                    case 9:
                        board.append("b");
                        break;
                    case 10:
                        board.append("n");
                        break;
                    case 11:
                        board.append("p");
                        break;
                    default:
                        break;
                }
                if(character_counter%8 == 0) board.append("\n");
                else board.append(" ");
            }
        }
        last_cnt = cnt;
    }
    for(int i = 0; i < end_idx; i++)
    {
        board.append("*");
        character_counter++;
        if(character_counter%8 == 0) board.append("\n");
        else board.append(" ");
    }
    return board;
}

string Chess::bin_board(uint64_t bin)
{
    string board = "";
    int character_counter = 0;

    uint64_t full_board = bin;
    int last_cnt = 0;
    int end_idx = 64;
    while(full_board != 0)
    {
        uint64_t m = 9223372036854775808;
        uint64_t temp = full_board;
        uint64_t res = 9223372036854775808;
        int cnt = 0;
        while(res >= 9223372036854775808)
        {
            res = (m ^ temp);
            temp <<= 1;
            cnt++;
        }
        for(uint i = 0; i < cnt-last_cnt-1; i++)
        {
            board.append("0");
            character_counter++;
            if(character_counter%8 == 0) board.append("\n");
            else board.append(" ");
            end_idx--;
        }

        if(cnt == 64) full_board = 0;
        else
        {
            full_board <<= cnt;
            full_board >>= cnt;
        }

        uint64_t idx = pow(2, 64-cnt);
        end_idx--;
        character_counter++;
        board.append("1");
        if(character_counter%8 == 0) board.append("\n");
        else board.append(" ");
        last_cnt = cnt;
    }
    for(int i = 0; i < end_idx; i++)
    {
        board.append("0");
        character_counter++;
        if(character_counter%8 == 0) board.append("\n");
        else board.append(" ");
    } 
    return board;
}

void Chess::turn()
{
    this->wturn = !this->wturn;
}

void Chess::check360(uint64_t to)
{
    for(uint i = 0; i < 4; i++)
    {
        shootDiags(i, to);
        shootStraights(i, to);
        shootPawns(i, to);
    }
    shootKnights(to);
}

void Chess::shootDiags(int diag, uint64_t coord)
{
    int idx = 0;
    uint64_t h = coord;
    while(h >>= 1) idx++;

    int x = (idx%8)-1;
    int y = floor(idx/8);
    if(idx%8 == 0)
    {
        y--;
        x = 7;
    }
    uint64_t white_pieces = this->white_pieces();
    uint64_t black_pieces = this->black_pieces();

    int depth = 1;
    bool found = false;
    stack<int> oldKingless;
    oldKingless.push(idx);
    uint64_t piece = 0;
    if(diag == 0)
    {
        while(x <= 7-depth && y <= 7-depth && !found)
        {
            uint64_t targ = pow(2, (y+depth)*8+x+depth);
            oldKingless.push((y+depth)*8+x+depth);
            if((this->bitboard[3] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[9] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(diag == 1)
    {
        while(x <= 7-depth && y >= depth && !found)
        {
            uint64_t targ = pow(2, (y+depth)*8+x-depth);
            oldKingless.push((y+depth)*8+x-depth);
            if((this->bitboard[3] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[9] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(diag == 2)
    {
        while(x >= depth && y >= depth && !found)
        {
            uint64_t targ = pow(2, (y-depth)*8+x-depth);
            oldKingless.push((y-depth)*8+x-depth);
            if((this->bitboard[3] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[9] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(diag == 3)
    {
        while(x <= 7-depth && y >= depth && !found)
        {
            uint64_t targ = pow(2, (y-depth)*8+x+depth);
            oldKingless.push((y-depth)*8+x+depth);
            if((this->bitboard[3] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[9] & targ) != 0)
            {
                this->bishop_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }

    if(!found)
    {
        while(!oldKingless.empty())
        {
            oldKingless.pop();
        }
    }
    else
    {
        while(!oldKingless.empty())
        {
            uint64_t val = pow(2, oldKingless.top());
            if((piece & black_pieces) != 0)
            {
                this->kingSftyCntWhite[oldKingless.top()]--;
                if(this->kingSftyCntWhite[oldKingless.top()] == 0)
                    this->kingless_zones_white ^= val;
            }
            else
            {
                this->kingSftyCntBlack[oldKingless.top()]--;
                if(this->kingSftyCntBlack[oldKingless.top()] == 0)
                    this->kingless_zones_black ^= val;
            }
            oldKingless.pop();
        }
    }
}

void Chess::shootStraights(int straight, uint64_t coord)
{
    int idx = 0;
    uint64_t h = coord;
    while(h >>= 1) idx++;

    int x = (idx%8)-1;
    int y = floor(idx/8);
    if(idx%8 == 0)
    {
        y--;
        x = 7;
    }

    uint64_t white_pieces = this->white_pieces();
    uint64_t black_pieces = this->black_pieces();

    int depth = 1;
    bool found = false;
    stack<int> oldKingless;
    oldKingless.push(idx);
    uint64_t piece = 0;
    if(straight == 0)
    {
        while(y >= depth && !found)
        {
            uint64_t targ = pow(2, (y-depth)*8+x);
            oldKingless.push((y-depth)*8+x);
            if((this->bitboard[2] & targ) != 0)
            {
                this->rook_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[8] & targ) != 0)
            {
                this->rook_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[5] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[11] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(straight == 1)
    {
        while(x <= 7-depth && !found)
        {
            uint64_t targ = pow(2, y*8+x+depth);
            oldKingless.push(y*8+x+depth);
            if((this->bitboard[2] & targ) != 0)
            {
                this->rook_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[8] & targ) != 0)
            {
                this->rook_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[5] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[11] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(straight == 2)
    {
        while(y <= 7-depth && !found)
        {
            uint64_t targ = pow(2, (y+depth)*8+x);
            oldKingless.push((y+depth)*8+x);
            if((this->bitboard[2] & targ) != 0)
            {
                this->rook_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[8] & targ) != 0)
            {
                this->rook_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[5] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[11] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    else if(straight == 3)
    {
        while(x >= depth && !found)
        {
            uint64_t targ = pow(2, y*8+x-depth);
            oldKingless.push(y*8+x-depth);
            if((this->bitboard[2] & targ) != 0)
            {
                this->rook_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[8] & targ) != 0)
            {
                this->rook_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[1] & targ) != 0)
            {
                this->queen_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[7] & targ) != 0)
            {
                this->queen_legal_moves(targ, 1);
                found = true;
            }
            if((this->bitboard[5] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 0);
                found = true;
            }
            if((this->bitboard[11] & targ) != 0)
            {
                this->pawn_legal_moves(targ, 1);
                found = true;
            }
            depth++;
            if(found) piece = targ;
        }
    }
    
    if(!found)
    {
        while(!oldKingless.empty())
        {
            oldKingless.pop();
        }
    }
    else
    {
        while(!oldKingless.empty())
        {
            uint64_t val = pow(2, oldKingless.top());
            if((piece & black_pieces) != 0)
            {
                this->kingSftyCntWhite[oldKingless.top()]--;
                if(this->kingSftyCntWhite[oldKingless.top()] == 0)
                    this->kingless_zones_white ^= val;
            }
            else
            {
                this->kingSftyCntBlack[oldKingless.top()]--;
                if(this->kingSftyCntBlack[oldKingless.top()] == 0)
                    this->kingless_zones_black ^= val;
            }
            oldKingless.pop();
        }
    }
}

void Chess:: shootKnights(uint64_t coord)
{
    int idx = 0;
    uint64_t h = coord;
    while(h >>= 1) idx++;

    int x = (idx%8)-1;
    int y = floor(idx/8);
    if(idx%8 == 0)
    {
        y--;
        x = 7;
    }

    uint64_t white_pieces = this->white_pieces();
    uint64_t black_pieces = this->black_pieces();

    stack<int> idxs;
    if(x > 1 && y < 7) idxs.push((y+1)*8+x-2);
    if(x > 0 && y < 6) idxs.push((y+2)*8+x-1);
    if(x < 7 && y < 6) idxs.push((y+2)*8+x+1);
    if(x < 6 && y < 7) idxs.push((y+1)*8+x+2);
    if(x < 6 && y > 0) idxs.push((y-1)*8+x+2);
    if(x < 7 && y > 1) idxs.push((y-2)*8+x+1);
    if(x > 0 && y > 1) idxs.push((y-2)*8+x-1);
    if(x > 1 && y > 0) idxs.push((y-1)*8+x-2);

    while(!idxs.empty())
    {
        uint64_t val = pow(2, idxs.top());
        if((this->bitboard[4] & val) != 0)
        {
            this->knight_legal_moves(val, 0);
        }
        if((this->bitboard[10] & val) != 0)
        {
            this->knight_legal_moves(val, 1);
        }
        idxs.pop();
    }
}

void Chess::shootPawns(int diag, uint64_t coord)
{
    int idx = 0;
    uint64_t h = coord;
    while(h >>= 1) idx++;

    int x = (idx%8)-1;
    int y = floor(idx/8);
    if(idx%8 == 0)
    {
        y--;
        x = 7;
    }

    uint64_t white_pieces = this->white_pieces();
    uint64_t black_pieces = this->black_pieces();

    if(diag == 0)
    {
        this->pawn_legal_moves((y+1)*8+x+1, 0);
    }
    else if(diag == 1)
    {
        this->pawn_legal_moves((y+1)*8+x-1, 0);
    }
    else if(diag == 2)
    {
        this->pawn_legal_moves((y-1)*8+x+1, 1);
    }
    else if(diag == 3)
    {
        this->pawn_legal_moves((y-1)*8+x-1, 1);
    }
}

bool Chess::step(uint64_t from, uint64_t to)
{
    //steps 2 make a move
    //1. move availability by comparing from to pieces
    //2. make sure piece is not locked by king
    //3. ensure that the piece has legal moves and that "to" is one of them
    //4. remove the old legal moves and the idx from the array
    //5. change bitboard and make move
    //6. compare all blocks ints with from.
    //7. shoot out to the pieces it was blocking or affecting and re evaluate their legal moves
    //8. modify kingless zones to remove old legal moves from it as long as kingsfty cnt is 1 remove the idx. always dec kingsftycnt
    //9. check360 and recalibrate pieces that are near: rules
    /*10. logic for this.
    For pawns depending on color if you move in the 3 squares in front
    For queens if you line up diag or straight
    For Bishops if you line up diag
    For rooks if you line up straight
    Will run a knight mask and recal knight
    Will run a king mask and recal king*/
    bool valid = true;
    uint64_t pieces_avail = 0;
    if(this->wturn)
    {
        pieces_avail = this->white_pieces();
    }
    else
    {
        pieces_avail = this->black_pieces();
    }
    //REMEMBER TO ADD THS LATER
    uint64_t hasToMoveToOneOfThese = 0;
    bool kingLocked = false;

    if(this->wturn)
    {
        uint i = 0;
        while(i < 8 && hasToMoveToOneOfThese != 0)
        {
            if((this->king_safety_lines_white[i] & from) != 0)
            {
                hasToMoveToOneOfThese = this->king_safety_lines_white[i];
                kingLocked = true;
            }
            i++;
        }
    }
    else
    {
        uint i = 0;
        while(i < 8 && hasToMoveToOneOfThese != 0)
        {
            if((this->king_safety_lines_black[i] & from) != 0)
            {
                hasToMoveToOneOfThese = this->king_safety_lines_black[i];
                kingLocked = true;
            }
            i++;
        }
    }

    if((from & pieces_avail) != 0)
    {
        uint idx = log2(from);
        uint num_1s = 0;
        uint64_t avail = this->pieces();
        avail <<= idx;
        avail >>= idx;
        while(avail > 0)
        {
            avail &= (avail-1);
            num_1s++;
            
        }
        idx = num_1s;
        vector<uint64_t> avail_moves;
        int a_idx = idx;
        if(this->wturn) a_idx = idx - this->legal_moves_black.size()-1;
        bool v = false;
        if(this->wturn)
        {
            avail_moves = this->legal_moves_white;
            if((to & avail_moves[a_idx]) != 0) v = true;
        }
        else
        {
            avail_moves = this->legal_moves_black;
            if((to & avail_moves[a_idx]) != 0) v = true;
        }
        
        if(v)
        {
            uint64_t nMoves = avail_moves[a_idx];
            if(kingLocked)
            {
                if((hasToMoveToOneOfThese & nMoves) == 0)
                {
                    //cant move at all
                    return false;
                }
                else
                {
                    nMoves &= hasToMoveToOneOfThese;
                }
            }
            while(nMoves != 0)
            {
                uint64_t m = 9223372036854775808;
                uint64_t temp = nMoves;
                uint64_t res = 9223372036854775808;
                int cnt = 0;
                while(res >= 9223372036854775808)
                {
                    res = (m ^ temp);
                    temp <<= 1;
                    cnt++;
                }
                if(cnt == 64) nMoves = 0;
                else
                {
                    nMoves <<= cnt;
                    nMoves >>= cnt;
                }

                uint64_t idx = pow(2, 64-cnt);
                if(this->wturn)
                {
                    if((idx & this->kingless_zones_black) != 0)
                    {
                        this->kingSftyCntBlack[cnt]--;
                        if(this->kingSftyCntBlack[cnt] == 0)
                        {
                            this->kingless_zones_black ^= idx; 
                        }
                    }
                }
                else
                {
                    if((idx & this->kingless_zones_white) != 0)
                    {
                        this->kingSftyCntWhite[cnt]--;
                        if(this->kingSftyCntWhite[cnt] == 0)
                        {
                            this->kingless_zones_white ^= idx;
                        }
                    }
                }
            }
            if(this->wturn)
            {
                this->legal_moves_white.erase(this->legal_moves_white.begin() + a_idx);
                for(uint i = 0 ; i < 6; i++)
                {
                    if((from & this->bitboard[i]) != 0)
                    {
                        this->bitboard[i] ^= from;
                        this->bitboard[i] |= to;
                    }
                    if(i == 0 && from == 8)
                    {
                        if(to != 2 && to != 32)
                        {
                            this->wCanCastle = false;
                            this->wCanQCastle = false;
                        }
                        else
                        {
                            //CASTLING SPECIAL MOVE NEEDS TO BE PROGRAMMED
                        }
                    }
                    if(i == 2)
                    {
                        if(this->wCanCastle && from == 1) this->wCanCastle = false;
                        if(this->wCanQCastle && from == 128) this->wCanQCastle = false;
                    }
                }
            }
            else
            {
                this->legal_moves_black.erase(this->legal_moves_black.begin() + a_idx);
                for(uint i = 6; i < 12; i++)
                {
                    if((from & this->bitboard[i]) != 0)
                    {
                        this->bitboard[i] ^= from;
                    }

                    this->bitboard[i] &= to;
                    if(i == 6 && from == 576460752303423488)
                    {
                        if(to != 144115188075855872 && to != 2305843009213693952)
                        {
                            this->bCanCastle = false;
                            this->bCanQCastle = false;
                        }
                        else
                        {
                            //CASTLING SPECIAL MOVE NEEDS TO BE PROGRAMMED
                        }
                    }
                    if(i == 8)
                    {
                        if(this->bCanQCastle && from == 9223372036854775808) this->bCanQCastle = false;
                        if(this->bCanCastle && from == 72057594037927936) this->bCanCastle = false;
                    }
                }
            }
            for(uint i = 0; i < 4; i++)
            {
                if((from & this->blocks_diagonal[i]) != 0)
                {
                    shootDiags(i, from);
                    this->blocks_diagonal[i] ^= from;
                }
                if((from & this->blocks_straight[i]) != 0)
                {
                    shootStraights(i, from);
                    this->blocks_straight[i] ^= from;
                }
            }
            if((from & this->blocks_knight) != 0)
            {
                shootKnights(from);
                this->blocks_knight ^= from;
            }
            int x = (from % 8)-1;
            int y = floor(from/8);
            if((from & this->affects_pawn_diagonal[0]) != 0)
                pawn_legal_moves(pow(2, (y+1)*8+x+1), 1);
            if((from & this->affects_pawn_diagonal[1]) != 0)
                pawn_legal_moves(pow(2, (y+1)*8+x-1), 1);
            if((from & this->affects_pawn_diagonal[2]) != 0)
                pawn_legal_moves(pow(2, (y-1)*8+x+1), 0);
            if((from & this->affects_pawn_diagonal[3]) != 0)
                pawn_legal_moves(pow(2, (y-1)*8+x-1), 0);
            //WILL NEED TO REMOVE THESE IDX FROM THE LISTS IF THEY APPEAR
            //WE WILL RECOMPUTE KINGS EACH MOVE
            king_legal_moves(this->bitboard[0], 0);
            king_legal_moves(this->bitboard[6], 1);
            //check360 causing prolem
            check360(to);
        }
    }
    else return false;
    return true;
}

string Chess::cypherMove(uint64_t move)
{
    return "";
}

void Chess::startConsole()
{
    cout << this->board_representation() << endl;
    while(true)
    {
        cout << "WHITE MOVE:";
        string move;
        cin >> move;
        cout << endl;
    }
}

void Chess::init_move_map()
{
    Num2let n;
    uint64_t id = 9223372036854775808;
    for(uint i = 1;i < 65; i++)
    {
        int x = (i % 8) - 1;
        int y = floor(i/8);
        if(i%8 == 0)
        {
            y--;
            x = 7;
        }
        
        string res = n.get(x) + to_string(y);
        moves[res] = pow(2, y*8+x);
        id >>= 1;
    }
}

Num2let::Num2let()
{
    let.push_back("a");
    let.push_back("b");
    let.push_back("c");
    let.push_back("d");
    let.push_back("e");
    let.push_back("f");
    let.push_back("g");
    let.push_back("h");
}

string Num2let::get(int x)
{
    return let[x];
}