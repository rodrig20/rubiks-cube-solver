import twophase.defs as defs

defs.FOLDER = "./tools/twophase"
import twophase.solver as sv


import requests
from bs4 import BeautifulSoup
import os
import json


def get_alg_size(alg: str):
    alg = alg.strip()
    size = alg.count(" ") + 1
    for char in alg:
        if char in ["x", "y", "z"]:
            size -= 1
        elif char in ["M", "E", "S"]:
            size += 1
    return size


def to_state(face: str, side: str = ""):
    face = face[::-1].upper()
    if side != "":
        face = face[-3:]
    face = face.replace("Y", "D")
    face = face.replace("G", "F")
    face = face.replace("O", "L")
    if side != "":
        return side * 6 + face
    else:
        return face


def get_normalize(state_: str):
    state = state_
    side2num = {"U": "0", "F": "1", "R": "2", "B": "3", "L": "4", "D": "5"}
    num_state = "".join(map(side2num.get, state))
    last_layer_side_stickers = [
        (i * 9) + j - 1 for i in [2, 4, 5, 1] for j in range(9, 6, -1)
    ]
    main_sticker_num = int(num_state[25])
    lalast_layer_stickers = last_layer_side_stickers + [27, 29, 33, 35]
    if main_sticker_num != 1:
        norm = ""
        for i in lalast_layer_stickers:
            if num_state[i] != "5":
                norm += str(((int(num_state[i]) + 4 - main_sticker_num) % 4) + 1)
            else:
                norm += "5"
        return norm
    else:
        return "".join(num_state[i] for i in lalast_layer_stickers)


zbll_types = ["U", "L", "T", "H", "Pi", "S", "AS"]
for z, zbll in enumerate(zbll_types):
    print("=" * 20)
    print(f"Getting ZBLL {zbll} [{z+1}/{len(zbll_types)}]...")
    print("=" * 20)
    req = requests.get(f"https://www.speedcubedb.com/a/3x3/ZBLL{zbll}")

    soup = BeautifulSoup(req.text, "html.parser")
    divs = soup.find_all("div", class_="singlealgorithm")

    zbll_dict = {}
    os.makedirs("./data/info/ZBLL", exist_ok=True)

    for i, div in enumerate(divs):
        if i % 10 == 0:
            print(f"\n{i} of {len(divs)}\n")

        cube = div.find("div", class_="jcube")
        if cube is not None:
            front = to_state(cube["data-uf"], "F")
            left = to_state(cube["data-ur"], "L")
            back = to_state(cube["data-ub"], "B")
            down = to_state(cube["data-us"], "")
            right = to_state(cube["data-ul"], "R")

            cubestring = f"UUUUUUUUU{right}{front}{down}{left}{back}"
            print(f"Cube: {cubestring}")
            norm = get_normalize(cubestring)

            solution = sv.solve(cubestring, 0, 4)
            idx = solution.find("(")
            solution = solution[: idx - 1]
            solution = solution.replace("3", "'").replace("1", "")

            print(f"Solution: {solution}")
            print(f"Normalization: {norm}")

            zbll_dict[norm] = solution

    with open(f"./data/info/ZBLL/ZBLL{zbll}.json", "w") as f:
        json.dump(zbll_dict, f, indent=4)
