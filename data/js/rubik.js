Array.prototype.clone = function () {
	var c = [];
	var len = this.length;
	for (var i = 0; i < len; i++) { c.push(this[i]); }
	return c;
}

Array.prototype.random = function () {
	return this[Math.floor(Math.random() * this.length)];
}

var Face = OZ.Class();
Face.SIZE = 100;
Face.LEFT = 0;
Face.RIGHT = 1;
Face.TOP = 2;
Face.BOTTOM = 3;
Face.FRONT = 4;
Face.BACK = 5;

Face.ROTATION = [
	[Face.TOP, Face.FRONT, Face.BOTTOM, Face.BACK].reverse(),
	[Face.LEFT, Face.BACK, Face.RIGHT, Face.FRONT].reverse(),
	[Face.LEFT, Face.BOTTOM, Face.RIGHT, Face.TOP].reverse()
];

Face.prototype.init = function (cube, type) {
	this._cube = cube;
	this._type = type;
	this._color = null;
	this._node = OZ.DOM.elm("div", { className: "face face" + type, width: Face.SIZE + "px", height: Face.SIZE + "px", position: "absolute", left: "0px", top: "0px" });
	OZ.CSS3.set(this._node, "box-sizing", "border-box");
	//	OZ.CSS3.set(this._node, "backface-visibility", "hidden");

	switch (type) {
		case Face.LEFT:
			OZ.CSS3.set(this._node, "transform-origin", "100% 50%");
			OZ.CSS3.set(this._node, "transform", "translate3d(-" + Face.SIZE + "px, 0px, 0px) rotateY(-90deg)");
			break;
		case Face.RIGHT:
			OZ.CSS3.set(this._node, "transform-origin", "0% 50%");
			OZ.CSS3.set(this._node, "transform", "translate3d(" + Face.SIZE + "px, 0px, 0px) rotateY(90deg)");
			break;
		case Face.TOP:
			OZ.CSS3.set(this._node, "transform-origin", "50% 100%");
			OZ.CSS3.set(this._node, "transform", "translate3d(0px, -" + Face.SIZE + "px, 0px) rotateX(90deg)");
			break;
		case Face.BOTTOM:
			OZ.CSS3.set(this._node, "transform-origin", "50% 0%");
			OZ.CSS3.set(this._node, "transform", "translate3d(0px, " + Face.SIZE + "px, 0px) rotateX(-90deg)");
			break;
		case Face.FRONT:
			break;
		case Face.BACK:
			OZ.CSS3.set(this._node, "transform", "translate3d(0px, 0px, -" + Face.SIZE + "px) rotateY(180deg)");
			break;
	}

	this._node._faceObj = this;
}

Face.prototype.getCube = function () {
	return this._cube;
}

Face.prototype.getNode = function () {
	return this._node;
}

Face.prototype.getType = function () {
	return this._type;
}

Face.prototype.setColor = function (color) {
	this._color = color;
	this._node.style.backgroundColor = color;
}

Face.prototype.getColor = function () {
	return this._color;
}

var Cube = OZ.Class();
Cube.prototype.init = function (position) {
	this._rotation = null;
	this._position = position;
	this._node = OZ.DOM.elm("div", { className: "cube", position: "absolute", width: Face.SIZE + "px", height: Face.SIZE + "px" });
	this._faces = {};
	this._tmpFaces = {};
	OZ.CSS3.set(this._node, "transform-style", "preserve-3d");

	this._update();
}

Cube.prototype.getFaces = function () {
	return this._faces;
}

Cube.prototype.setFace = function (type, color) {
	if (!(type in this._faces)) {
		var face = new Face(this, type);
		this._node.appendChild(face.getNode());
		this._faces[type] = face;

		// Adiciona a letra correspondente nos centros das faces externas
		const pos = this.getPosition();
		let centerLetter = null;
		if (type === Face.TOP    && pos[0] === 1 && pos[1] === 0 && pos[2] === 1) centerLetter = 'U';
		else if (type === Face.BOTTOM && pos[0] === 1 && pos[1] === 2 && pos[2] === 1) centerLetter = 'D';
		else if (type === Face.FRONT  && pos[0] === 1 && pos[1] === 1 && pos[2] === 0) centerLetter = 'R';
		else if (type === Face.BACK   && pos[0] === 1 && pos[1] === 1 && pos[2] === 2) centerLetter = 'L';
		else if (type === Face.LEFT   && pos[0] === 0 && pos[1] === 1 && pos[2] === 1) centerLetter = 'F';
		else if (type === Face.RIGHT  && pos[0] === 2 && pos[1] === 1 && pos[2] === 1) centerLetter = 'B';
		if (centerLetter) {
			let extraStyle = "";
			if (centerLetter === 'U') {
				extraStyle = "transform:rotate(90deg);";
			}
			else if (centerLetter === 'D') {
				extraStyle = "transform:rotate(-90deg);";
			}
			face.getNode().innerHTML = `<span style="display:flex;align-items:center;justify-content:center;width:100%;height:100%;font-size:2.5em;font-weight:bold;pointer-events:none;color:black;${extraStyle}">${centerLetter}</span>`;
		}
	}
	this._faces[type].setColor(color);
}

Cube.prototype.setRotation = function (rotation) {
	this._rotation = rotation;
	this._update();
}

Cube.prototype.complete = function () {
	for (var i = 0; i < 6; i++) {
		if (i in this._faces) { continue; }
		this.addFace(i, "black");
	}
}

Cube.prototype.prepareColorChange = function (sourceCube, rotation) {
	this._tmpFaces = {};
	var sourceFaces = sourceCube.getFaces();
	for (var p in sourceFaces) {
		var sourceType = parseInt(p);
		var targetType = this.rotateType(sourceType, rotation);
		this._tmpFaces[targetType] = sourceFaces[sourceType].getColor();
	}
}

Cube.prototype.commitColorChange = function () {
	//	var parent = this._node.parentNode;
	//	parent.removeChild(this._node);

	OZ.DOM.clear(this._node);
	this._faces = {};
	for (var p in this._tmpFaces) {
		var type = parseInt(p);
		this.setFace(type, this._tmpFaces[p]);
	}
	this._tmpFaces = {};

	this._rotation = null;
	this._update();
	//	parent.appendChild(this._node);
}

Cube.prototype.rotateType = function (type, rotation) {
	for (var i = 0; i < 3; i++) {
		if (!rotation[i]) { continue; }
		var faces = Face.ROTATION[i];
		var index = faces.indexOf(type);
		if (index == -1) { continue; } /* no rotation available */
		index = (index + rotation[i] + faces.length) % faces.length;
		return faces[index];
	}

	return type;
}

Cube.prototype._update = function() {
	var transform = "";
	transform += "translate3d("+(-Face.SIZE/2)+"px, "+(-Face.SIZE/2)+"px, "+(-Face.SIZE/2)+"px) ";
	if (this._rotation) { transform += this._rotation + " "; }

	var half = Math.floor(Rubik.SIZE/2) - (Rubik.SIZE % 2 === 0 ? 1/2 : 0);
	var x = this._position[0];
	var y = this._position[1];
	var z = -this._position[2];
	x -= half;
	y -= half;
	z += half + 1/2;
	transform += "translate3d("+(x*Face.SIZE)+"px, "+(y*Face.SIZE)+"px, "+(z*Face.SIZE)+"px)";

	var prop = OZ.CSS3.getProperty("transform");
	var val = this._rotation ? prop + " 300ms" : "";
	OZ.CSS3.set(this._node, "transition", val);

	OZ.CSS3.set(this._node, "transform", transform);
}

Cube.prototype.getPosition = function () {
	return this._position;
}

Cube.prototype.getNode = function () {
	return this._node;
}

Cube.prototype.getFaces = function () {
	return this._faces;
}

var Rubik = OZ.Class();
Rubik.SIZE = 3;
Rubik.prototype.init = function (state) {
	this._init_state = JSON.parse(state)
	this._cubes = [];
	this._faces = [];
	this._faceNodes = [];
	this._help = {};
	this._drag = {
		ec: [],
		mouse: [],
		face: null
	};

	this._rotation = Quaternion.fromRotation([1, 0, 0], -30).multiply(Quaternion.fromRotation([0, 1, 0], 60));
	this._node = OZ.DOM.elm("div", {
		position: "relative",
		width: "0px",
		height: "0px",
		margin: "auto"
	});

	// Adicionar o cubo à div cube-container em vez do body
	const cubeContainer = document.getElementById("cube-container");
	if (cubeContainer) {
		cubeContainer.appendChild(this._node);
	} else {
		document.body.appendChild(this._node);
	}

	OZ.CSS3.set(this._node, "transform-style", "preserve-3d");

	this._build();
	this._update();
	OZ.Event.add(document.body, "mousedown touchstart", this._dragStart.bind(this));

	//setTimeout(this.randomize.bind(this), 500);
}

Rubik.prototype._update = function () {
	OZ.CSS3.set(this._node, "transform", "translateZ(" + (-Face.SIZE / 2 - Face.SIZE) + "px) " + this._rotation.toRotation() + " translateZ(" + (Face.SIZE / 2) + "px)");
}

Rubik.prototype._eventToFace = function (e) {
	if (document.elementFromPoint) {
		e = (e.touches ? e.touches[0] : e);
		var node = document.elementFromPoint(e.clientX, e.clientY);
	} else {
		var node = OZ.Event.target(e);
	}
	var index = this._faceNodes.indexOf(node);
	if (index == -1) { return null; }
	return this._faces[index];
}

Rubik.prototype._dragStart = function (e) {
    let target = e.target;
    while (target) {
        if (target.id === 'move-sequence' || target.id === 'button-container') {
            return;
        }
        target = target.parentElement;
    }

    this._faces = [];
    this._faceNodes = [];
    for (var i=0; i<this._cubes.length; i++) {
        var faces = this._cubes[i].getFaces();
        for (var p in faces) {
            this._faces.push(faces[p]);
            this._faceNodes.push(faces[p].getNode());
        }
    }

    OZ.Event.prevent(e);
    this._drag.face = this._eventToFace(e);
    e = (e.touches ? e.touches[0] : e);
    this._drag.mouse = [e.clientX, e.clientY];

    this._drag.ec.push(OZ.Event.add(document.body, "mousemove touchmove", this._dragMove.bind(this)));
    this._drag.ec.push(OZ.Event.add(document.body, "mouseup touchend", this._dragEnd.bind(this)));
}

Rubik.prototype._dragMove = function (e) {
	if (e.touches && e.touches.length > 1) { return; }
	if (this._drag.face) { /* check second face for rotation */
		var thisFace = this._eventToFace(e);
		if (!thisFace || thisFace == this._drag.face) { return; }
		this._dragEnd();
		this.rotate(this._drag.face, thisFace);
	} else { /* rotate cube */
		e = (e.touches ? e.touches[0] : e);
		var mouse = [e.clientX, e.clientY];
		var dx = mouse[0] - this._drag.mouse[0];
		var dy = mouse[1] - this._drag.mouse[1];
		var norm = Math.sqrt(dx * dx + dy * dy);
		if (!norm) { return; }
		var N = [-dy / norm, dx / norm];

		this._drag.mouse = mouse;
		this._rotation = Quaternion.fromRotation([N[0], N[1], 0], norm / 2).multiply(this._rotation);
		this._update();
	}
}

Rubik.prototype._dragEnd = function (e) {
	while (this._drag.ec.length) { OZ.Event.remove(this._drag.ec.pop()); }
	if (!this._drag.face && this._help.a) {
		this._help.a.style.opacity = 0;
		this._help.a = null;
	}
}

Rubik.prototype.rotate = function (face1, face2) {
	if (!enbable_manual_move) { return; }
	var t1 = face1.getType();
	var t2 = face2.getType();
	var pos1 = face1.getCube().getPosition();
	var pos2 = face2.getCube().getPosition();

	/* find difference between cubes */
	var diff = 0;
	var diffIndex = -1;
	for (var i = 0; i < 3; i++) {
		var d = pos1[i] - pos2[i];
		if (d) {
			if (diffIndex != -1) { return; } /* different in >1 dimensions */
			diff = (d > 0 ? 1 : -1);
			diffIndex = i;
		}
	}

	if (t1 == t2) { /* same face => diffIndex != -1 */
		switch (t1) {
			case Face.FRONT:
			case Face.BACK:
				var coef = (t1 == Face.FRONT ? 1 : -1);
				if (diffIndex == 0) { this.rotateY(coef * diff, pos1[1]); } else { this.rotateX(coef * diff, pos1[0]); }
				break;

			case Face.LEFT:
			case Face.RIGHT:
				var coef = (t1 == Face.LEFT ? 1 : -1);
				if (diffIndex == 2) { this.rotateY(-coef * diff, pos1[1]); } else { this.rotateZ(coef * diff, pos1[2]); }
				break;

			case Face.TOP:
			case Face.BOTTOM:
				var coef = (t1 == Face.TOP ? 1 : -1);
				if (diffIndex == 0) { this.rotateZ(-coef * diff, pos1[2]); } else { this.rotateX(-coef * diff, pos1[0]); }
				break;
		}
		return;
	}

	switch (t1) { /* different face => same cube, diffIndex == 1 */
		case Face.FRONT:
		case Face.BACK:
			var coef = (t1 == Face.FRONT ? 1 : -1);
			if (t2 == Face.LEFT) { this.rotateY(1 * coef, pos1[1]); }
			if (t2 == Face.RIGHT) { this.rotateY(-1 * coef, pos1[1]); }
			if (t2 == Face.TOP) { this.rotateX(1 * coef, pos1[0]); }
			if (t2 == Face.BOTTOM) { this.rotateX(-1 * coef, pos1[0]); }
			break;

		case Face.LEFT:
		case Face.RIGHT:
			var coef = (t1 == Face.LEFT ? 1 : -1);
			if (t2 == Face.FRONT) { this.rotateY(-1 * coef, pos1[1]); }
			if (t2 == Face.BACK) { this.rotateY(1 * coef, pos1[1]); }
			if (t2 == Face.TOP) { this.rotateZ(1 * coef, pos1[2]); }
			if (t2 == Face.BOTTOM) { this.rotateZ(-1 * coef, pos1[2]); }
			break;

		case Face.TOP:
		case Face.BOTTOM:
			var coef = (t1 == Face.TOP ? 1 : -1);
			if (t2 == Face.FRONT) { this.rotateX(-1 * coef, pos1[0]); }
			if (t2 == Face.BACK) { this.rotateX(1 * coef, pos1[0]); }
			if (t2 == Face.LEFT) { this.rotateZ(-1 * coef, pos1[2]); }
			if (t2 == Face.RIGHT) { this.rotateZ(1 * coef, pos1[2]); }
			break;
	}

}

Rubik.prototype.rotateX = function (dir, layer, send = true) {
	if (layer == 1) { return; }
	var cubes = [];
	for (var i = 0; i < Rubik.SIZE * Rubik.SIZE; i++) {
		cubes.push(this._cubes[layer + i * Rubik.SIZE]);
	}
	this.rotateCubes(cubes, [dir, 0, 0]);
	if (send) {
		if (layer == 0) {
			if (dir == 1) {
				console.log("F'");
				send_moves("F'");
			}
			else {
				console.log("F");
				send_moves("F");
			}
		} else {
			if (dir == 1) {
				console.log("B");
				send_moves("B");
			}
			else {
				console.log("B'");
				send_moves("B'");
			}
		}
	}
}

Rubik.prototype.rotateY = function (dir, layer, send = true) {
	if (layer == 1) { return; }
	var cubes = [];
	for (var i = 0; i < Rubik.SIZE; i++) {
		for (var j = 0; j < Rubik.SIZE; j++) {
			cubes.push(this._cubes[j + layer * Rubik.SIZE + i * Rubik.SIZE * Rubik.SIZE]);
		}
	}
	this.rotateCubes(cubes, [0, -dir, 0]);
	if (send) {
		if (layer == 0) {
			if (dir == 1) {
				console.log("U");
				send_moves("U");
			}
			else {
				console.log("U'");
				send_moves("U'");
			}
		} else {
			if (dir == 1) {
				console.log("D'");
				send_moves("D'");
			}
			else {
				console.log("D");
				send_moves("D");
			}
		}
	}
}

Rubik.prototype.rotateZ = function (dir, layer, send = true) {
	if (layer == 1) { return; }
	var cubes = [];
	var offset = layer * Rubik.SIZE * Rubik.SIZE;
	for (var i = 0; i < Rubik.SIZE * Rubik.SIZE; i++) {
		cubes.push(this._cubes[offset + i]);
	}
	this.rotateCubes(cubes, [0, 0, dir]);
	if (send) {
		if (layer == 0) {
			if (dir == 1) {
				console.log("R");
				send_moves("R");
			}
			else {
				console.log("R'");
				send_moves("R'");
			}
		} else {
			if (dir == 1) {
				console.log("L'");
				send_moves("L'");
			}
			else {
				console.log("L");
				send_moves("L");
			}
		}
	}
}

Rubik.prototype.rotateCubes = function (cubes, rotation) {
	var suffixes = ["X", "Y", ""];
	var prefix = OZ.CSS3.getPrefix("transition");
	if (prefix === null) {
		this._finalizeRotation(cubes, rotation);
	} else {
		var cb = function () {
			OZ.Event.remove(e);
			this._finalizeRotation(cubes, rotation);
		}
		var e = OZ.Event.add(document.body, "webkitTransitionEnd transitionend MSTransitionEnd oTransitionEnd", cb.bind(this));

		var str = "";
		for (var i = 0; i < 3; i++) {
			if (!rotation[i]) { continue; }
			str = "rotate" + suffixes[i] + "(" + (90 * rotation[i]) + "deg)";
		}
		for (var i = 0; i < cubes.length; i++) { cubes[i].setRotation(str); }
	}

}

/**
 * Remap colors
 */
Rubik.prototype._finalizeRotation = function (cubes, rotation) {
	var direction = 0;
	for (var i = 0; i < 3; i++) {
		if (rotation[i]) { direction = rotation[i]; }
	}

	if (rotation[0]) { direction *= -1; } /* FIXME wtf */

	var half = Math.floor(Rubik.SIZE / 2) - (Rubik.SIZE % 2 === 0 ? 1 / 2 : 0);

	for (var i = 0; i < cubes.length; i++) {
		var x = i % Rubik.SIZE - half;
		var y = Math.floor(i / Rubik.SIZE) - half;

		var source = [y * direction + half, -x * direction + half];
		var sourceIndex = source[0] + Rubik.SIZE * source[1];

		cubes[i].prepareColorChange(cubes[sourceIndex], rotation);
	}

	for (var i = 0; i < cubes.length; i++) { cubes[i].commitColorChange(); }

	setTimeout(function () {
		if (this._help.b) {
			this._help.b.style.opacity = 0;
			this._help.b = null;
		}

		this.dispatch("rotated");
		removeCenterLetters();
		addContextMenuToFaces();
	}.bind(this), 100);
}

Rubik.prototype.state2pices = function (state) {
	for (var z = 0; z < Rubik.SIZE; z++) {
		for (var y = 0; y < Rubik.SIZE; y++) {
			for (var x = 0; x < Rubik.SIZE; x++) {
				var cube = new Cube([x, y, z]);
				//console.log(cube)
				this._cubes.push(cube);
			}
		}
	}
	// TOP
	for (var i = 0; i < Rubik.SIZE ** 2; i++) {
		this._cubes[i].setFace(Face.TOP, state[i]);
	}
	// FRONT
	for (var i = 0; i < Rubik.SIZE; i++) {
		for (var j = 0; j < Rubik.SIZE; j++) {
			this._cubes[i].setFace(Face.FRONT, state[i]);
		}
	}
}

function num2State(num) {
	if (num == 0) return "white";
	if (num == 1) return "green";
	if (num == 2) return "red";
	if (num == 3) return "blue";
	if (num == 4) return "orange";
	if (num == 5) return "yellow";
	if (num == -1) return "black";
}

Rubik.prototype._build = function () {
	couter = -1;
	for (let z = 0; z < Rubik.SIZE; z++) {
		for (let y = 0; y < Rubik.SIZE; y++) {
			for (let x = 0; x < Rubik.SIZE; x++) {
				var cube = new Cube([x, y, z]);
				//console.log(cube)
				this._cubes.push(cube);
				if (z != 1 || y != 1 || x != 1) {
					couter += 1;
					for (let i = 0; i < 3; i++) {
						if (i == 0) { cube.setFace(Face.TOP, num2State(this._init_state[couter][i])); cube.setFace(Face.BOTTOM, num2State(this._init_state[couter][i])); }
						else if (i == 1) { cube.setFace(Face.RIGHT, num2State(this._init_state[couter][i])); cube.setFace(Face.LEFT, num2State(this._init_state[couter][i])); }
						else { cube.setFace(Face.FRONT, num2State(this._init_state[couter][i])); cube.setFace(Face.BACK, num2State(this._init_state[couter][i])); }
					}
				}


				this._node.appendChild(cube.getNode());
			}
		}
	}

	addContextMenuToFaces();
}


function addContextMenuToFaces() {
    setTimeout(() => {
        document.querySelectorAll('.face').forEach((face, idx) => {
            face.dataset.faceIdx = idx; // Atribui índice único
            face.addEventListener('dblclick', function(e) {
                e.preventDefault();
                const faceObj = face._faceObj;
                if (faceObj) {
                    const cube = faceObj.getCube();
                    const pos = cube.getPosition(); // [x, y, z]
                    const type = faceObj.getType();
                    /*if (
                        (type === Face.TOP && pos[0] === 1 && pos[1] === 0 && pos[2] === 1) ||
                        (type === Face.BOTTOM && pos[0] === 1 && pos[1] === 2 && pos[2] === 1) ||
                        (type === Face.BACK && pos[0] === 1 && pos[1] === 1 && pos[2] === 2) ||
                        (type === Face.FRONT && pos[0] === 1 && pos[1] === 1 && pos[2] === 0) ||
                        (type === Face.LEFT && pos[0] === 0 && pos[1] === 1 && pos[2] === 1) ||
                        (type === Face.RIGHT && pos[0] === 2 && pos[1] === 1 && pos[2] === 1)
                    ) {
                        return; // Não mostra popup para centros
                    }*/
                }

                // Mostra o popup na posição do mouse
                const popup = document.getElementById("popup");
                popup.style.left = `${e.pageX}px`;
                popup.style.top = `${e.pageY}px`;
                popup.classList.remove("hidden");

                // Guarda o índice da face clicada
                popup.dataset.faceIdx = face.dataset.faceIdx;

                // Adiciona listeners aos botões de cor (só uma vez por botão)
                document.querySelectorAll('.color-btn').forEach(btn => {
                    if (!btn.dataset.listenerAdded) {
                        btn.addEventListener('click', function() {
                            const color = btn.getAttribute('data-color');
                            const faceIdx = popup.dataset.faceIdx;
                            const faceNode = document.querySelector(`.face[data-face-idx='${faceIdx}']`);
                            if (faceNode && faceNode._faceObj) {
                                faceNode._faceObj.setColor(color);
								sendCubeStateUpdate();
								showCenterLetters();
                            } else if (faceNode) {
								faceNode.style.backgroundColor = color;
								sendCubeStateUpdate();
								showCenterLetters();
                            }
                            popup.classList.add("hidden");
                        });
                        btn.dataset.listenerAdded = "true";
                    }
                });
            });
        });
    }, 100);
}

document.addEventListener("click", (e) => {
    if (!popup.contains(e.target) && !e.target.classList.contains("face")) {
        popup.classList.add("hidden");
    }
});

// Função para remover as letras das faces centrais
function removeCenterLetters() {
    const centers = [
        {type: Face.TOP,    pos: [1,0,1]},
        {type: Face.BOTTOM, pos: [1,2,1]},
        {type: Face.FRONT,  pos: [1,1,0]},
        {type: Face.BACK,   pos: [1,1,2]},
        {type: Face.LEFT,   pos: [0,1,1]},
        {type: Face.RIGHT,  pos: [2,1,1]},
    ];
    document.querySelectorAll('.face').forEach(faceNode => {
        const faceObj = faceNode._faceObj;
        if (!faceObj) return;
        const pos = faceObj.getCube().getPosition();
        const type = faceObj.getType();
        for (const center of centers) {
            if (type === center.type && pos[0] === center.pos[0] && pos[1] === center.pos[1] && pos[2] === center.pos[2]) {
                // Remove apenas o <span> da letra, se existir
                const span = faceNode.querySelector('span');
                if (span) span.remove();
            }
        }
    });
}

// Função para mostrar as letras nas faces centrais
function showCenterLetters() {
    const centers = [
        {type: Face.TOP,    pos: [1,0,1], letter: 'U'},
        {type: Face.BOTTOM, pos: [1,2,1], letter: 'D'},
        {type: Face.FRONT,  pos: [1,1,0], letter: 'R'},
        {type: Face.BACK,   pos: [1,1,2], letter: 'L'},
        {type: Face.LEFT,   pos: [0,1,1], letter: 'F'},
        {type: Face.RIGHT,  pos: [2,1,1], letter: 'B'},
    ];
    document.querySelectorAll('.face').forEach(faceNode => {
        const faceObj = faceNode._faceObj;
        if (!faceObj) return;
        const pos = faceObj.getCube().getPosition();
        const type = faceObj.getType();
        for (const center of centers) {
            if (type === center.type && pos[0] === center.pos[0] && pos[1] === center.pos[1] && pos[2] === center.pos[2]) {
                // Só adiciona se não existir já o <span>
                if (!faceNode.querySelector('span')) {
                    let extraStyle = '';
                    if (center.letter === 'U' || center.letter === 'D') extraStyle = 'transform:rotate(90deg);';
                    faceNode.insertAdjacentHTML('beforeend', `<span style="display:flex;align-items:center;justify-content:center;width:100%;height:100%;font-size:2.5em;font-weight:bold;pointer-events:none;color:black;${extraStyle}">${center.letter}</span>`);
                }
            }
        }
    });
}