import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

# Desenha um quadrado (autocolante do cubo) numa determinada face
def draw_sticker(ax, verts, color):
    square = Poly3DCollection([verts], facecolors=color, edgecolors='k', linewidths=0.5)
    ax.add_collection3d(square)

# Gera as coordenadas dos 9 quadrados (3x3) de uma face do cubo
def cube_face_coords(start, u_dir, v_dir, size=1.0):
    coords = []
    for i in range(3):
        for j in range(3):
            # Define os 4 vértices de cada quadrado a partir do ponto inicial
            p0 = start + u_dir * i * size + v_dir * j * size
            p1 = p0 + u_dir * size
            p2 = p1 + v_dir * size
            p3 = p0 + v_dir * size
            coords.append([p0, p1, p2, p3])
    return coords

# Função principal que desenha as faces selecionadas do cubo com as cores fornecidas
def plot_selected_faces(sticker_colors):
    fig = plt.figure(figsize=(8,8))
    ax = fig.add_subplot(111, projection='3d')
    s = 1.0  # tamanho de cada quadrado

    # Define as faces a desenhar: posição inicial, direção horizontal (u_dir), direção vertical (v_dir)
    faces = {
        'top':   (np.array([0, 0, 3]), np.array([1, 0, 0]), np.array([0, 1, 0])),
        # 'bottom': (np.array([0, 0, 0]), np.array([1, 0, 0]), np.array([0, 1, 0])),
        'right':  (np.array([3, 0, 0]), np.array([0, 0, 1]), np.array([0, 1, 0])),
        'front': (np.array([0, 0, 0]), np.array([1, 0, 0]), np.array([0, 0, 1])),
    }

    # Para cada face, desenha os 9 quadrados com as cores definidas
    for face, (start, u_dir, v_dir) in faces.items():
        coords = cube_face_coords(start, u_dir, v_dir, s)
        for i, verts in enumerate(coords):
            draw_sticker(ax, verts, sticker_colors[face][i])

    # Define o ângulo da câmara para uma visualização agradável
    ax.view_init(elev=25, azim=-45)

    ax.set_box_aspect([3,3,3])  # mantém proporções iguais em todos os eixos
    ax.set_xlim(0,3); ax.set_ylim(0,3); ax.set_zlim(0,3)
    ax.axis('off')  # remove os eixos para um aspecto mais limpo

    # Guarda a imagem final como ficheiro, sem margens
    plt.savefig("/home/rodri/Faculdade/Projeto/F2L.png", bbox_inches='tight', pad_inches=0, dpi=400)
    plt.close(fig)  # liberta memória fechando a figura

    plt.show()  # opcional: mostra a imagem numa janela interativa

# Dicionário com as cores de cada face (cada face tem 9 quadrados)
cores = {
    'top':   ['white']*9,
    # 'bottom':   ['white']*9,
    'right':  ['red']*9,
    'front': ['green']*9,
}

# Chama a função para desenhar o cubo com as cores definidas
plot_selected_faces(cores)
