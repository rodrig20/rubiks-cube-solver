#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <ArduinoJson.h>

#include <array>
#include <string>
#include <tuple>

using namespace std;
using Face = array<array<int, 3>, 3>;

class Corner {
   private:
    int* stickers[3];

   public:
    Corner() {}

    Corner(int* sticker0, int* sticker1, int* sticker2);

    int orientation();
    tuple<int, int, int> colors();
    int pair(int color1, int color2);
};

class Edge {
   private:
    int* stickers[2];

   public:
    Edge() {};

    Edge(int* sticker0, int* sticker1);

    int orientation();
    tuple<int, int> colors();
    int is_UDSlice();
};

class Solver {
   private:
    int state[5][5][5] = {{{}}};
    Corner corners[8];
    Edge edges[12];

    void set_face(int face_orientation, int face_side, int* face_array,
                  int inv_x, int inv_y);
    void string_to_face_array(string face_string, int face_array[9]);
    int* U_sticker(int x, int y);
    int* L_sticker(int x, int y);
    int* F_sticker(int x, int y);
    int* R_sticker(int x, int y);
    int* B_sticker(int x, int y);
    int* D_sticker(int x, int y);

    Face get_U();
    Face get_L();
    Face get_F();
    Face get_R();
    Face get_B();
    Face get_D();

    static void rotate90AntiClockwise(int matrix[5][5]);
    static void rotate90Clockwise(int matrix[5][5]);

    void get_slice_x(int slice[5][5], int idx);
    void set_slice_x(int slice[5][5], int idx);
    void get_slice_y(int slice[5][5], int idx);
    void set_slice_y(int slice[5][5], int idx);
    void get_slice_z(int slice[5][5], int idx);
    void set_slice_z(int slice[5][5], int idx);

    void rot_slice(int slice[5][5], int clockwise);
    void rot_U(int clockwise);
    void rot_D(int clockwise);
    void rot_F(int clockwise);
    void rot_B(int clockwise);
    void rot_L(int clockwise);
    void rot_R(int clockwise);

    int get_edge_ori_coord();

    int is_solved_face(Face face);

    int check_state_EO();
    int check_state_cross();
    int check_state_F2L();
    int check_state_OLL();

    void get_algs(const char* jsonString, StaticJsonDocument<1024>& doc);

    string finish_last_layer();

   public:
    Solver(string cube_string);

    static string solved_string();
    void show();

    void move(const string move_string);
    string revert_move(const string move_string);

    string mirror_move(const string move_string, int right_left_mirror,
                       int front_back_mirror);

    string EO();
    string EO_force();
    string cross();
    string F2L();
    string OLL();
    string PLL();

    string solve();
    int is_solved();
    static string simplify_move(const string move_string);

    string scramble(int size);

    array<array<int, 3>, 26> piece_state();
    string sticker_state();

    string match_state(Solver& original_cube);
};

#endif