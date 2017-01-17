var express = require('express');
var app = express();

app.get('/', function(peticion, respuesta) {
	// body... 
	respuesta.send('Bienvenido a  Express!');

})

app.listen(3000, function(){
	console.log('Escuchando desde el puerto 3000');
})