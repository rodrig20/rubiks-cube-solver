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

    // Obter cores do sticker da face
    int* U_sticker(int x, int y);
    int* L_sticker(int x, int y);
    int* F_sticker(int x, int y);
    int* R_sticker(int x, int y);
    int* B_sticker(int x, int y);
    int* D_sticker(int x, int y);

    // Obter face
    Face get_U();
    Face get_L();
    Face get_F();
    Face get_R();
    Face get_B();
    Face get_D();

    // Rotacionar camada completa do cubo
    void rot_slice(int slice[5][5], int clockwise);
    static void rotate90AntiClockwise(int matrix[5][5]);
    static void rotate90Clockwise(int matrix[5][5]);

    // Obter/Definir camada de completa
    void get_slice_x(int slice[5][5], int idx);
    void set_slice_x(int slice[5][5], int idx);
    void get_slice_y(int slice[5][5], int idx);
    void set_slice_y(int slice[5][5], int idx);
    void get_slice_z(int slice[5][5], int idx);
    void set_slice_z(int slice[5][5], int idx);

    // Rotacionar uma camada especifica
    void rot_U(int clockwise);
    void rot_D(int clockwise);
    void rot_F(int clockwise);
    void rot_B(int clockwise);
    void rot_L(int clockwise);
    void rot_R(int clockwise);

    // Obter coordenadas das edges do cubo
    int get_edge_ori_coord();

    // Verifica se uma face está resolvida
    int is_solved_face(Face face);

    // Verificar etapas da resolução
    int check_state_EO();
    int check_state_cross();
    int check_state_F2L();
    int check_state_OLL();

    // Obter algoritmos do json
    void get_algs(const char* jsonString, JsonDocument& doc);

    // Girar a ultima camada para a acertar
    string finish_last_layer();

    // Obter o tipo de ZBLL
    tuple<int, string> ZBLL_find(int last_layer_state[16]);

   public:
    // Construtor
    Solver(string cube_string);

    // Retorna uma string com estado do cubo resolvido
    static string solved_string();
    // Mostra o estado do cubo no terminal
    void show();

    // Aplica uma sequencia de movimentos normal / reverço / espelhado
    void move(const string move_string);
    string revert_move(const string move_string);
    string mirror_move(const string move_string, int right_left_mirror,
                       int front_back_mirror);

    // Etapas de resolução do cubo
    string EO();
    string cross();
    string F2L();
    string OLL();
    string PLL();
    string ZBLL();

    // Resolução completa
    string solve();
    // Verificar se está solucionado
    int is_solved();

    // Simplificar sequencia de movimentos
    static string simplify_move(const string move_string);

    // Aplica embaralhamento
    string scramble(int size);

    // Obtém represetações do cubo
    array<array<int, 3>, 26> piece_state();
    string sticker_state();

    // Transforma o cubo em outro
    string match_state(Solver& original_cube);
};

#endif