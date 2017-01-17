var five = require("johnny-five");
var keypress = require('keypress');

var board = new five.Board({port: "/dev/ttyUSB0"});
 
// make `process.stdin` begin emitting "keypress" events 
keypress(process.stdin);
 
// listen for the "keypress" event 
process.stdin.on('keypress', function (ch, key) {
  



  console.log('got "keypress"', key);




  if (key && key.ctrl && key.name == 'c') {
    process.stdin.pause();
  }


  if (key.name === "up") {

      	//led.blink(2500);

      	var led = new five.Led(13);
        console.log('Subir');
        led.on();
        console.log('Led encendido');
      }
   if (key.name === "down") {
   		var led = new five.Led(13);
      	//led.blink(2500);
        console.log('bajar');
        led.stop();
        console.log('Led apagado');
      }   

   if (key.name === "left") {

      	//led.blink(2500);
        console.log('Izquierda');
      }
      if (key.name === "right") {

      	//led.blink(2500);
        console.log('Derecha');
      }         

});
 
process.stdin.setRawMode(true);
process.stdin.resume();
