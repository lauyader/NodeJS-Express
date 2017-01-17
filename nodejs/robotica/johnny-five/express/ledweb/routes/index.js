'use strict';
var express = require('express');
var five = require('johnny-five');
var router = express.Router();

var temperature = 0;
var time = null;
var reading = false;
var led;


/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});


/*
 *  * Servicio para obtener la temperatura.
 *   */


router.get('/temperature', function (req, res) {
	  res.json({ reading, temperature, time });
});



/*
 *  * Servicio para encender un led.
 *   */
router.get('/led_on', function (req, res) {
	  var result = false;
	  if (led) {
		      result = true;
		      led.on();
		    }
	  res.json({ success: result });
});


/*
 *  * Servicio para apagar un led.
 *   */
router.get('/led_off', function (req, res) {
	  var result = false;
	  if (led) {
		      result = true;
		      led.off();
		    }
	  res.json({ success: result });
});




/*
 *  * Creación de la placa con un sensor de temperatura
 *   * y un led.
 *    */
five.Board({ repl: false }).on('ready', function () {

	  var tmp36 = new five.Temperature({
		      controller: 'TMP36',
		      pin: 'A0',
		      freq: 2000
		    });

	  tmp36.on('data', function () {
		      reading = true;
		      temperature = this.celsius;
		      time = new Date();
		    });

	  led = new five.Led(13);
});


module.exports = router;
