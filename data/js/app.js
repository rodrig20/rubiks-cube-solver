
var rubik;
var enbable_manual_move = true;
var OZ = {
	$: function (x) { return typeof (x) == "string" ? document.getElementById(x) : x; },
	select: function (x) { return document.querySelectorAll(x); },
	opera: !!window.opera,
	ie: !!document.attachEvent && !window.opera,
	gecko: !!document.getAnonymousElementByAttribute,
	webkit: !!navigator.userAgent.match(/webkit/i),
	khtml: !!navigator.userAgent.match(/khtml/i) || !!navigator.userAgent.match(/konqueror/i),
	Event: {
		_id: 0,
		_byName: {},
		_byID: {},
		add: function (elm, event, cb) {
			var id = OZ.Event._id++;
			var element = OZ.$(elm);
			var fnc = (element && element.attachEvent ? function () { return cb.apply(element, arguments); } : cb);
			var rec = [element, event, fnc];
			var parts = event.split(" ");
			while (parts.length) {
				var e = parts.pop();
				if (element) {
					if (element.addEventListener) {
						element.addEventListener(e, fnc, false);
					} else if (element.attachEvent) {
						element.attachEvent("on" + e, fnc);
					}
				}
				if (!(e in OZ.Event._byName)) { OZ.Event._byName[e] = {}; }
				OZ.Event._byName[e][id] = rec;
			}
			OZ.Event._byID[id] = rec;
			return id;
		},
		remove: function (id) {
			var rec = OZ.Event._byID[id];
			if (!rec) { return; }
			var elm = rec[0];
			var parts = rec[1].split(" ");
			while (parts.length) {
				var e = parts.pop();
				if (elm) {
					if (elm.removeEventListener) {
						elm.removeEventListener(e, rec[2], false);
					} else if (elm.detachEvent) {
						elm.detachEvent("on" + e, rec[2]);
					}
				}
				delete OZ.Event._byName[e][id];
			}
			delete OZ.Event._byID[id];
		},
		stop: function (e) { e.stopPropagation ? e.stopPropagation() : e.cancelBubble = true; },
		prevent: function (e) { e.preventDefault ? e.preventDefault() : e.returnValue = false; },
		target: function (e) { return e.target || e.srcElement; }
	},
	Class: function () {
		var c = function () {
			var init = arguments.callee.prototype.init;
			if (init) { init.apply(this, arguments); }
		};
		c.implement = function (parent) {
			for (var p in parent.prototype) { this.prototype[p] = parent.prototype[p]; }
			return this;
		};
		c.extend = function (parent) {
			var tmp = function () { };
			tmp.prototype = parent.prototype;
			this.prototype = new tmp();
			this.prototype.constructor = this;
			return this;
		};
		c.prototype.bind = function (fnc) { return fnc.bind(this); };
		c.prototype.dispatch = function (type, data) {
			var obj = {
				type: type,
				target: this,
				timeStamp: (new Date()).getTime(),
				data: data
			}
			var tocall = [];
			var list = OZ.Event._byName[type];
			for (var id in list) {
				var item = list[id];
				if (!item[0] || item[0] == this) { tocall.push(item[2]); }
			}
			var len = tocall.length;
			for (var i = 0; i < len; i++) { tocall[i](obj); }
		}
		return c;
	},
	DOM: {
		elm: function (name, opts) {
			var elm = document.createElement(name);
			for (var p in opts) {
				var val = opts[p];
				if (p == "class") { p = "className"; }
				if (p in elm) { elm[p] = val; }
			}
			OZ.Style.set(elm, opts);
			return elm;
		},
		text: function (str) { return document.createTextNode(str); },
		clear: function (node) { while (node.firstChild) { node.removeChild(node.firstChild); } },
		pos: function (elm) { /* relative to _viewport_ */
			var cur = OZ.$(elm);
			var html = cur.ownerDocument.documentElement;
			var parent = cur.parentNode;
			var x = y = 0;
			if (cur == html) { return [x, y]; }
			while (1) {
				if (OZ.Style.get(cur, "position") == "fixed") {
					x += cur.offsetLeft;
					y += cur.offsetTop;
					return [x, y];
				}

				if (OZ.opera && (parent == html || OZ.Style.get(cur, "display") != "block")) { } else {
					x -= parent.scrollLeft;
					y -= parent.scrollTop;
				}
				if (parent == cur.offsetParent || cur.parentNode == html) {
					x += cur.offsetLeft;
					y += cur.offsetTop;
					cur = parent;
				}

				if (parent == html) { return [x, y]; }
				parent = parent.parentNode;
			}
		},
		scroll: function () {
			var x = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
			var y = document.documentElement.scrollTop || document.body.scrollTop || 0;
			return [x, y];
		},
		win: function (avail) {
			return (avail ? [window.innerWidth, window.innerHeight] : [document.documentElement.clientWidth, document.documentElement.clientHeight]);
		},
		hasClass: function (node, className) {
			var cn = OZ.$(node).className;
			var arr = (cn ? cn.split(" ") : []);
			return (arr.indexOf(className) != -1);
		},
		addClass: function (node, className) {
			if (OZ.DOM.hasClass(node, className)) { return; }
			var cn = OZ.$(node).className;
			var arr = (cn ? cn.split(" ") : []);
			arr.push(className);
			OZ.$(node).className = arr.join(" ");
		},
		removeClass: function (node, className) {
			if (!OZ.DOM.hasClass(node, className)) { return; }
			var cn = OZ.$(node).className;
			var arr = (cn ? cn.split(" ") : []);
			var arr = arr.filter(function ($) { return $ != className; });
			OZ.$(node).className = arr.join(" ");
		},
		append: function () {
			if (arguments.length == 1) {
				var arr = arguments[0];
				var root = OZ.$(arr[0]);
				for (var i = 1; i < arr.length; i++) { root.appendChild(OZ.$(arr[i])); }
			} else for (var i = 0; i < arguments.length; i++) { OZ.DOM.append(arguments[i]); }
		}
	},
	Style: {
		get: function (elm, prop) {
			if (document.defaultView && document.defaultView.getComputedStyle) {
				try {
					var cs = elm.ownerDocument.defaultView.getComputedStyle(elm, "");
				} catch (e) {
					return false;
				}
				if (!cs) { return false; }
				return cs[prop];
			} else {
				return elm.currentStyle[prop];
			}
		},
		set: function (elm, obj) {
			for (var p in obj) {
				var val = obj[p];
				if (p == "opacity" && OZ.ie) {
					p = "filter";
					val = "alpha(opacity=" + Math.round(100 * val) + ")";
					elm.style.zoom = 1;
				} else if (p == "float") {
					p = (OZ.ie ? "styleFloat" : "cssFloat");
				}
				if (p in elm.style) { elm.style[p] = val; }
			}
		}
	},
	Request: function (url, callback, options) {
		var o = { data: false, method: "get", headers: {}, xml: false }
		for (var p in options) { o[p] = options[p]; }
		o.method = o.method.toUpperCase();

		var xhr = false;
		if (window.XMLHttpRequest) { xhr = new XMLHttpRequest(); }
		else if (window.ActiveXObject) { xhr = new ActiveXObject("Microsoft.XMLHTTP"); }
		else { return false; }
		xhr.open(o.method, url, true);
		xhr.onreadystatechange = function () {
			if (xhr.readyState != 4) { return; }
			if (!callback) { return; }
			var data = (o.xml ? xhr.responseXML : xhr.responseText);
			var headers = {};
			var h = xhr.getAllResponseHeaders();
			if (h) {
				h = h.split(/[\r\n]/);
				for (var i = 0; i < h.length; i++) if (h[i]) {
					var v = h[i].match(/^([^:]+): *(.*)$/);
					headers[v[1]] = v[2];
				}
			}
			callback(data, xhr.status, headers);
		};
		if (o.method == "POST") { xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded"); }
		for (var p in o.headers) { xhr.setRequestHeader(p, o.headers[p]); }
		xhr.send(o.data || null);
		return xhr;
	}
}

if (!Function.prototype.bind) {
	Function.prototype.bind = function (thisObj) {
		var fn = this;
		var args = Array.prototype.slice.call(arguments, 1);
		return function () {
			return fn.apply(thisObj, args.concat(Array.prototype.slice.call(arguments)));
		}
	}
};

if (!Array.prototype.indexOf) {
	Array.prototype.indexOf = function (item, from) {
		var len = this.length;
		var i = from || 0;
		if (i < 0) { i += len; }
		for (; i < len; i++) {
			if (i in this && this[i] === item) { return i; }
		}
		return -1;
	}
}
if (!Array.indexOf) {
	Array.indexOf = function (obj, item, from) { return Array.prototype.indexOf.call(obj, item, from); }
}

if (!Array.prototype.lastIndexOf) {
	Array.prototype.lastIndexOf = function (item, from) {
		var len = this.length;
		var i = from || len - 1;
		if (i < 0) { i += len; }
		for (; i > -1; i--) {
			if (i in this && this[i] === item) { return i; }
		}
		return -1;
	}
}
if (!Array.lastIndexOf) {
	Array.lastIndexOf = function (obj, item, from) { return Array.prototype.lastIndexOf.call(obj, item, from); }
}

if (!Array.prototype.forEach) {
	Array.prototype.forEach = function (cb, _this) {
		var len = this.length;
		for (var i = 0; i < len; i++) {
			if (i in this) { cb.call(_this, this[i], i, this); }
		}
	}
}
if (!Array.forEach) {
	Array.forEach = function (obj, cb, _this) { Array.prototype.forEach.call(obj, cb, _this); }
}

if (!Array.prototype.every) {
	Array.prototype.every = function (cb, _this) {
		var len = this.length;
		for (var i = 0; i < len; i++) {
			if (i in this && !cb.call(_this, this[i], i, this)) { return false; }
		}
		return true;
	}
}
if (!Array.every) {
	Array.every = function (obj, cb, _this) { return Array.prototype.every.call(obj, cb, _this); }
}

if (!Array.prototype.some) {
	Array.prototype.some = function (cb, _this) {
		var len = this.length;
		for (var i = 0; i < len; i++) {
			if (i in this && cb.call(_this, this[i], i, this)) { return true; }
		}
		return false;
	}
}
if (!Array.some) {
	Array.some = function (obj, cb, _this) { return Array.prototype.some.call(obj, cb, _this); }
}

if (!Array.prototype.map) {
	Array.prototype.map = function (cb, _this) {
		var len = this.length;
		var res = new Array(len);
		for (var i = 0; i < len; i++) {
			if (i in this) { res[i] = cb.call(_this, this[i], i, this); }
		}
		return res;
	}
}
if (!Array.map) {
	Array.map = function (obj, cb, _this) { return Array.prototype.map.call(obj, cb, _this); }
}

if (!Array.prototype.filter) {
	Array.prototype.filter = function (cb, _this) {
		var len = this.length;
		var res = [];
		for (var i = 0; i < len; i++) {
			if (i in this) {
				var val = this[i];
				if (cb.call(_this, val, i, this)) { res.push(val); }
			}
		}
		return res;
	}
}
if (!Array.filter) {
	Array.filter = function (obj, cb, _this) { return Array.prototype.filter.call(obj, cb, _this); }
}





OZ.CSS3 = {
	getProperty: function (property) {
		var prefix = this.getPrefix(this._normalize(property));
		if (prefix === null) { return null; }
		return (prefix ? "-" + prefix.toLowerCase() + "-" : "") + property;
	},
	set: function (node, prop, value) {
		prop = this._normalize(prop);
		var prefix = this.getPrefix(prop);
		if (prefix === null) { return false; }
		var p = (prefix ? prefix + prop.charAt(0).toUpperCase() + prop.substring(1) : prop);
		node.style[p] = value;
		return true;
	},
	getPrefix: function (property) {
		var prefixes = ["", "ms", "Webkit", "O", "Moz"];
		for (var i = 0; i < prefixes.length; i++) {
			var p = prefixes[i];
			var prop = (p ? p + property.charAt(0).toUpperCase() + property.substring(1) : property);
			if (prop in this._node.style) { return p; }
		}
		return null;
	},
	_normalize: function (property) {
		return property.replace(/-([a-z])/g, function (match, letter) { return letter.toUpperCase(); });
	},
	_node: OZ.DOM.elm("div")
}


var Quaternion = OZ.Class();

Quaternion.fromRotation = function (axis, angle) {
	var DEG2RAD = Math.PI / 180;
	var a = angle * DEG2RAD;

	var sin = Math.sin(a / 2);
	var cos = Math.cos(a / 2);

	return new this(
		axis[0] * sin, axis[1] * sin, axis[2] * sin,
		cos
	);
}

Quaternion.fromUnit = function () {
	return new this(0, 0, 0, 1);
}

Quaternion.prototype.init = function (x, y, z, w) {
	this.x = x;
	this.y = y;
	this.z = z;
	this.w = w;
}

Quaternion.prototype.normalize = function () {
	var norm = Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
	return new this.constructor(this.x / norm, this.y / norm, this.z / norm, this.w / norm);
}

Quaternion.prototype.conjugate = function () {
	return new this.constructor(-this.x, -this.y, -this.z, this.w);
}

Quaternion.prototype.toString = function () {
	return [this.x, this.y, this.z, this.w].toString(", ");
}

Quaternion.prototype.multiply = function (q) {
	var p = this;

	var x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	var y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
	var z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
	var w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;

	return new this.constructor(x, y, z, w);
}

Quaternion.prototype.toAxis = function () {
	return [this.x, this.y, this.z];
}

Quaternion.prototype.toAngle = function () {
	var RAD2DEG = 180 / Math.PI;
	return RAD2DEG * 2 * Math.acos(this.w);
}

Quaternion.prototype.toRotation = function () {
	var axis = this.toAxis();
	var angle = this.toAngle();
	return "rotate3d(" + axis[0].toFixed(10) + "," + axis[1].toFixed(10) + "," + axis[2].toFixed(10) + "," + angle.toFixed(10) + "deg)";
}

Quaternion.prototype.toRotations = function () {
	var RAD2DEG = 180 / Math.PI;

	var x = RAD2DEG * Math.atan2(2 * (this.w * this.x + this.y * this.z), 1 - 2 * (this.x * this.x + this.y * this.y));
	var y = RAD2DEG * Math.asin(2 * (this.w * this.y - this.x * this.z));
	var z = RAD2DEG * Math.atan2(2 * (this.w * this.z + this.x * this.y), 1 - 2 * (this.y * this.y + this.z * this.z));

	if (x < 0) { x += 360; }
	if (y < 0) { y += 360; }
	if (z < 0) { z += 360; }

	return "rotateX(" + x.toFixed(10) + "deg) rotateY(" + y.toFixed(10) + "deg) rotate(" + z.toFixed(10) + "deg)";
}




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

Cube.prototype._update = function () {
	var transform = "";
	transform += "translate3d(" + (-Face.SIZE / 2) + "px, " + (-Face.SIZE / 2) + "px, " + (-Face.SIZE / 2) + "px) ";
	if (this._rotation) { transform += this._rotation + " "; }

	var half = Math.floor(Rubik.SIZE / 2) - (Rubik.SIZE % 2 === 0 ? 1 / 2 : 0);
	var x = this._position[0];
	var y = this._position[1];
	var z = -this._position[2];
	x -= half;
	y -= half;
	z += half + 1 / 2;
	transform += "translate3d(" + (x * Face.SIZE) + "px, " + (y * Face.SIZE) + "px, " + (z * Face.SIZE) + "px)";

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

	this._rotation = Quaternion.fromRotation([1, 0, 0], -35).multiply(Quaternion.fromRotation([0, 1, 0], 45));
	this._node = OZ.DOM.elm("div", { position: "absolute", left: "50%", top: "40%", width: "0px", height: "0px" });
	document.body.appendChild(this._node);

	OZ.CSS3.set(document.body, "perspective", "460px");
	OZ.CSS3.set(this._node, "transform-style", "preserve-3d");

	this._build();
	this._update();
	OZ.Event.add(document.body, "mousedown touchstart", this._dragStart.bind(this));

	//setTimeout(this.randomize.bind(this), 500);
}

/*
Rubik.prototype.randomize = function () {
	var remain = 10;
	var cb = function () {
		remain--;
		if (remain > 0) {
			this.rotateRandom();
		} else {
			OZ.Event.remove(e);

			this._help.a = OZ.DOM.elm("p", { innerHTML: "Drag or swipe the background to rotate the whole cube." });
			this._help.b = OZ.DOM.elm("p", { innerHTML: "Drag or swipe the cube to rotate its layers." });
			document.body.appendChild(this._help.a);
			document.body.appendChild(this._help.b);
			OZ.CSS3.set(this._help.a, "transition", "opacity 500ms");
			OZ.CSS3.set(this._help.b, "transition", "opacity 500ms");

		}
	}
	var e = OZ.Event.add(null, "rotated", cb.bind(this));
	this.rotateRandom();
}

Rubik.prototype.rotateRandom = function () {
	var method = "rotate" + ["X", "Y", "Z"].random();
	var dir = [-1, 1].random();
	var layer = Math.floor(Math.random() * Rubik.SIZE);
	this[method](dir, layer);
}
*/

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
    // Verificar se o clique foi no input ou em seus elementos pai
    let target = e.target;
    while (target) {
        if (target.id === 'move-sequence' || target.id === 'button-container') {
            return; // Não inicia o drag se clicar no input ou no container
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
}

async function make_moves(moves_seq) {
	const moves = moves_seq.split(/\s+/); // Divide a string pelos espaços

	for (const move of moves) {
		console.log(move)
		if (move == "U2") {
			rubik.rotateY(1, 0, false); await sleep(375); rubik.rotateY(1, 0, false);
		}
		else if (move == "U'")
			rubik.rotateY(-1, 0, false);
		else if (move == "U")
			rubik.rotateY(1, 0, false);
		else if (move == "F2") {
			rubik.rotateX(1, 0, false); await sleep(375); rubik.rotateX(1, 0, false)
		}
		else if (move == "F'")
			rubik.rotateX(1, 0, false)
		else if (move == "F")
			rubik.rotateX(-1, 0, false)
		else if (move == "R2") {
			rubik.rotateZ(1, 0, false); await sleep(375); rubik.rotateZ(1, 0, false)
		}
		else if (move == "R'")
			rubik.rotateZ(-1, 0, false)
		else if (move == "R")
			rubik.rotateZ(1, 0, false)
		else if (move == "B2") {
			rubik.rotateX(1, 2, false); await sleep(375); rubik.rotateX(1, 2, false);
		}
		else if (move == "B'")
			rubik.rotateX(-1, 2, false)
		else if (move == "B")
			rubik.rotateX(1, 2, false)
		else if (move == "L2") {
			rubik.rotateZ(-1, 2, false); await sleep(375); rubik.rotateZ(-1, 2, false)
		}
		else if (move == "L'")
			rubik.rotateZ(1, 2, false)
		else if (move == "L")
			rubik.rotateZ(-1, 2, false)
		else if (move == "D2") {
			rubik.rotateY(-1, 2, false); await sleep(375); rubik.rotateY(-1, 2, false);
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

			make_moves(data).then(() => {
				enbable_manual_move = true;
			});

		} catch (error) {
			console.log("Sem Solução")
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