const canvas = document.querySelector("canvas");
const ctx = canvas.getContext("2d", { willReadFrequently: true });
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

ctx.fillStyle = "black";
ctx.fillRect(0, 0, canvas.width, canvas.height);

const image_width = Math.round(canvas.width);
const image_height = Math.round(image_width * 9 / 16);
const widesetup = new Image();
widesetup.src = "../tests/wideuse.jpg";
widesetup.onload = () => {
	ctx.drawImage(widesetup, 0, 0, image_width, image_height);
}

function lerp(a, b, t) {
	return a * (1 - t) + b * t;
}
function lerp_vec4(a, b, t) {
	const vec4 = [0, 0, 0, 0];
	
	for (let i = 0; i < 4; i++) {
		vec4[i] = lerp(a[i], b[i], t);
	}
	
	return vec4;
}
function lerp_vec2(a, b, t) {
	const vec2 = [0, 0];
	
	for (let i = 0; i < 2; i++) {
		vec2[i] = lerp(a[i], b[i], t);
	}
	
	return vec2;
}

function sample_bilinear(x, y) {
	const left = Math.floor(x);
	const right = Math.ceil(x);
	const upper = Math.floor(y);
	const lower = Math.ceil(y);
	
	const topleft = ctx.getImageData(left, upper, 1, 1).data;
	const topright = ctx.getImageData(right, upper, 1, 1).data;
	const bottomleft = ctx.getImageData(left, lower, 1, 1).data;
	const bottomright = ctx.getImageData(right, lower, 1, 1).data;
	
	const dx = x - left;
	const dy = y - upper;
	
	const r0 = lerp_vec4(topleft, topright, dx);
	const r1 = lerp_vec4(bottomleft, bottomright, dx);
	
	return lerp_vec4(r0, r1, dy);
}

// document.onmousemove = (e) => { console.log(e.clientX, e.clientY); }

const topleft = [432, 507];
const topright = [642, 500];
const bottomleft = [410, 563];
const bottomright = [673, 547];


function perspect_uv_pixel(u, v) {
	// console.log(u, v);
	const r0 = lerp_vec2(topleft, topright, u);
	const r1 = lerp_vec2(bottomleft, bottomright, u);
	return lerp_vec2(r0, r1, v);
}

function perspect_image() {
	const start_time = performance.now();
	const framebuffer = new Array(image_width * image_height);
	
	for (let y = 0; y < image_height; y++) {
		for (let x = 0; x < image_width; x++) {
			const u = x / (image_width - 1);
			const v = y / (image_height - 1);
			
			const mapped = perspect_uv_pixel(u, v);
			framebuffer[y * image_width + x] = sample_bilinear(...mapped);
		}
		if (y % 5 == 0 || y == image_height - 1) console.log(`PERSPECTING: ${(y / (image_height - 1) * 100).toFixed(2)}%`);
	}
	
	// ctx.fillStyle = "black";
	// ctx.fillRect(0, 0, canvas.width, canvas.height);
	
	for (let y = 0; y < image_height; y++) {
		for (let x = 0; x < image_width; x++) {
			const pix = framebuffer[y * image_width + x];
			let hcc = (cc) => Math.floor(cc).toString(16);
			
			ctx.fillStyle = `#${hcc(pix[0])}${hcc(pix[1])}${hcc(pix[2])}${hcc(pix[3])}`;
			ctx.fillRect(x, y, 1, 1);
		}
		if (y % 50 == 0 || y == image_height - 1) console.log(`DRAWING: ${(y / (image_height - 1) * 100).toFixed(2)}%`);
	}
	
	return (performance.now() - start_time) / 1000;
}