#include "Solver.hpp"

#include <LittleFS.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <vector>

using namespace std;

using Face = array<array<int, 3>, 3>;

const char* EO_Path = "/info/EO.txt";

const char* F2L_Json =
    R"rawliteral({"corner_top":{"0":{"0":"D2 R2 D2 R D R' D R2","1":"R' D2 R D R' D' R","2":"D' R' D2 R D' R' D R","3":"R' D R D2 R' D' R"},"1":{"0":"R' D' R D2 R' D R D' R' D R","1":"D' R' D R","2":"D R' D' R D2 R' D R","3":"D R' D2 R D2 R' D R"},"2":{"0":"R D2 R2 D' R2 D' R'","1":"D R' D R D' R' D' R","2":"R' D' R","3":"D R' D' R D' R' D' R"}},"corner_bottom":{"0":"R F R' D' R' D R F'","1":"R' D' R D R' D' R","2":"R' D R D' R' D R"}})rawliteral";

const char* OLL_Json =
    R"rawliteral({"1002":"R F L' F' R' F L F'","1011":"R' D' R D' R' D2 R","1122":"R' D2 R2 D R2 D R2 D2 R'","1200":"R2 U' R D2 R' U R D2 R","2121":"R' D' R D' R' D R  D' R' D2 R","0222":"L D L' D L D2 L'","0201":"F' R F L' F' R' F L"})rawliteral";

const char* PLL_Json =
    R"rawliteral({"113241434322":"L' F  L' B2 L F' L' B2 L2","114343231422":"L2 B2 L F L' B2 L F' L","134341413222":"L' D' F' L D L' D' L' F L2 D' L' D' L D L' D L","144321413232":"L2 D L' D L' D' L D' L2 D' U L' D L U'","134311423242":"L' D' L D U' L2 D L' D L D' L D' L2 U","134321443212":"L2 D' L D' L D L' D L2 D U' L D' L' U","124331413242":"L D L' D' U L2 D' L D' L' D L' D L2 U'","114331443222":"L2 U L U' L F2 R' D R F2","144333211422":"R2 U' R' U R' F2 L D' L' F2","114341423232":"L D' L' D' L D L U L' D' L U' L' D2 L'","144323231412":"R' D R D R' D' R' U' R D R' U R D2 R","114321433242":"L D L' D' L' F L2 D' L' D' L D L' F'","143234321412":"L B' L' F L B L' F2 R' B R F R' B' R","133244311422":"R D R' D R2 U' R' U R' F2 L D' L' F2 D' R D' R'","113224331442":"L' D' L D' L2 U L U' L F2 R' D R F2 D L' D L","113234341422":"L' D L' D' B' L' B2 D' B' D B' L B L","133214321442":"F' R' D R D R' D' R F R' D' R D R F' R' F","131424313242":"R2 L2 U R2 L2 D2 R2 L2 U R2 L2","141424333212":"F2 D' R L' F2 R' L D' F2","121414333242":"F2 D R L' F2 R' L D F2","141414323232":"R2 L2 U R2 L2 D R L' F2 R2 L2 B2 R L'","111444333222":""})rawliteral";

void roll_array(int array[], int size, int n) {
    int* temp = new int[size];  // Aloca o array temporário dinamicamente

    // Faz o "roll" do array
    for (int i = 0; i < size; i++) {
        temp[(i + n) % size] = array[i];
    }

    // Copia de volta para o array original
    for (int i = 0; i < size; i++) {
        array[i] = temp[i];
    }

    delete[] temp;
    temp = nullptr;
}

void roll_array(int array[], int size) { roll_array(array, size, 1); }

string array_to_string(int arr[], int size) {
    stringstream ss;

    // Itera pelo array e adiciona os elementos ao stringstream
    for (int i = 0; i < size; ++i) {
        ss << arr[i];
    }

    // Converte o conteúdo do stringstream para uma string
    return ss.str();
}

Corner::Corner(int* sticker0, int* sticker1, int* sticker2) {
    stickers[0] = sticker0;
    stickers[1] = sticker1;
    stickers[2] = sticker2;
}

// Calcular a orientação (entre 0, 1 e 2)
int Corner::orientation() {
    // Percorrer cores do canto
    for (size_t i = 0; i < 3; i++) {
        int sticker = *stickers[i];
        // Cores da top ou bottom layer
        if (sticker == 0 || sticker == 5) {
            return i;
        }
    }
    return -1;
}

// Tupla com as cores dos stickers
tuple<int, int, int> Corner::colors() {
    return make_tuple(*stickers[0], *stickers[1], *stickers[2]);
}

// Verifica se o canto atual faz um par F2L com as cores de uma edge (0
// mais 2 cores)
int Corner::pair(int color1, int color2) {
    for (int i = 0; i < 3; i++) {
        int sticker = *stickers[i];

        // Se for uma das cores da edge ou cor da top layer
        if (!(sticker == color1 || sticker == color2 || sticker == 0)) {
            return 0;
        }
    }
    return 1;
}

Edge::Edge(int* sticker0, int* sticker1) {
    stickers[0] = sticker0;
    stickers[1] = sticker1;
}

// Calcular a orientação (entre 0 e 1)
int Edge::orientation() {
    for (size_t i = 0; i < 2; i++) {
        int sticker = *stickers[i];
        // Cores da top ou bottom layer
        if (sticker == 0 || sticker == 5) {
            return i;
        }
    }
    // Se nenhum sticker for da top ou bottom layer
    for (int i = 0; i < 2; i++) {
        int sticker = *stickers[i];

        // Cores da front ou back layer
        if (sticker == 1 || sticker == 3) {
            return i;
        }
    }
    return -1;
}

// Tupla com as cores dos stickers
tuple<int, int> Edge::colors() {
    return make_tuple(*stickers[0], *stickers[1]);
}

// Verifica se a Edge faz parte da camada do meio
int Edge::is_UDSlice() {
    for (int i = 0; i < 2; i++) {
        int sticker = *stickers[i];
        // Verifica se é alguma das cores da top ou bottom layer
        if (sticker == 0 || sticker == 5) {
            return 0;
        }
    }
    return 1;
}

// Método para atualizar o state do de uma face do cubo
void Solver::set_face(int face_orientation, int face_side, int* face_array,
                      int inv_x, int inv_y) {
    int count = 0;
    // Se for do lado "negativo", troca pela ultima camada
    if (face_side == -1) {
        face_side = 4;
    }

    // Percorrer a face e atualizar valores (a ordem depende de inv_x e
    // inv_y)
    for (int i = (inv_x ? 3 : 1); (inv_x ? i >= 1 : i <= 3);
         (inv_x ? i-- : i++)) {
        for (int j = (inv_y ? 3 : 1); (inv_y ? j >= 1 : j <= 3);
             (inv_y ? j-- : j++)) {
            // Atualizar o valor da face_side
            if (face_orientation == 0) {
                state[face_side][i][j] = face_array[count];
            } else if (face_orientation == 1) {
                state[i][face_side][j] = face_array[count];
            } else if (face_orientation == 2) {
                state[i][j][face_side] = face_array[count];
            }
            count++;
        }
    }
}

// Converte uma string que representa uma face em um array
void Solver::string_to_face_array(string face_string, int face_array[9]) {
    // Percorrer todos os caracteres
    for (int i = 0; i < 9; i++) {
        // Converte o char para int
        face_array[i] = face_string[i] - static_cast<int>('0');
    }
}

// Obter a face Up
Face Solver::get_U() {
    Face face;

    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[0][4 - i][j];
        }
    }

    return face;
}

// Obter a face Left
Face Solver::get_L() {
    Face face;

    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[i][4 - j][0];
        }
    }

    return face;
}

// Obter a face Front
Face Solver::get_F() {
    Face face;

    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[i][0][j];
        }
    }

    return face;
}

// Obter a face Right
Face Solver::get_R() {
    Face face;
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[i][j][4];
        }
    }
    return face;
}

// Obter a face Back
Face Solver::get_B() {
    Face face;
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[i][4][4 - j];
        }
    }
    return face;
}

// Obter a face Down
Face Solver::get_D() {
    Face face;
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            face[i - 1][j - 1] = state[4][i][j];
        }
    }
    return face;
}

// Obtem a cor de um sticker em determinada posção na camada Up
int* Solver::U_sticker(int x, int y) { return &(state[0][4 - (x + 1)][y + 1]); }
// Obtem a cor de um sticker em determinada posção na camada Left
int* Solver::L_sticker(int x, int y) { return &(state[x + 1][4 - (y + 1)][0]); }
// Obtem a cor de um sticker em determinada posção na camada Front
int* Solver::F_sticker(int x, int y) { return &(state[x + 1][0][y + 1]); }
// Obtem a cor de um sticker em determinada posção na camada Right
int* Solver::R_sticker(int x, int y) { return &(state[x + 1][y + 1][4]); }
// Obtem a cor de um sticker em determinada posção na camada Back
int* Solver::B_sticker(int x, int y) { return &(state[x + 1][4][4 - (y + 1)]); }
// Obtem a cor de um sticker em determinada posção na camada Right
int* Solver::D_sticker(int x, int y) { return &(state[4][x + 1][y + 1]); }

// Rotacionar camada Up
void Solver::rot_U(int clockwise) {
    int slice[5][5];
    get_slice_x(slice, 0);
    rot_slice(slice, clockwise);
    set_slice_x(slice, 0);

    get_slice_x(slice, 1);
    rot_slice(slice, clockwise);
    set_slice_x(slice, 1);
}

// Rotacionar camada Down
void Solver::rot_D(int clockwise) {
    int slice[5][5];
    get_slice_x(slice, 4);
    // clockwise é invertido por estar por estar na "face não primária"
    rot_slice(slice, -clockwise);
    set_slice_x(slice, 4);
    // clockwise é invertido por estar por estar na "face não primária"
    get_slice_x(slice, 3);
    rot_slice(slice, -clockwise);
    set_slice_x(slice, 3);
}

// Rotacionar camada Front
void Solver::rot_F(int clockwise) {
    int slice[5][5];
    get_slice_y(slice, 0);
    // clockwise é invertido por estar por estar na "face não primária"
    rot_slice(slice, -clockwise);
    set_slice_y(slice, 0);

    get_slice_y(slice, 1);
    // clockwise é invertido por estar por estar na "face não primária"
    rot_slice(slice, -clockwise);
    set_slice_y(slice, 1);
}

// Rotacionar camada Back
void Solver::rot_B(int clockwise) {
    int slice[5][5];
    get_slice_y(slice, 4);
    rot_slice(slice, clockwise);
    set_slice_y(slice, 4);

    get_slice_y(slice, 3);
    rot_slice(slice, clockwise);
    set_slice_y(slice, 3);
}

// Rotacionar camada Left
void Solver::rot_L(int clockwise) {
    int slice[5][5];
    get_slice_z(slice, 0);
    rot_slice(slice, clockwise);
    set_slice_z(slice, 0);

    get_slice_z(slice, 1);
    rot_slice(slice, clockwise);
    set_slice_z(slice, 1);
}

// Rotacionar camada Right
void Solver::rot_R(int clockwise) {
    int slice[5][5];
    get_slice_z(slice, 4);
    // clockwise é invertido por estar por estar na "face não primária"
    rot_slice(slice, -clockwise);
    set_slice_z(slice, 4);

    get_slice_z(slice, 3);
    // clockwise é invertido por estar por estar na "face não primária"
    rot_slice(slice, -clockwise);
    set_slice_z(slice, 3);
}

// Rodar layer 5x5 no sentido anti horário
void Solver::rotate90AntiClockwise(int matrix[5][5]) {
    // Transpor a matriz
    for (int i = 0; i < 5; ++i) {
        for (int j = i; j < 5; ++j) {
            swap(matrix[i][j], matrix[j][i]);
        }
    }

    // Inverter colunas
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5 / 2; ++j) {
            swap(matrix[i][j], matrix[i][5 - j - 1]);
        }
    }
}
// Rodar layer 5x5 no sentido horário
void Solver::rotate90Clockwise(int matrix[5][5]) {
    // Transpor a matriz
    for (int i = 0; i < 5; ++i) {
        for (int j = i; j < 5; ++j) {
            swap(matrix[i][j], matrix[j][i]);
        }
    }

    // Inverter as linhas
    for (int j = 0; j < 5; ++j) {
        for (int i = 0; i < 5 / 2; ++i) {
            swap(matrix[i][j], matrix[5 - i - 1][j]);
        }
    }
}

// Obter a camada idx no sentido x
void Solver::get_slice_x(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            slice[i][j] = state[idx][i][j];
        }
    }
}
// Definir valores da camada idx no sentido x
void Solver::set_slice_x(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            state[idx][i][j] = slice[i][j];
        }
    }
}
// Obter a camada idx no sentido y
void Solver::get_slice_y(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            slice[i][j] = state[i][idx][j];
        }
    }
}

// Definir valores da camada idx no sentido y
void Solver::set_slice_y(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            state[i][idx][j] = slice[i][j];
        }
    }
}

// Obter a camada idx no sentido z
void Solver::get_slice_z(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            slice[i][j] = state[i][j][idx];
        }
    }
}

// Definir valores da camada idx no sentido z
void Solver::set_slice_z(int slice[5][5], int idx) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            state[i][j][idx] = slice[i][j];
        }
    }
}

// Rotacionar Camada no sentido hórario ou anti horário
void Solver::rot_slice(int slice[5][5], int clockwise) {
    while (clockwise > 0) {
        rotate90Clockwise(slice);
        clockwise--;
    }
    while (clockwise < 0) {
        rotate90AntiClockwise(slice);
        clockwise++;
    }
}

// Obter inteiro que representa a orientação de todos as edges
int Solver::get_edge_ori_coord() {
    int result = 0;
    // Transforma o array (que simula um inteiro de base 3) em um inteiro
    // (base10)
    for (size_t i = 0; i < 11; i++) {
        result += edges[i].orientation() * (pow(2, 10 - i));
    }
    return result;
}

// Verifica se uma face está resolvido
int Solver::is_solved_face(Face face) {
    int color = face[0][0];
    // Percorrer todos as posições e comparar a sua cor
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (face[i][j] != color) {
                return 0;
            }
        }
    }

    return 1;
}

// Verifica se a etapa EO está concluida
int Solver::check_state_EO() { return get_edge_ori_coord() == 0; }

// Verifica se a etapa de cruz está concluida
int Solver::check_state_cross() {
    Face faces_list[4] = {get_F(), get_L(), get_B(), get_R()};
    for (int e = 0; e < 4; e++) {
        int primary_color, secundary_color;
        tie(primary_color, secundary_color) = edges[e].colors();
        // Verifica se a cor é diferente zero e se a cor é diferente da do
        // seu centro
        if (primary_color != 0 || secundary_color != faces_list[e][1][1]) {
            return 0;
        }
    }
    return check_state_EO();
}

// Verifica se a etapa de F2L está concluida
int Solver::check_state_F2L() {
    Face faces_list[4] = {get_F(), get_L(), get_B(), get_R()};
    for (int c = 0; c < 4; c++) {
        int color = faces_list[c][1][1];
        // Verificar par F2L
        for (int i = 0; i < 3; i += 2) {
            for (int j = 0; j < 2; j++) {
                if (faces_list[c][j][i] != color) {
                    return 0;
                }
            }
        }
    }
    return is_solved_face(get_U()) && check_state_cross();
}

// Verifica se a etapa de OLL está concluida
int Solver::check_state_OLL() {
    return check_state_F2L() && is_solved_face(get_D());
}

// Obtem um json com o estado e o seu respetivo algoritmo para resolver
void Solver::get_algs(const char* jsonString, StaticJsonDocument<1024>& doc) {
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        cout << "Erro ao analisar JSON: ";
        cout << error.c_str() << endl;
    }
}

// Resolve a ultima camada
string Solver::finish_last_layer() {
    string move_sequence = "";
    // Girar a ultima camada até estar resolvido
    for (int i = 0; i < 4; i++) {
        if (is_solved() == 1) {
            break;
        }
        move_sequence += "D' ";
        move("D'");
    }
    return move_sequence;
}

Solver::Solver(string cube_string) {
    for (int i = 0; i < 6; i++) {
        int face_array[9];
        // Converter a string em um array com as cores dos stickers por
        // ordem
        string_to_face_array(cube_string.substr(i * 9, 9), face_array);
        switch (i) {
            // Atualizar a face de acordo com o array
            case 0:
                set_face(0, 0, face_array, 1, 0);
                break;
            case 1:
                set_face(1, 0, face_array, 0, 0);
                break;
            case 2:
                set_face(2, -1, face_array, 0, 0);
                break;
            case 3:
                set_face(1, -1, face_array, 0, 1);
                break;
            case 4:
                set_face(2, 0, face_array, 0, 1);
                break;
            case 5:
                set_face(0, -1, face_array, 0, 0);
                break;
        }
    }

    int down_right[2] = {2, 2};
    int down_left[2] = {2, 0};
    int up_left[2] = {0, 0};
    int up_right[2] = {0, 2};
    // Camada de cima
    corners[0] = Corner(U_sticker(down_right[0], down_right[1]),
                        R_sticker(up_left[0], up_left[1]),
                        F_sticker(up_right[0], up_right[1]));
    corners[1] = Corner(U_sticker(down_left[0], down_left[1]),
                        F_sticker(up_left[0], up_left[1]),
                        L_sticker(up_right[0], up_right[1]));
    corners[2] = Corner(U_sticker(up_left[0], up_left[1]),
                        L_sticker(up_left[0], up_left[1]),
                        B_sticker(up_right[0], up_right[1]));
    corners[3] = Corner(U_sticker(up_right[0], up_right[1]),
                        B_sticker(up_left[0], up_left[1]),
                        R_sticker(up_right[0], up_right[1]));
    // Camada de baixo
    corners[4] = Corner(D_sticker(up_right[0], up_right[1]),
                        F_sticker(down_right[0], down_right[1]),
                        R_sticker(down_left[0], down_left[1]));
    corners[5] = Corner(D_sticker(up_left[0], up_left[1]),
                        L_sticker(down_right[0], down_right[1]),
                        F_sticker(down_left[0], down_left[1]));
    corners[6] = Corner(D_sticker(down_left[0], down_left[1]),
                        B_sticker(down_right[0], down_right[1]),
                        L_sticker(down_left[0], down_left[1]));
    corners[7] = Corner(D_sticker(down_right[0], down_right[1]),
                        R_sticker(down_right[0], down_right[1]),
                        B_sticker(down_left[0], down_left[1]));

    int left[2] = {1, 0};
    int down[2] = {2, 1};
    int up[2] = {0, 1};
    int right[2] = {1, 2};

    // Camada de cima
    edges[0] = Edge(U_sticker(down[0], down[1]), F_sticker(up[0], up[1]));
    edges[1] = Edge(U_sticker(left[0], left[1]), L_sticker(up[0], up[1]));
    edges[2] = Edge(U_sticker(up[0], up[1]), B_sticker(up[0], up[1]));
    edges[3] = Edge(U_sticker(right[0], right[1]), R_sticker(up[0], up[1]));
    // Camada de baixo
    edges[4] = Edge(D_sticker(up[0], up[1]), F_sticker(down[0], down[1]));
    edges[5] = Edge(D_sticker(left[0], left[1]), L_sticker(down[0], down[1]));
    edges[6] = Edge(D_sticker(down[0], down[1]), B_sticker(down[0], down[1]));
    edges[7] = Edge(D_sticker(right[0], right[1]), R_sticker(down[0], down[1]));
    // Camada do meio
    edges[8] = Edge(F_sticker(right[0], right[1]), R_sticker(left[0], left[1]));
    edges[9] = Edge(F_sticker(left[0], left[1]), L_sticker(right[0], right[1]));
    edges[10] =
        Edge(B_sticker(right[0], right[1]), L_sticker(left[0], left[1]));
    edges[11] =
        Edge(B_sticker(left[0], left[1]), R_sticker(right[0], right[1]));
}

// Obtém a string de um cubo resolvido
string Solver::solved_string() {
    return "000000000111111111222222222333333333444444444555555555";
}

// Mostra o estado do cubo
void Solver::show() {
    std::map<int, string> cores = {{0, " W"}, {1, " G"}, {2, " R"},
                                   {3, " B"}, {4, " O"}, {5, " Y"}};

    auto show_face = [&cores](Face face) {
        for (int i = 0; i < 3; ++i) {
            cout << "       ";  // Espaço de tamanho 7
            for (int j = 0; j < 3; ++j) {
                cout << cores[face[i][j]];  // Acessa a cor correspondente
            }
            cout << endl;
        }
        cout << endl;
    };

    auto show_face_line = [&cores](Face face, int line_num) {
        for (int j = 0; j < 3; ++j) {
            cout << cores[face[line_num][j]];
        }
        cout << " ";
    };

    Face U_face = get_U();
    Face F_face = get_F();
    Face R_face = get_R();
    Face B_face = get_B();
    Face L_face = get_L();
    Face D_face = get_D();

    // Print da camada U
    show_face(U_face);

    // Print das camadas L, F, R, B
    for (int i = 0; i < 3; ++i) {
        show_face_line(L_face, i);

        show_face_line(F_face, i);

        show_face_line(R_face, i);

        show_face_line(B_face, i);
        cout << endl;
    }
    cout << endl;
    // Print da camada D
    show_face(D_face);
}

// Aplica uma string de movimentos a um cubo
void Solver::move(const string move_string) {
    stringstream ss(move_string);
    string move;
    // Traduzir movimento para função
    while (ss >> move) {
        if (move == "U2")
            rot_U(2);
        else if (move == "U'")
            rot_U(-1);
        else if (move == "U")
            rot_U(1);
        else if (move == "F2")
            rot_F(2);
        else if (move == "F'")
            rot_F(-1);
        else if (move == "F")
            rot_F(1);
        else if (move == "R2")
            rot_R(2);
        else if (move == "R'")
            rot_R(-1);
        else if (move == "R")
            rot_R(1);
        else if (move == "B2")
            rot_B(2);
        else if (move == "B'")
            rot_B(-1);
        else if (move == "B")
            rot_B(1);
        else if (move == "L2")
            rot_L(2);
        else if (move == "L'")
            rot_L(-1);
        else if (move == "L")
            rot_L(1);
        else if (move == "D2")
            rot_D(2);
        else if (move == "D'")
            rot_D(-1);
        else if (move == "D")
            rot_D(1);
    }
}

// Reverter uma sequencia de movimentos
string Solver::revert_move(const string move_string) {
    stringstream ss(move_string);
    vector<string> moves;
    string move;
    vector<string> final_moves;

    // Percorre a string e adiciona cada movimento a um vetor
    while (ss >> move) {
        moves.push_back(move);
    }

    // Reverte o vetor que contém todos os movimentos
    reverse(moves.begin(), moves.end());

    // Traduzir movimento para função invertida (X -> X' e X' -> X)
    for (size_t i = 0; i < moves.size(); i++) {
        string move = moves[i];
        if (move == "U2") {
            rot_U(2);
            final_moves.push_back("U2");
        } else if (move == "U'") {
            rot_U(1);
            final_moves.push_back("U");
        } else if (move == "U") {
            rot_U(-1);
            final_moves.push_back("U'");
        } else if (move == "F2") {
            rot_F(2);
            final_moves.push_back("F2");
        } else if (move == "F'") {
            rot_F(1);
            final_moves.push_back("F");
        } else if (move == "F") {
            rot_F(-1);
            final_moves.push_back("F'");
        } else if (move == "R2") {
            rot_R(2);
            final_moves.push_back("R2");
        } else if (move == "R'") {
            rot_R(1);
            final_moves.push_back("R");
        } else if (move == "R") {
            rot_R(-1);
            final_moves.push_back("R'");
        } else if (move == "B2") {
            rot_B(2);
            final_moves.push_back("B2");
        } else if (move == "B'") {
            rot_B(1);
            final_moves.push_back("B");
        } else if (move == "B") {
            rot_B(-1);
            final_moves.push_back("B'");
        } else if (move == "L2") {
            rot_L(2);
            final_moves.push_back("L2");
        } else if (move == "L'") {
            rot_L(1);
            final_moves.push_back("L");
        } else if (move == "L") {
            rot_L(-1);
            final_moves.push_back("L");
        } else if (move == "D2") {
            rot_D(2);
            final_moves.push_back("D2");
        } else if (move == "D'") {
            rot_D(1);
            final_moves.push_back("D");
        } else if (move == "D") {
            rot_D(-1);
            final_moves.push_back("D'");
        }
    }

    ostringstream oss;
    for (size_t i = 0; i < final_moves.size(); i++) {
        if (i > 0) oss << " ";
        oss << final_moves[i];
    }
    return oss.str();
}

// Obtem uma string espelhada (esquerda-direita e/ou frente-tras)
string Solver::mirror_move(const string move_string, int right_left_mirror,
                           int front_back_mirror) {
    // Se não houver nenhum espelhamento retorna o original
    if (!right_left_mirror && !front_back_mirror) {
        return move_string;
    }
    stringstream ss(move_string);
    string new_move_string = "";
    string move;

    while (ss >> move) {
        // Se houver espelhamento duplo
        if (right_left_mirror && front_back_mirror) {
            if (move == "F'")
                new_move_string += " B'";
            else if (move == "F")
                new_move_string += " B";
            else if (move == "R'")
                new_move_string += " L'";
            else if (move == "R")
                new_move_string += " L";
            else if (move == "B'")
                new_move_string += " F'";
            else if (move == "B")
                new_move_string += " F";
            else if (move == "L'")
                new_move_string += " R'";
            else if (move == "L")
                new_move_string += " R";
            else if (move == "L2")
                new_move_string += " R2";
            else if (move == "R2")
                new_move_string += " L2";
            else if (move == "F2")
                new_move_string += " B2";
            else if (move == "B2")
                new_move_string += " F2";
            else
                new_move_string += " " + move;
            // Espelhamento esquerda-direita
        } else if (right_left_mirror) {
            if (move == "U'")
                new_move_string += " U";
            else if (move == "U")
                new_move_string += " U'";
            else if (move == "F'")
                new_move_string += " F";
            else if (move == "F")
                new_move_string += " F'";
            else if (move == "R'")
                new_move_string += " L";
            else if (move == "R")
                new_move_string += " L'";
            else if (move == "B'")
                new_move_string += " B";
            else if (move == "B")
                new_move_string += " B'";
            else if (move == "L'")
                new_move_string += " R";
            else if (move == "L")
                new_move_string += " R'";
            else if (move == "D'")
                new_move_string += " D";
            else if (move == "D")
                new_move_string += " D'";
            else if (move == "R2")
                new_move_string += " L2";
            else if (move == "L2")
                new_move_string += " R2";
            else
                new_move_string += " " + move;
            // Espelhamento frente-tras
        } else if (front_back_mirror) {
            if (move == "U'")
                new_move_string += " U";
            else if (move == "U")
                new_move_string += " U'";
            else if (move == "F'")
                new_move_string += " B";
            else if (move == "F")
                new_move_string += " B'";
            else if (move == "R'")
                new_move_string += " R";
            else if (move == "R")
                new_move_string += " R'";
            else if (move == "B'")
                new_move_string += " F";
            else if (move == "B")
                new_move_string += " F'";
            else if (move == "L'")
                new_move_string += " L";
            else if (move == "L")
                new_move_string += " L'";
            else if (move == "D'")
                new_move_string += " D";
            else if (move == "D")
                new_move_string += " D'";
            else if (move == "B2")
                new_move_string += " F2";
            else if (move == "F2")
                new_move_string += " B2";
            else
                new_move_string += " " + move;
        }
    }

    return new_move_string;
}

// Resolve a etapa de orientação de edges (Edge Orientation) com brute-force
string Solver::EO_force() {
    if (check_state_EO() == 1) {
        return "";
    }
    string move_set[] = {"U",  "F",  "R",  "B",  "L",  "D",
                         "U'", "F'", "R'", "B'", "L'", "D'"};

    for (int move_count = 1; move_count < 8; move_count++) {
        // Número que vai represnetar a combinação de moviementos
        for (int move_c = 0; move_c < pow(12, move_count); move_c++) {
            yield();
            string move_sequence = "";
            int buffer = move_c;
            for (int i = 0; i < move_count; i++) {
                move_sequence += move_set[buffer % 12] + " ";
                buffer /= 12;
            }

            move(move_sequence);

            int edge_ori = get_edge_ori_coord();
            if (edge_ori == 0) {
                return move_sequence;
            }
            // Desfaz o movimento
            revert_move(move_sequence);
        }
    }

    // Caso não encontre solução
    return "-";
}

// Resolve a etapa de orientação de edges (Edge Orientation) com brute-force
string Solver::EO() {
    if (check_state_EO() == 1) {
        return "";
    }

    int edge_ori = get_edge_ori_coord();

    File file = LittleFS.open(EO_Path, "r");
    for (int i = 0; i < edge_ori; i++) {
        file.readStringUntil('\n');
    }

    string move_sequence = string(file.readStringUntil('\n').c_str());
    file.close();

    return revert_move(move_sequence);
    ;
}

// Resolve a etapa da cruz
string Solver::cross() {
    if (check_state_EO() == 0) {
        return "-";
    } else if (check_state_cross() == 1) {
        return "";
    }
    int count = 0;        // Número de peças válidas atuais
    int next_color = -1;  // Cor que deverá vir a seguir
    int starter_color;    // Cor que começou a sequencia de peças válidas
                          // atual
    int starter_pos;      // Posição que começou a sequencia de peças válidas
                          // atual

    int best_count = 0;          // Número de peças válidas máxima
    int best_starter_color = 1;  // Cor que começou a sequencia de peças
                                 // válidas máxima
    int best_starter_pos;        // Posição que começou a sequencia de peças
                                 // válidas máxima

    // Verificar o estado inicial da cruz com a intenção de preservar o
    // máximo de peças
    for (int i = 0; i < 4; i++) {
        int main, secundary;
        tie(main, secundary) = edges[i].colors();
        // Verificar se a cor que correspode à da cruz está no sentido
        // orientado e se a cor secundária é a esperada (ou não tiver
        // definida)
        if (main == 0 && (next_color == -1 || secundary == next_color)) {
            // Atualizar valores atuais
            count++;
            next_color = ((secundary + 2) % 4) + 1;
            if (count == 1) {
                starter_pos = i;
                starter_color = secundary;
            }
        } else {
            // Atualizar melhores valores (se necessario)
            if (count > best_count) {
                best_count = count;
                best_starter_color = starter_color;
                best_starter_pos = starter_pos;
            }
            // Reset a contagem
            count = 0;
        }
    }
    // Atualizar melhores valores (se necessario)
    if (count > best_count) {
        best_count = count;
        best_starter_color = starter_color;
        best_starter_pos = starter_pos;
    }

    string move_sequence = "";

    // Se houver algum peça da cruz já colocada
    if (best_count > 0) {
        // Ajustar camada da cruz
        int adjust_U = best_starter_pos + best_count - 1;
        for (int i = 0; i < adjust_U; i++) {
            move("U'");
            move_sequence += " U'";
        }
    }

    // Percorrer as restantes peças da cruz
    for (int cross_pices = best_count; cross_pices < 4; cross_pices++) {
        next_color = ((best_starter_color - cross_pices + 3) % 4) + 1;

        // Percorrer as 12 edges
        for (int e = 1; e < 12; e++) {
            int main, secundary;
            tie(main, secundary) = edges[e].colors();
            // Quando encontrar a próxima edge da cruz
            if (main == 0 && secundary == next_color) {
                //  Se ela já estiver na posição correta ignora
                if (e == 1) {
                    move("U'");
                    move_sequence += " U'";
                    break;

                    // Se estiver na camada correta (posição errada)
                } else if (e < 4) {
                    int up_move_count = e - 1;

                    // Girar a camada da cruz até a peça encontrada estiver
                    // na posição correta
                    for (int j = 0; j < up_move_count; j++) {
                        move("U'");
                        move_sequence += " U'";
                    }
                    // Tirar temporariamente a peça do lugar
                    move("L");
                    move_sequence += " L";
                    // Reverter a camada da cruz
                    for (int j = 0; j < up_move_count; j++) {
                        move("U");
                        move_sequence += " U";
                    }
                    // Reverter peça para a sua camada
                    move("L'");
                    move_sequence += " L'";
                    // Se a peça estiver na camada oposta
                } else if (e >= 4 && e < 8) {
                    int down_move_count = (e + 3) % 4;

                    // Girar a camada oposta à da cruz até ela estiver no
                    // mesmo lado do seu lugar
                    for (int j = 0; j < down_move_count; j++) {
                        move_sequence += " D";
                        move("D");
                    }
                    // Trazer a peça para o seu lugar
                    move("L2");
                    move_sequence += " L2";

                    // Se estiver na posição Frente-Direita
                } else if (e == 8) {
                    move("F2 L' F2");
                    move_sequence += " F2 L' F2";

                    // Se estiver na posição Frente-Esquerda
                } else if (e == 9) {
                    move("L'");
                    move_sequence += " L'";

                    // Se estiver na posição Tras-Esquerda
                } else if (e == 10) {
                    move("L");
                    move_sequence += " L";

                    // Se estiver na posição Tras-Direita
                } else {
                    move("B2 L B2");
                    move_sequence += " B2 L B2";
                }

                // Se a cruz ainda não tiver feita, girar a sua camada para
                // poder por a próxima
                if (cross_pices < 3) {
                    move("U'");
                    move_sequence += " U'";
                }
                break;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        int main, secundary;
        tie(main, secundary) = edges[0].colors();
        if (secundary == 1) {
            break;
        }
        move("U");
        move_sequence += " U";
    }

    return move_sequence;
}

// Resolve a etpa de resolução de duas primeiras camadas (First 2 Layers)
string Solver::F2L() {
    if (check_state_cross() == 0) {
        return "-";
    } else if (check_state_F2L() == 1) {
        return "";
    }
    // Obter os Algoritmos
    StaticJsonDocument<1024> F2L_ALGS;
    get_algs(F2L_Json, F2L_ALGS);

    string move_sequence = "";
    for (int i = 0; i < 4; i++) {
        // Percorrer todas as edges (excepto as da cruz)
        for (int e = 4; e < 12; e++) {
            // Verifica se a edge faz parte da camada do meio
            if (edges[e].is_UDSlice()) {
                // Extrair as cores das peça
                int secundary_color, tertiary_color;
                tie(secundary_color, tertiary_color) = edges[e].colors();
                // Se a Edge estiver na camda já na camada do meio
                if (e >= 8) {
                    // Verifica se o corner faz par com a edge e o corner
                    // está orientado
                    if (corners[e - 8].pair(secundary_color, tertiary_color) &&
                        corners[e - 8].orientation() == 0) {
                        // Verifica se o pair está na posição correta
                        if ((secundary_color == 1 && tertiary_color == 2 &&
                             e == 8) ||
                            (secundary_color == 1 && tertiary_color == 4 &&
                             e == 9) ||
                            (secundary_color == 3 && tertiary_color == 4 &&
                             e == 10) ||
                            (secundary_color == 3 && tertiary_color == 2 &&
                             e == 11)) {
                            continue;
                        }
                    }
                    // Tirar da camada do meio
                    string move_to_remove_udslice;
                    if (e == 8 || e == 11) {
                        move_to_remove_udslice = "R";
                    } else {
                        move_to_remove_udslice = "L";
                    }
                    // Direção do movimento
                    if (e % 2 == 0) {
                        move_to_remove_udslice += "'";
                    }

                    // Execuatar o movibemento e fazer o seu reverso para
                    // voltar ao normal (sem a peça no sitio)
                    move(move_to_remove_udslice + " D");
                    string move_to_remove_udslice_mirror =
                        mirror_move(move_to_remove_udslice, 0, 1);
                    move(move_to_remove_udslice_mirror + " D'");

                    move_sequence += " " + move_to_remove_udslice + " D" +
                                     move_to_remove_udslice_mirror + " D'";
                    // Subtrair i pois a peça não foi encaixada, apenas
                    // retirada da camada do meio
                    i--;
                    break;
                }
                int correct_corner = 1;
                int on_cross_layer = 0;
                // Verifica se o corner está na primeira
                for (int c = 0; c < 4; c++) {
                    // Se estiver o corner estiver na primeira
                    if (corners[c].pair(secundary_color, tertiary_color)) {
                        // Mas na posição errada
                        if (!((c == 0 && secundary_color == 1 &&
                               tertiary_color == 2) ||
                              (c == 1 && secundary_color == 1 &&
                               tertiary_color == 4) ||
                              (c == 2 && secundary_color == 3 &&
                               tertiary_color == 4) ||
                              (c == 4 && secundary_color == 3 &&
                               tertiary_color == 2))) {
                            // Movimento para remover o corner
                            string move_to_remove_from_cross;
                            if (c == 0 || c == 3) {
                                move_to_remove_from_cross = "R";
                            } else {
                                move_to_remove_from_cross = "L";
                            }
                            if (c % 2 == 0) {
                                move_to_remove_from_cross += "'";
                            }
                            // Aplicar o movimento e depois o reverso
                            move(move_to_remove_from_cross + " D'");
                            string move_to_remove_from_bottom_mirror =
                                mirror_move(move_to_remove_from_cross, 0, 1);
                            move(move_to_remove_from_bottom_mirror + " D");

                            move_sequence +=
                                " " + move_to_remove_from_cross + " D'" +
                                move_to_remove_from_bottom_mirror + " D";
                            correct_corner = 0;
                            // Se estiver na posição correta
                        } else {
                            on_cross_layer = 1;
                        }
                        break;
                    }
                }

                if (correct_corner == 0) {
                    i--;
                    break;
                }
                int front;  // 1-> Front 0-> Back
                int side;   // 1-> Right 0-> Left
                int edge_pos;
                int corner_pos;

                // Saber qual dos 4 cantos é com as variaveis front e side
                if (secundary_color == 1) {
                    front = 1;
                } else {
                    front = 0;
                }
                if (tertiary_color == 2) {
                    side = 1;
                    edge_pos = 7;
                    if (front == 1) {
                        corner_pos = 4;
                    } else {
                        corner_pos = 7;
                    }
                } else {
                    side = 0;
                    edge_pos = 5;
                    if (front == 1) {
                        corner_pos = 5;
                    } else {
                        corner_pos = 6;
                    }
                }

                // Se estiver na primeira camada é preciso ajustar a
                // corner_pos
                corner_pos -= on_cross_layer * 4;
                string f2l_case;
                // Se o corner não estiver na primeira camada
                if (on_cross_layer == 0) {
                    // Girar a camada oposta à da cruz, até que o corner
                    // fique em na mesma coluna do seu lugar
                    while (corners[corner_pos].pair(secundary_color,
                                                    tertiary_color) == 0) {
                        move("D");
                        move_sequence += " D";
                    }
                    tuple<int, int> vals = {secundary_color, tertiary_color};
                    for (int edge_pos = 4; edge_pos < 8; edge_pos++) {
                        // Percorrer as edges do topo
                        if (edges[edge_pos].colors() == vals) {
                            string edge_pos_json;
                            string corner_ori;
                            // Se a peça não tiver de ir para frente-direita ou
                            // tras-esquerda
                            if (side != front) {
                                // Espelhar
                                switch ((corner_pos - edge_pos + 4) % 4) {
                                    case 0:
                                        edge_pos_json = "1";
                                        break;
                                    case 1:
                                        edge_pos_json = "0";
                                        break;
                                    case 2:
                                        edge_pos_json = "3";
                                        break;
                                    case 3:
                                        edge_pos_json = "2";
                                        break;
                                }

                                switch (corners[corner_pos].orientation()) {
                                    case 1:
                                        corner_ori = "2";
                                        break;
                                    case 2:
                                        corner_ori = "1";
                                        break;

                                    default:
                                        corner_ori = "0";
                                        break;
                                }

                            } else {
                                corner_ori = to_string(
                                    corners[corner_pos].orientation());
                                edge_pos_json =
                                    to_string((corner_pos - edge_pos + 4) % 4);
                            }
                            f2l_case = (F2L_ALGS["corner_top"][corner_ori]
                                                [edge_pos_json]
                                                    .as<string>());
                            break;
                        }
                    }

                } else {
                    tuple<int, int> vals = {secundary_color, tertiary_color};
                    // Girar a camada oposta à da cruz, até que a edge fique
                    // na edge_pos
                    while (edges[edge_pos].colors() != vals) {
                        move("D");
                        move_sequence += " D";
                    }
                    string corner_ori;
                    // Se a peça não tiver de ir para frente-direita ou
                    // tras-esquerda
                    if (side != front) {
                        // Espelhar
                        switch (corners[corner_pos].orientation()) {
                            case 1:
                                corner_ori = "2";
                                break;
                            case 2:
                                corner_ori = "1";
                                break;

                            default:
                                corner_ori = "0";
                                break;
                        }
                    } else {
                        corner_ori =
                            to_string(corners[corner_pos].orientation());
                    }

                    f2l_case =
                        (F2L_ALGS["corner_bottom"][corner_ori].as<string>());
                }
                // Aplicar espelhamento
                f2l_case = mirror_move(f2l_case, !side, !front);
                move(f2l_case);
                move_sequence += " " + f2l_case;

                break;
            }
        }
    }

    return move_sequence;
}

// Resolve a etapa de orientação da ultima camada (Orientation of the Last
// Layer)
string Solver::OLL() {
    if (check_state_F2L() == 0) {
        return "-";
        // Verificar OLL skip
    } else if (is_solved_face(get_D())) {
        return "";
    }

    // Reconhecimento da ultima camada
    int corner_orientation[4];
    for (int c = 0; c < 4; c++) {
        corner_orientation[c] = corners[c + 4].orientation();
    }

    StaticJsonDocument<1024> OLL_ALGS;
    get_algs(OLL_Json, OLL_ALGS);

    string move_sequence = "";
    string oll_case;
    do {
        oll_case = array_to_string(corner_orientation, 4);

        // Verificar se o caso de OLL existe
        if (OLL_ALGS.containsKey(oll_case)) {
            break;
        } else {
            // Se não existir é aplicado a face é girada
            roll_array(corner_orientation, 4);

            move("D'");
            move_sequence += " D'";
        }

    } while (1);

    // Aplicar movimentos
    string oll_alg = OLL_ALGS[oll_case].as<string>();
    move(oll_alg);

    return move_sequence + " " + oll_alg;
}

// Resolve a etapa de permutação da ultima camada (Permutation of the Last
// Layer)
string Solver::PLL() {
    if (check_state_OLL() == 0) {
        return "-";
    }

    // Estado das das 12 cores da ultima camada
    int last_layer_state[12] = {};
    for (int i = 0; i < 4; i++) {
        // Obter as cores
        int top, color1, color2, color3;
        tie(top, color1, color3) = corners[i + 4].colors();
        color2 = get<1>(edges[i + 4].colors());
        // Preencher array
        last_layer_state[i * 3] = color1;
        last_layer_state[(i * 3) + 1] = color2;
        last_layer_state[((i * 3 + 11) % 12)] = color3;
    }

    StaticJsonDocument<1024> PLL_ALGS;
    get_algs(PLL_Json, PLL_ALGS);

    string move_sequence = "";
    string pll_case;
    // Percorrer cada uma das faces
    for (int t = 0; t < 4; t++) {
        // Normalizar (1 como primeiro valor)
        int fst_val = last_layer_state[0];
        for (int i = 0; i < 12; i++) {
            last_layer_state[i] = ((last_layer_state[i] + 4 - fst_val) % 4) + 1;
        }

        // Verifica se existe alguma PLL com aquela orientação
        string last_layer_state_str = array_to_string(last_layer_state, 12);
        if (PLL_ALGS.containsKey(last_layer_state_str)) {
            move_sequence += PLL_ALGS[last_layer_state_str].as<string>();
            break;
        }
        // Se não a face é girada (roll de 3)
        roll_array(last_layer_state, 12, 3);
        move_sequence += "D' ";
    }
    move(move_sequence);
    return move_sequence + " " + finish_last_layer();
}

// Resolve todas as etapas do cubo
string Solver::solve() {
    string move_sequence = "";

    // Resolve EO
    string move_sequence_EO = EO();
    if (move_sequence_EO == "-") {
        return "-";
    }
    move_sequence += move_sequence_EO + " ";

    // Resolve cross
    string move_sequence_cross = cross();
    if (move_sequence_cross == "-") {
        return "-";
    }
    move_sequence += move_sequence_cross + " ";

    // Resolve F2L
    string move_sequence_F2L = F2L();
    if (move_sequence_F2L == "-") {
        return "-";
    }
    move_sequence += move_sequence_F2L + " ";

    // Resolve OLL
    string move_sequence_OLL = OLL();
    if (move_sequence_OLL == "-") {
        return "-";
    }
    move_sequence += move_sequence_OLL + " ";

    // Resolve PLL
    string move_sequence_PLL = PLL();
    if (move_sequence_PLL == "-") {
        return "-";
    }
    move_sequence += move_sequence_PLL;

    return simplify_move(move_sequence);
}

// Verificar se o cubo está resolvido
int Solver::is_solved() {
    Face face_list[6] = {get_U(), get_F(), get_R(), get_B(), get_L(), get_D()};
    // Percorrer todas as faces
    for (int i = 0; i < 6; i++) {
        if (is_solved_face(face_list[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

// Simplifica a sequencia de movimentos
string Solver::simplify_move(const string move_string) {
    stringstream ss(move_string);

    vector<string> moves_seq;

    string move;

    // Percorre a string e adiciona cada movimento a um vetor
    while (ss >> move) {
        moves_seq.push_back(move);
    }

    vector<string> new_moves_seq;

    for (size_t i = 0; i < moves_seq.size(); i++) {
        move = moves_seq[i];
        if (new_moves_seq.size() > 0) {
            string last_move = new_moves_seq.back();
            // Mesmo tipo de movimento
            if (last_move[0] == move[0]) {
                if (last_move.size() == 1) {
                    // Movimento X X = X2
                    if (move.size() == 1) {
                        new_moves_seq.back() = string(1, last_move[0]) + "2";
                        // Movimento X X' = ∅
                    } else if (move[1] == '\'') {
                        new_moves_seq.pop_back();
                        // Movimento X X2 = X'
                    } else if (move[1] == '2') {
                        new_moves_seq.back() = string(1, last_move[0]) + "'";
                    }
                } else if (last_move[1] == '\'') {
                    // Movimento X' X = ∅
                    if (move.size() == 1) {
                        new_moves_seq.pop_back();
                        // Movimento X' X' = X2
                    } else if (move[1] == '\'') {
                        new_moves_seq.back() = string(1, last_move[0]) + "2";
                        // Movimento X' X2 = X
                    } else if (move[1] == '2') {
                        new_moves_seq.back() = last_move[0];
                    }
                } else if (last_move[1] == '2') {
                    // Movimento X2 X = X'
                    if (move.size() == 1) {
                        new_moves_seq.back() = string(1, last_move[0]) + "'";
                        // Movimento X2 X' = X
                    } else if (move[1] == '\'') {
                        new_moves_seq.back() = string(1, last_move[0]);
                        // Movimento X2 X2 = ∅
                    } else if (move[1] == '2') {
                        new_moves_seq.pop_back();
                    }
                }
            } else {
                new_moves_seq.push_back(move);
            }
        } else {
            new_moves_seq.push_back(move);
        }
    }

    string new_move_string = "";
    for (size_t i = 0; i < new_moves_seq.size(); i++) {
        new_move_string += new_moves_seq[i] + " ";
    }
    return new_move_string;
}

// Aplica um embaralhamento de um determinado tamanho
string Solver::scramble(int size) {
    string move_set[] = {"U",  "F",  "R",  "B",  "L",  "D",  "U'", "F'", "R'",
                         "B'", "L'", "D'", "U2", "F2", "R2", "B2", "L2", "D2"};

    // Obter tamanho da sequencia de movimentos
    auto move_sequence_size = [](string move_sequence) {
        int counter = 0;
        bool last_is_space = true;
        // Percorrer todos os caracteres da string
        for (int c = 0; c < move_sequence.size(); c++) {
            if (!last_is_space && move_sequence[c] == ' ') {
                counter++;
                last_is_space = true;
            } else {
                last_is_space = false;
            }
        }
        // Adicionar mais 1 ao número de espaços caso não tenha terminado em
        // espaço
        if (!last_is_space) {
            counter++;
        }
        return counter;
    };

    string move_sequence = "";
    // Adicionar um moviemnto aletório
    while (move_sequence_size(move_sequence) < size) {
        int move_index = rand() % 18;
        move_sequence += move_set[move_index] + " ";
        move_sequence = simplify_move(move_sequence);
    }
    // Simplifica a sequencia de movimentos, aplica-a e retorna a mesma
    move(move_sequence);
    return move_sequence;
}

// Obtem o estado atual do cubo organizado por peças
array<array<int, 3>, 26> Solver::piece_state() {
    // Array de peças com em que cada peça é representada por outro array com as
    // cores na ordem: {TOP-DOWN, RIGHT-LEFT, FRONT-BACK}
    array<array<int, 3>, 26> state;

    // É necessário trocar a ordem para que fique num posição melhor (para o web
    // server)

    // Nova posição dos corners
    int corner_pos[] = {0, 17, 19, 2, 6, 23, 25, 8};
    for (int i = 0; i < 8; i++) {
        int color1, color2, color3;
        // Decidir ordem das cores
        if (i % 2 == 0 && i < 4 || i % 2 != 0 && i >= 4) {
            tie(color1, color3, color2) = corners[i].colors();
        } else {
            tie(color1, color2, color3) = corners[i].colors();
        }
        // Adicionar ao array
        state[corner_pos[i]] = {color1, color2, color3};
    }

    // Nova posição dos edges
    int edge_pos[] = {9, 18, 11, 1, 14, 24, 16, 7, 3, 20, 22, 5};
    for (int i = 0; i < 12; i++) {
        int color1, color2;
        tie(color1, color2) = edges[i].colors();
        // Decidir ordem das cores
        if (i < 8 && i % 2 == 0) {
            state[edge_pos[i]] = {color1, color2, -1};
        } else if (i < 8) {
            state[edge_pos[i]] = {color1, -1, color2};
        } else {
            state[edge_pos[i]] = {-1, color1, color2};
        }
    }

    // Nova posição dos centros
    int center_pos[] = {10, 12, 4, 13, 21, 15};
    for (int i = 0; i < 6; i++) {
        // Igual em todas os lados
        state[center_pos[i]] = {i, i, i};
    }

    return state;
}

// Obtem o estado atual do cubo organizado por cores (o método que é usado para
// criar o objeto)
string Solver::sticker_state() {
    // Converter face em string
    auto faceToString = [](Face face) -> string {
        ostringstream oss;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                oss << face[i][j];
            }
        }
        string resultado = oss.str();
        return resultado;
    };

    // Juntar todas as faces
    string cube_state = faceToString(get_U());
    cube_state += faceToString(get_F());
    cube_state += faceToString(get_R());
    cube_state += faceToString(get_B());
    cube_state += faceToString(get_L());
    cube_state += faceToString(get_D());

    return cube_state;
}

// Transforma o cubo em outro cubo e retorna a sequencia de movimentos para tal
string Solver::match_state(Solver& cube_to_match) {
    // Cria um objeto temporário com o estado atual do cubo e resolve-o
    Solver* start_cube = new Solver(sticker_state());
    string move_sequence_1 = start_cube->solve();
    // Apaga o objeto para não pesar na memória
    delete start_cube;
    start_cube = nullptr;

    // Criar cópia temporária do objeto para não modificar o cubo que foi
    // passado como argumento e resolve-o
    Solver* end_cube = new Solver(cube_to_match.sticker_state());
    string move_sequence_2 = end_cube->solve();
    // Apaga o objeto para não pesar na memória
    delete end_cube;
    end_cube = nullptr;

    // Cria um cubo para aplicar as soluções anteriores
    Solver final_cube(Solver::solved_string());
    // Cria cubo com equivalente à distancia entre os dois estados e resolve-o
    final_cube.revert_move(move_sequence_1 + " " + move_sequence_2);
    string final_move_sequence = final_cube.solve();

    // Aplica a solução e retorna-a
    move(final_move_sequence);
    return final_move_sequence;
}