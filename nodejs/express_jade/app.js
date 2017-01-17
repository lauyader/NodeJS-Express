var express = require('express');
var app = express();
var fs = require('fs');

app.use(express.static(__dirname + '/public'));
app.get('/', function(peticion, respuesta) {
	// body...
	//respuesta.send('Bienvenido a  Express!');
	respuesta.render('index.jade',{
		titulo:"Pagina con JADE",
		author:"Luis Americo Auyadermont",
		descripcion:"Espacio para el aprendizaje de jade nodejs y nginx"


	});

});

app.listen(3000, function(){
	console.log('Escuchando desde el puerto 3000');
});
