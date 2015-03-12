/* Javascript */

var column_count = 320;
var row_count = 240;

function canvas() {
	var i;

	//this.pixels = pixels;
}

/**
   @param color the color (an #Image).
 */
canvas.prototype.draw_line = function(x1, y1, x2, y2, color) {
	// this.

	var xr = x2 - x1;
	var yr = y2 - y1;

	if (xr == 0 && yr == 0) {
		return;
	}

	var d = 0.0;
	// ..1.0 step 1/Math.max(xr, yr)
	step = 1.0 / Math.max(xr, yr);

	while (d </*=*/ 1.0) {
		var x = x1 + d * xr;
		var y = y1 + d * yr;

		if (y >= 0 && y < row_count && x >= 0 && x < column_count) {
			this.pixels[y][x] = color;
		}

		d = d + step;
	}
}

canvas.prototype.load_colors = function() {
	var image = new Image();
	image.src = "black.png";
	this.prototype.colors = Array();
	this.prototype.colors["black"] = image;
	return this.prototype.colors;
}

canvas.prototype.mouse_down = function(e) {
}

canvas.prototype.mouse_up = function(e) {
}

canvas.prototype.mouse_move = function(e) {
}

function find_element(e) {
	var element;
	if (!e) {
		var e = window.event;
	}

	if (e.target) {
		element = e.target;
	} else if (e.srcElement) {
		element = e.srcElement;
	}
	if (element.nodeType == 3) { // workaround Safari bug.
		element = element.parentNode;
	}

	return element;

	while (element != null && element.tagName != "table") {
	}
}

/* document.write */

/*	var replace = document.createElement('img');*/

var the_canvas = canvas();

