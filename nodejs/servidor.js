var http = require('http');
http.createServer(function(request, respuesta){
	respuesta.writeHead(200, {'Content-Type': 'text/plain'} );
	respuesta.write("Bienvenido al servidor Nginx con nodejs");
	respuesta.end();


}).listen(3000,'localhost');

console.log('Servidor web http://localhost/:8000');

