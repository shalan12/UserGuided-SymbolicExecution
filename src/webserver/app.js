
/**
 * Module dependencies.
 */

var express = require('express'),
  routes = require('./routes'),
  user = require('./routes/user'),
  http = require('http'),
  path = require('path');

var app = express();
var net = require('net');

var HOST = '127.0.0.1';
var PORT = 30000;
var timeout = 1000;

var sys = require('sys')
var exec = require('child_process').exec,
    child;

var fs = require('fs');
var util = require('util');
// var log_file = fs.createWriteStream(__dirname + '/debug.log', {flags : 'w'});
// var log_stdout = process.stdout;

// // console.log = function(d) { //
// //   log_file.write(util.format(d) + '\n');
// //   log_stdout.write(util.format(d) + '\n');
// // };
// // Create a server instance, and chain the listen function to it
// // The function passed to net.createServer() becomes the event handler for the 'connection' event
// // The sock object the callback function receives UNIQUE for each connection

// //  exec('../../symbolicexecutor.o ' + PORT.toString(), function(error, stdout, stderr) {
// //    if (error !== null) {
// //       console.log('exec error: ' + error);
// //     }
// //   console.log(stdout);
// // });


var map = []; // contains sessionid->filename,lastPinged
var toSend = [];
function getRandomInt()
{
  var randomNumber;
  var found;
 do 
 {
    found = true;
    randomNumber = Math.random();
    randomNumber = randomNumber.toString().substring(2,randomNumber.length);

    for (var key in map) 
    {
      if (map.hasOwnProperty(key) &&  randomNumber == key) 
      {
        found = false
        break
      }
    }

  }while (!found);
  return randomNumber;
}
client = net.createConnection(PORT)
client.on('connect',function()
{
  console.log("Connected to symbolicexecutor\n")
});
client.on('data',function(data)
{
      console.log("recieved from symbolicexecutor " + data);
      data = JSON.parse(data);
      toSendToUser = {};
      toSendToUser["node"] = data["node"];
      toSendToUser["parent"] = data["parent"];
      toSendToUser["text"] = data["text"];
      toSendToUser["fin"] = data["fin"];
      toSendToUser["updated"] = true;
      toSendToUser["constraints"] = data["constraints"];
      toSend[data.fileId] = toSendToUser;
      toSend["startLine"] = data["startLine"];
      toSend["endLine"] = data["endLine"];
      console.log("sending to user : " + JSON.stringify(toSendToUser));  
      if(data.fin  === "1")
      {
        var idx = map.indexOf(data.sessionid);
        if(idx != -1) map.splice(idx,1);
        client.end("FIN");
        //delete file
      }
     
});
// client.setTimeout(timeout,function()
// {

// });

app.configure(function(){
  app.set('port', process.env.PORT || 3000);
  app.set('views', __dirname + '/views');
  app.set('view engine', 'ejs');
  app.engine('.html', require('ejs').renderFile);
  app.use(express.favicon());
  app.use(express.logger('dev'));
  app.use(express.cookieParser());
  app.use(express.bodyParser());
  app.use(express.methodOverride());
  app.use(app.router);
  app.use(function (req, res, next) {
  // check if client sent cookie
    var cookie = req.cookies.sessionid;
    if (cookie === undefined)
    {
      // no: set a new cookie
      res.cookie('sessionid',getRandomInt(), { maxAge: 900000, httpOnly: true });
      console.log('cookie created successfully');
    } 
    else
    {
      // yes, cookie was already present 
      console.log('cookie exists', cookie);
    } 
    next(); // <-- important!
  });
  
  app.use(require('stylus').middleware(__dirname + '/public'));
  app.use(express.static(path.join(__dirname, 'public')));
});

app.configure('development', function(){
  app.use(express.errorHandler());
});

app.get('/', function(req, res){
  res.render('index', {
    title: 'Home'
  });
});

app.get('/about', function(req, res){
  res.render('about', {
    title: 'About'
  });
});

app.get('/contact', function(req, res){
  res.render('contact', {
    title: 'Contact'
  });
});
app.post('/upload',function(req,res){

  console.log(req)
  var filename = req.files.SelectedFile.name; //fileToUpload is the name of the inputfield
  var base = filename.substring(0,filename.length - 4); // remove extension
  var extension = '.cpp'; 
  //filename = base + "_" + req.cookies.sessionid + extension; 
  filename = req.cookies.sessionid;
  fs.readFile(req.files.SelectedFile.path, function (err, data) 
  {
      var newPath = __dirname + "/uploads/" + filename; //__dirname is a global, containing the current dir
      fs.writeFile(newPath+extension, data,function(err)
      {
            bcFile = newPath+".bc";
            toExec = "clang-3.5 -emit-llvm " + newPath  + ".cpp -g -c -o " + bcFile;
            exec(toExec, function (error, stdout, stderr) {
              map[req.cookies.sessionid] = bcFile; // store mapping between sessionid and filename
              // things from this map will need to be deleted later .. when client leaves .. or when execution is completed
              toSendToExecutor["isBFS"] = 1;
              toSendToExecutor["branch"] = 1;
              toSendToExecutor["steps"] = 1;
              toSendToExecutor["prevId"] = -1;
              toSendToExecutor["id"] = bcFile;
              client.write(toSendToExecutor.stringify()); // pass the filename to symbolic executor
            });
      });
       
  }); 
  res.redirect('back'); // return to the previous page
});

app.get('/next',function(req,res){
  toSendToExecutor = {};
  var parts = url.parse(request.url, true);
  var query = parts.query;
  var fileId = map[req.cookies.sessionid];
  toSendToExecutor["isBFS"] = query["isBFS"];
  toSendToExecutor["branch"] = query["branch"];
  toSendToExecutor["steps"] = query["steps"];
  toSendToExecutor["prevId"] = query["prevId"];
  toSendToExecutor["id"] = query[fileId];
  client.write(toSendToExecutor.stringify());
  // res.redirect('back')
  var toSendToUser = toSend[fileId];
  if(!toSendToUser)
  {
    toSendToUser = {};
    toSendToUser["updated"] = false;
  }
  res.send(toSendToUser);
  toSendToUser["updated"] = false;
  
});
server = http.createServer(app).listen(app.get('port'), function(){
  console.log("Express server listening on port " + app.get('port'));
});
server.on('connection', function(socket){
    console.log('Connection :  ' + socket.remoteAddress);
  });


