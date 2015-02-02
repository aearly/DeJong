var express = require("express");
var exec = require("child_process").exec;
var fs = require("fs");
var dejongOut = __dirname + "/out.png";

var app = express();

app.use(require("express-static")(__dirname + "/../public"));

app.use(function (req, res) {
  var a = req.query.a;
  var b = req.query.b;

  deJong(a, b, function () {
    var stream = fs.createReadStream(dejongOut);

    res.set("Content-Type", "image/png");
    console.log("image generated " + a + " " + b);
    stream.pipe(res);
  });

});

app.listen(3434);


function deJong(a, b, done) {
  var dejongApp = __dirname + "/../dejong -w 512 -i 1 -s 2 -o out.png";
  a = Math.abs(+a);
  b = Math.abs(+b);

  exec(dejongApp + " -a " + a + " -b " + b, done);
}
