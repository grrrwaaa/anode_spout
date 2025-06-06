
const assert = require("assert"),
	fs = require("fs"),
	path = require("path")

// add anode_gl to the module search paths:
module.paths.push(path.resolve(path.join(__dirname, "..", "anode_gl")))

const gl = require('gles3.js'),
	glutils = require('glutils.js')
const { ok } = glutils;
const glfw = require('glfw3.js')
const Window = require("window.js")

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


let spoutTex = glutils.createTexture(gl, { width: 1024, height: 1024 })
ok(gl, "made spout tex")

const spout = require("./index.js")

ok(gl, "loaded spout")

let receiver = new spout.Receiver()
{
	let senders = receiver.getSenders()
	console.log("senders", senders)
	receiver.setActiveSender(senders[0])
}

let metabuf = new Uint8Array(256);

window.draw = function () {
	let { dim } = this;

	ok(gl, "start")

	//receiver.receiveTexture(GLuint TextureID, GLuint TextureTarget, bool bInvert, GLuint HostFbo)
	let received = receiver.receiveTexture(spoutTex.id, gl.TEXTURE_2D, true)
	ok(gl, "receiveTexture")

	if (received && receiver.isFrameNew()) {
		// receiver.metadata is a raw Uint8array
		//receiver.getMetadata(metabuf)
		//console.log("receive dim", receiver.getSenderWidth(), receiver.getSenderHeight())

		//console.log(receiver.metadata)  
		// if it was supposed to be a string (which e.g. Max sends data as), convert to string like this:
		//console.log("metadata", spout.metadata2string(metabuf))

		if (receiver.isUpdated()) {
			console.log("receive from", receiver.getSenderName())
			let w = receiver.getSenderWidth()
			let h = receiver.getSenderHeight()
			console.log(w, h, spoutTex.width, spoutTex.height)


			console.log("receive from", receiver.getSenderName())
			console.log("receive dim", receiver.getSenderWidth(), receiver.getSenderHeight())
			console.log("receive frame", receiver.getSenderFrame(), "fps", receiver.getSenderFps())
			console.log("receive format", receiver.getSenderFormat())
			ok(gl, "updated")

			if (w != spoutTex.width || h != spoutTex.height) {
				console.log("resizing texture")
				// resize the texture
				spoutTex.dispose()
				spoutTex = glutils.createTexture(gl, { width: receiver.getSenderWidth(), height: receiver.getSenderHeight() })
				receiver.receiveTexture(spoutTex.id, gl.TEXTURE_2D, true)
				ok(gl, "reallocated")
				//console.log(spoutTex)
			}


		}
	}

	glfw.setWindowTitle(this.window, `receiver ${receiver.isConnected()} ${receiver.getSenderName()}, ${received}, ${receiver.getSenderFrame()}`);

	gl.viewport(0, 0, dim[0], dim[1]);
	gl.enable(gl.DEPTH_TEST)
	gl.clearColor(0.5, 0.5, 0.5, 1);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	spoutTex.bind()
	show_shader.begin()
	quad_vao.bind().draw()




	ok(gl, "end")
}

function exitHandler() {
	receiver.release()
	console.log('Goodbye!');
}

function Cleanup(callback) {

	// attach user callback to the process event emitter
	// if no callback, it will still exit gracefully on Ctrl-C
	callback = callback || function noOp() {};
	process.on('cleanup', callback);

	// do app specific cleaning before exiting
	process.on('exit', function () {
		process.emit('cleanup');
	});

	// catch ctrl+c event and exit normally
	process.on('SIGINT', function () {
		console.log('Ctrl-C...');
		process.exit(2);
	});

	//catch uncaught exceptions, trace, then exit normally
	process.on('uncaughtException', function (e) {
		console.log('Uncaught Exception...');
		console.log(e.stack);
		process.exit(99);
	});

	// catches "kill pid" (for example: nodemon restart)
	process.on('SIGUSR1', function () {
		console.log("kill pid (e.g. nodemon restart)")
		process.exit(100);
	})
	process.on('SIGUSR2', function () {
		console.log("kill pid (e.g. nodemon restart)")
		process.exit(100);
	})

	process.on('SIGTERM', () => {
		console.debug('process received SIGTERM')
	})
};

Cleanup(exitHandler)

// // do something when app is closing
// process.on('exit', exitHandler.bind(null,{cleanup:true}));


Window.animate()