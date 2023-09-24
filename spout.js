const spout = require('bindings')('spout.node');

spout.metadata2string = function(metadata) {
	let metastring = Buffer.from(metadata.buffer).toString()
	let c = metastring.indexOf('\0')
	return (c >= 0) ? metastring.substring(0,c) : metastring;
}

module.exports = spout