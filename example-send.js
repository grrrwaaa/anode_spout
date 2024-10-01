
const assert = require("assert"),
	fs = require("fs"),
    path = require("path")

const spout = require("./index.js")

// add anode_gl to the module search paths:
module.paths.push(path.resolve(path.join(__dirname, "..", "anode_gl")))

const gl = require('gles3.js'),
	glfw = require('glfw3.js'),
    Window = require("window.js"),
	glutils = require('glutils.js')

let window = new Window({
	width: 720, height: 720
})

const quad_vao = glutils.createVao(gl, glutils.makeQuad())

let fbo = glutils.makeFboWithDepth(gl)

let test_shader = glutils.makeProgram(gl,
`#version 330
layout(location = 0) in vec3 a_position;
//layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
out vec2 v_uv;

void main() {
	gl_Position = vec4(a_position.xy, 0, 1);
	v_uv = a_texCoord;
}`,
`#version 330
precision mediump float;
uniform float t;

in vec2 v_uv;
layout(location = 0) out vec4 frag_out0;

void main() {
	vec3 input = mod(t * vec3(0.1, 0.2, 0.3) + v_uv.xyy, 1.);
	frag_out0 = vec4(input, 1);
	//frag_out0 = vec4(v_uv, 0., 1.);
}
`);

let show_shader = glutils.makeProgram(gl,
`#version 330
layout(location = 0) in vec3 a_position;
//layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
out vec2 v_uv;

void main() {
	gl_Position = vec4(a_position.xy, 0, 1);
	v_uv = a_texCoord;
}`,
`#version 330
precision mediump float;
uniform sampler2D u_tex0;

in vec2 v_uv;
layout(location = 0) out vec4 frag_out0;

void main() {
    frag_out0 = texture(u_tex0, v_uv);
    //frag_out0 = vec4(v_uv, 0, 1);
}
`);

let sender = new spout.Sender("nodejs")

let metadata = new Uint8Array(8)

window.draw = function() {
	let { t, dim } = this;

	// generate a texture to send:
	fbo.begin()
	gl.viewport(0, 0, fbo.width, fbo.height);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	test_shader.begin().uniform("t", t)
	quad_vao.bind().draw()
	// set metadata:
	metadata[0] = t
	sender.setMetadata(metadata)
	// you can either send from within the fbo render like this...
	sender.sendFbo(fbo.id, fbo.width, fbo.height, true)
	fbo.end()

	gl.viewport(0, 0, dim[0], dim[1]);
	gl.enable(gl.DEPTH_TEST)
	gl.clearColor(0.5, 0.5, 0.5, 1);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	fbo.bind()
	// ... or you can send a texture like this:
	//sender.sendTexture(fbo.colorTexture, gl.TEXTURE_2D, fbo.width, fbo.height, true)
	show_shader.begin()
	quad_vao.bind().draw()
}

Window.animate()