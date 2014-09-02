(function () {
  var color1 = [1.0, 1.0, 0.4];
  var color2 = [0.0, 0.6, 1.0];
  var a = 3.4;
  var b = -5.43;
  var c = -a;
  var d = -b;
  var width;
  var height;
  var iterLength = 1000;
  var lastX = 0;
  var lastY = 0;
  var verts = new Array(iterLength * 2);
  var colors = new Array(iterLength * 4);

  function lerp(a, b, t) {
    return (1 - t) * a + b * t;
  }

  function createContext(canvas) {
    var gl = canvas.getContext("webgl", {preserveDrawingBuffer: true});

    gl.clearColor(0, 0, 0, 1);
    gl.clear(gl.COLOR_BUFFER_BIT);
    //gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.BLEND);
    gl.blendEquation(gl.FUNC_ADD);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE);

    width = canvas.width;
    height = canvas.height;

    return gl;
  }

  function getShader(gl, id) {
    var shaderScript = document.getElementById(id);
    var source = shaderScript.innerHTML;
    var shader;

    if (shaderScript.type == "x-shader/x-fragment") {
      shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (shaderScript.type == "x-shader/x-vertex") {
      shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
      throw new Error("Unknown shader type");
    }

    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      throw new Error("An error occurred compiling the shaders: " +
        gl.getShaderInfoLog(shader));
    }

    return shader;
  }

  function initShaders(gl) {
    var fragmentShader = getShader(gl, "fragment-shader");
    var vertexShader = getShader(gl, "vertex-shader");

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
      throw new Error("Unable to initialize the shader program.");
    }

    gl.useProgram(shaderProgram);
    return shaderProgram;
  }

  function renderBuffers(gl, program) {
    // look up the locations for the inputs to our shaders.
    var u_matLoc = gl.getUniformLocation(program, "u_matrix");
    var colorLoc = gl.getAttribLocation(program, "a_color");
    var vertLoc = gl.getAttribLocation(program, "a_vertex");

    gl.uniformMatrix4fv(u_matLoc, false, [
      2 / width, 0, 0, 0,
      0, 2 / height, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    ]);

    var colorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);

    var vertBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(verts), gl.STATIC_DRAW);

    // Tell the shader how to get data out of the buffers.
    gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
    gl.vertexAttribPointer(colorLoc, 4, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(colorLoc);
    gl.bindBuffer(gl.ARRAY_BUFFER, vertBuffer);
    gl.vertexAttribPointer(vertLoc, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertLoc);


    gl.drawArrays(gl.POINTS, 0, iterLength);
  }

  function exposePixel(gl, x, y, i, distance) {
    var cx = x * width  / 4.0;// + width / 2.0;
    var cy = y * height  / 4.0;// + height / 2.0;

    colors[i*4 + 0] = lerp(color1[0], color2[0], distance);
    colors[i*4 + 1] = lerp(color1[1], color2[1], distance);
    colors[i*4 + 2] = lerp(color1[2], color2[2], distance);
    colors[i*4 + 3] = 1.0 / 256.0;

    verts[i*2 + 0] = cx;
    verts[i*2 + 1] = cy;
  }

  function dejong(gl, i) {
    var x = Math.sin(lastY * a) - Math.cos(lastX * b);
    var y = Math.sin(lastX * c) - Math.cos(lastY * d);
    exposePixel(gl, x, y, i, (Math.abs(x - lastX) + Math.abs(y - lastY)) / 4.0);
    lastX = x;
    lastY = y;
  }


  function start() {
    var gl = createContext(document.getElementById("dejong"));

    var program = initShaders(gl);

    function iterate() {
      for (var i = 0; i < 1024; i ++) {
        dejong(gl, i);
      }

      renderBuffers(gl, program);

      setTimeout(iterate, 0);
    }

    iterate();

  }

  start();

}());
