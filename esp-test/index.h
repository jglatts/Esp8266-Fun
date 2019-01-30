
// html main page
const char page[] PROGMEM = R"=====(
<HTML>
  <HEAD>
      <TITLE>AYYYY</TITLE>
  </HEAD>
  <style>
    #test:hover {
      color: brown;
    }
    #color {
	    color: black;
      font-size:3em;
    }
    h1 {
      font-size: 5em;
    }
    button {
      font-size: 2.5em;
    }
    a {
      text-decoration: none;
    }
  </style>
<BODY>
  <CENTER>
      <h1><B>Motor Control!!!</B></h1>
      <p id="color">Motor is Off!</p>
      <button><a href="LEDOn">Motor On</a></button>
      <button><a href="LEDOff">Motor Off</a></button>
  </CENTER> 
</BODY>
</HTML>
)=====";



/* Motor OFF html page */
const char off_page[] PROGMEM = R"=====(
<HTML>
  <HEAD>
      <TITLE>AYYYY</TITLE>
  </HEAD>
  <style>
    #test:hover {
      color: brown;
    }
    #color {
	    color: black;
      font-size:3em;
    }
    h1 {
      font-size: 5em;
    }
    button {
      font-size: 2.5em;
    }
    a {
      text-decoration: none;
    }
  </style>
<BODY>
  <CENTER>
      <h1><B>Motor Control!!!</B></h1>
      <p id="color">Motor is Off!</p>
      <button><a href="LEDOn">Motor On</a></button>
      <button><a href="LEDOff">Motor Off</a></button>
  </CENTER> 
</BODY>
</HTML>
)=====";


/* Motor ON html page */
const char on_page[] PROGMEM = R"=====(
<HTML>
  <HEAD>
      <TITLE>AYYYY</TITLE>
  </HEAD>
  <style>
    #test:hover {
      color: brown;
    }
    #color {
	    color: red;
      font-size:3em;
    }
    h1 {
      font-size: 5em;
    }
    button {
      font-size: 2.5em;
    }
    a {
      text-decoration: none;
    }
  </style>
<BODY>
  <CENTER>
      <h1><B>Motor Control!!!</B></h1>
      <p id="color">Motor is On!</p>
      <button><a href="LEDOn">Motor On</a></button>
      <button><a href="LEDOff">Motor Off</a></button>
  </CENTER> 
</BODY>
</HTML>
)=====";
