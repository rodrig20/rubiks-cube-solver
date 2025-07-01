var rubik;
var enbable_manual_move = true;
var sleep_time = 500;

async function make_moves(moves_seq) {
	// Verificar se a string está vazia ou é inválida
	if (!moves_seq || moves_seq.trim() === "" || moves_seq.trim() === "-") {
		return;
	}

	const moves = moves_seq.split(/\s+/); // Divide a string pelos espaços

	for (const move of moves) {
		// Pular movimentos vazios
		if (!move || move.trim() === "") {
			continue;
		}
		console.log(move)
		if (move == "U2") {
			rubik.rotateY(1, 0, false); await sleep(sleep_time); rubik.rotateY(1, 0, false);
		}
		else if (move == "U'")
			rubik.rotateY(-1, 0, false);
		else if (move == "U")
			rubik.rotateY(1, 0, false);
		else if (move == "F2") {
			rubik.rotateX(1, 0, false); await sleep(sleep_time); rubik.rotateX(1, 0, false)
		}
		else if (move == "F'")
			rubik.rotateX(1, 0, false)
		else if (move == "F")
			rubik.rotateX(-1, 0, false)
		else if (move == "R2") {
			rubik.rotateZ(1, 0, false); await sleep(sleep_time); rubik.rotateZ(1, 0, false)
		}
		else if (move == "R'")
			rubik.rotateZ(-1, 0, false)
		else if (move == "R")
			rubik.rotateZ(1, 0, false)
		else if (move == "B2") {
			rubik.rotateX(1, 2, false); await sleep(sleep_time); rubik.rotateX(1, 2, false);
		}
		else if (move == "B'")
			rubik.rotateX(-1, 2, false)
		else if (move == "B")
			rubik.rotateX(1, 2, false)
		else if (move == "L2") {
			rubik.rotateZ(-1, 2, false); await sleep(sleep_time); rubik.rotateZ(-1, 2, false)
		}
		else if (move == "L'")
			rubik.rotateZ(1, 2, false)
		else if (move == "L")
			rubik.rotateZ(-1, 2, false)
		else if (move == "D2") {
			rubik.rotateY(-1, 2, false); await sleep(sleep_time); rubik.rotateY(-1, 2, false);
		}
		else if (move == "D'")
			rubik.rotateY(1, 2, false)
		else if (move == "D")
			rubik.rotateY(-1, 2, false)
		await sleep(525);

	}
}

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

async function getCube() {
	try {
		let response = await fetch('/cubestate');
		if (!response.ok) {
			throw new Error(`Erro HTTP: ${response.status}`);
		}
		let data = await response.text(); // Lê a resposta como string
		rubik = new Rubik(data);
	} catch (error) {
		// Resolvido
		rubik = new Rubik("[[0, 1, 2],[0, -1, 2],[0, 3, 2],[-1, 1, 2],[2, 2, 2],[-1, 3, 2],[5, 1, 2],[5, -1, 2],[5, 3, 2],[0, 1, -1],[0, 0, 0],[0, 3, -1],[1, 1, 1],[3, 3, 3],[5, 1, -1],[5, 5, 5],[5, 3, -1],[0, 1, 4],[0, -1, 4],[0, 3, 4],[-1, 1, 4],[4, 4, 4],[-1, 3, 4],[5, 1, 4],[5, -1, 4],[5, 3, 4]]");
		console.error("Erro:", error);
	}
}

function applyMoveSequence() {
	const moveSequence = document.getElementById("move-sequence").value.trim();
	if (moveSequence) {
		if (enbable_manual_move) {
			enbable_manual_move = false;
			send_moves(moveSequence);
			make_moves(moveSequence).then(() => {
				enbable_manual_move = true;
			});
		}
	}
}


function send_moves(move) {
	fetch('/sendmove', {
		method: 'POST',
		headers: {
			'Content-Type': 'text/plain'
		},
		body: move
	}).catch(() => { });
}

async function solve() {
	if (enbable_manual_move) {
		enbable_manual_move = false;
		try {
			let response = await fetch('/solve');
			if (!response.ok) {
				throw new Error(`Erro HTTP: ${response.status}`);
			}
			let data = await response.text();
			console.log("DATA: "+ data)

			// Verificar se data é válido e não é "-"
			if (data && data.trim() !== "-") {
				make_moves(data).then(() => {
					enbable_manual_move = true;
				});
			} else {
				alert("Invalid Cube State");
				enbable_manual_move = true;
			}

		} catch (error) {
			alert("Invalid Cube State");
			enbable_manual_move = true;
		}
	}
}

async function scramble() {
	if (enbable_manual_move) {
		enbable_manual_move = false;
		try {
			let response = await fetch('/scramble');
			if (!response.ok) {
				throw new Error(`Erro HTTP: ${response.status}`);
			}
			let data = await response.text();

			make_moves(data).then(() => {
				enbable_manual_move = true;
			});

		} catch (error) {
			console.log("Erro: " + error)
			enbable_manual_move = true;
		}
	}
}

async function scan() {
	if (enbable_manual_move) {
		enbable_manual_move = false;
		try {
			// Remove o cubo existente do DOM se existir
			if (rubik && rubik._node && rubik._node.parentNode) {
				rubik._node.parentNode.removeChild(rubik._node);
			}
			let response = await fetch('/scan');
			if (!response.ok) {
				throw new Error(`Erro HTTP: ${response.status}`);
			}
			let data = await response.text(); // Lê a resposta como string
			rubik = new Rubik(data);
			enbable_manual_move = true;
		} catch (error) {
			// Resolvido
			rubik = new Rubik("[[0, 1, 2],[0, -1, 2],[0, 3, 2],[-1, 1, 2],[2, 2, 2],[-1, 3, 2],[5, 1, 2],[5, -1, 2],[5, 3, 2],[0, 1, -1],[0, 0, 0],[0, 3, -1],[1, 1, 1],[3, 3, 3],[5, 1, -1],[5, 5, 5],[5, 3, -1],[0, 1, 4],[0, -1, 4],[0, 3, 4],[-1, 1, 4],[4, 4, 4],[-1, 3, 4],[5, 1, 4],[5, -1, 4],[5, 3, 4]]");
			console.error("Erro:", error);
			enbable_manual_move = true;
		}
	}
}

function state2Num(color) {
	if (color === "white") return 0;
	if (color === "green") return 1;
	if (color === "red") return 2;
	if (color === "blue") return 3;
	if (color === "orange") return 4;
	if (color === "yellow") return 5;
	if (color === "black") return -1;
	return "?";
}

function getCubeStateString(rubik) {
	const size = Rubik.SIZE;
	const faces = [
		{ name: 'U', idx: Face.TOP, fixed: 'y', value: 0, var1: 'z', var2: 'x', order: [1, 1] },
		{ name: 'L', idx: Face.LEFT, fixed: 'x', value: 0, var1: 'y', var2: 'z', order: [1, 1] },
		{ name: 'F', idx: Face.FRONT, fixed: 'z', value: 0, var1: 'y', var2: 'x', order: [1, 1] },
		{ name: 'R', idx: Face.RIGHT, fixed: 'x', value: 2, var1: 'y', var2: 'z', order: [1, -1] },
		{ name: 'B', idx: Face.BACK, fixed: 'z', value: 2, var1: 'y', var2: 'x', order: [1, -1] },
		{ name: 'D', idx: Face.BOTTOM, fixed: 'y', value: 2, var1: 'z', var2: 'x', order: [-1, 1] },
	];

	let allDigits = "";

	faces.forEach(face => {
		let matrix = [];
		for (let i = 0; i < size; i++) {
			let row = [];
			for (let j = 0; j < size; j++) {
				let x, y, z;
				if (face.fixed === 'x') {
					x = face.value;
					y = face.order[0] === 1 ? i : size - 1 - i;
					z = face.order[1] === 1 ? j : size - 1 - j;
				} else if (face.fixed === 'y') {
					y = face.value;
					z = face.order[0] === 1 ? i : size - 1 - i;
					x = face.order[1] === 1 ? j : size - 1 - j;
				} else { // fixed === 'z'
					z = face.value;
					y = face.order[0] === 1 ? i : size - 1 - i;
					x = face.order[1] === 1 ? j : size - 1 - j;
				}
				let cubeIndex = x + y * size + z * size * size;
				let cube = rubik._cubes[cubeIndex];
				let color = cube && cube.getFaces()[face.idx] ? cube.getFaces()[face.idx].getColor() : 'black';
				row.push(color);
			}
			matrix.push(row);
		}

		// Transformações específicas para cada face
		if (face.name === 'U') {
			let transposed = matrix[0].map((_, col) => matrix.map(row => row[col]));
			transposed = transposed.reverse();
			transposed = transposed.map(row => row.reverse());
			matrix = transposed;
		} else if (face.name === 'D') {
			let transposed = matrix[0].map((_, col) => matrix.map(row => row[col]));
			transposed = transposed.map(row => row);
			//transposed = transposed.reverse();
			matrix = transposed;
		} else if (face.name === 'R' || face.name === 'L') {
			matrix = matrix.map(row => row.reverse());
			//}  else if (face.name === 'F') {
			//    matrix = matrix.reverse(); // Espelha as linhas da face F
		}

		let flat = matrix.flat();
		let digits = flat.map(state2Num).join('');
		allDigits += digits;
	});

	return allDigits;
}

function sendCubeStateUpdate() {
	const stateString = getCubeStateString(rubik);
	console.log(stateString);
	fetch('/update_state', {
		method: 'POST',
		headers: {
			'Content-Type': 'text/plain'
		},
		body: stateString
	});
}

function resetOrientation() {
	showCenterLetters();
	sendCubeStateUpdate();
}
