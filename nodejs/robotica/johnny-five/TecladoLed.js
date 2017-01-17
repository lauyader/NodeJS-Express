// Se activa el led 13 mediante la pulsacion de las teclas de direccion.


var five = require("johnny-five");
var keypress = require('keypress');

var board = new five.Board({port: "/dev/ttyUSB0"});
 
// make `process.stdin` begin emitting "keypress" events 
keypress(process.stdin); 
 
// listen for the "keypress" event 
process.stdin.on('keypress', function (ch, key) {
  
  console.log('got "keypress"', key);

   // si pulsa pulsa las teclas ctr+c el ciclo realiza una pausa.
  if (key && key.ctrl && key.name == 'c') {
    process.stdin.pause();
  }


  if (key.name === "up") {

      	//led.blink(2500);

      	var led = new five.Led(13);
        console.log('Subir');
        // Enciende el led
        led.on();
        console.log('Led encendido');
      }
   if (key.name === "down") {
   		var led = new five.Led(13);
      	//led.blink(2500);
        console.log('bajar');
        //Apaga el led 13
        led.off();
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
