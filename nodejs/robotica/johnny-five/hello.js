var five = require("johnny-five");
var board = new five.Board({port: "/dev/ttyUSB0"});

board.on("ready", function() {
  var led = new five.Led(13);
  led.blink(2500);
  console.log('Activado el led 13');


  

 
});





