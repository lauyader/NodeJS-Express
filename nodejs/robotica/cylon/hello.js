var Cylon = require('cylon');

Cylon.robot({
	work: function() {
		every((2).second(), function() {
		console.log("Hello, human!");
		});
	}
}).start();

