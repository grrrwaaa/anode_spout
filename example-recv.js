
const assert = require("assert"),
	fs = require("fs"),
    path = require("path")

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

let spoutTex = glutils.createPixelTexture(gl, 1024, 1024)

const spout = require("./spout.js")

let receiver = new spout.Receiver()
let senders = receiver.getSenders()
console.log("senders", senders)
receiver.setActiveSender(senders[0])

window.draw = function() {
	let { dim } = this;

	//receiver.receiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
	let received = receiver.receiveTexture(spoutTex.id, gl.TEXTURE_2D, true)

	// receiver.metadata is a raw Uint8array
	console.log(receiver.metadata)  
	// if it was supposed to be a string (which e.g. Max sends data as), convert to string like this:
	//console.log(spout.metadata2string(receiver.metadata))

    if (receiver.isUpdated()) {
        console.log("receive from", receiver.getSenderName())
        console.log("receive dim", receiver.getSenderWidth(), receiver.getSenderHeight())
        console.log("receive frame", receiver.getSenderFrame(), "fps", receiver.getSenderFps())
        console.log("receive format", receiver.getSenderFormat())

		// resize the texture
		spoutTex.dispose()
		spoutTex = glutils.createPixelTexture(gl, receiver.getSenderWidth(), receiver.getSenderHeight())
		//console.log(spoutTex)
    }

	glfw.setWindowTitle(this.window, `receiver ${receiver.isConnected()} ${receiver.getSenderName()}, ${received}, ${receiver.getSenderFrame()}`);

	gl.viewport(0, 0, dim[0], dim[1]);
	gl.enable(gl.DEPTH_TEST)
	gl.clearColor(0.5, 0.5, 0.5, 1);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	spoutTex.bind()
	show_shader.begin()
	quad_vao.bind().draw()
}

Window.animate()