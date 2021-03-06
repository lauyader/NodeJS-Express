"use strict";

var Cylon = require("cylon");

Cylon.robot({
  connections: {
    arduino: { adaptor: "firmata", port: "/dev/ttyUSB0" }
    
  },

  devices: {
    led: { driver: "led", pin: 13 }
  },

  work: function(my) {
    every((1).second(), my.led.toggle);
    console.log('Paso por el led');
  }
}).start();