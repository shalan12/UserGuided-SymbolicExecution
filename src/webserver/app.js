
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
var lodash = require('lodash');
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
var MSG_TYPE_EXCLUDENODE = 100;
var MSG_TYPE_EXPANDNODE = 200;
/////////////////////////////////////
////////////UTILS///////////////////
///////////////////////////////////
function isEmptyObject(obj) 
{
  return !Object.keys(obj).length;
}
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
function isPingQuery(query)
{
  return (query['isPing'].valueOf() != 'false');
}
function makeBitCodeFile(oldpath,newpath,id)
{
  fs.readFile(oldpath, function (err, data) 
  {
    fs.writeFile(newpath+".cpp", data,function(err)
    {      
      console.log(newpath+".cpp" + " file written")
      bcFile = newpath+".bc";
      toExec = "clang-3.5 -emit-llvm " + newpath  + ".cpp -g -c -o " + bcFile;
      exec(toExec, function (error, stdout, stderr) {
        if(error) console.log(error)
        else console.log(newpath+".bc" + " file emitted")
        map[id] = bcFile; // store mapping between sessionid and filename
        // things from this map will need to be deleted later .. when client leaves .. or when execution is completed
      });
    });
     
  });
}
////////////////////////////////////
//////////END-UTILS////////////////
//////////////////////////////////

client = net.createConnection(PORT)
client.on('connect',function()
{
  console.log("Connected to symbolicexecutor\n")
});
client.on('data',function(data)
{
    console.log("recieved from symbolicexecutor " + data);
    data = JSON.parse(data);
    if(data.fin  === "1")
    {
      var idx = map.indexOf(data.sessionid);
      if(idx != -1) map.splice(idx,1);
      client.end("FIN");
      //delete file
    }
    else
    {
      if(data["type"] == MSG_TYPE_EXPANDNODE)
      {
        var fileid = data.fileId;
        toSend[fileid] = {'nodes':Array(),'updated':true, 'completed':true};
        var toSendToUser = {}
        data = data.nodes;
        for(var i = 0; i < data.length; i++)
        {
          toSendToUser["node"] = data[i]["node"];
          toSendToUser["parent"] = data[i]["parent"];
          toSendToUser["text"] = data[i]["text"];
          toSendToUser["constraints"] = data[i]["constraints"];
          toSendToUser["startLine"] = data[i]["startLine"];
          toSendToUser["endLine"] = data[i]["endLine"];
          toSend[fileid]['nodes'].push(lodash.cloneDeep(toSendToUser));
        }
      }
      else if (data["type"] == MSG_TYPE_EXCLUDENODE)
      {
        toSend[data.fileId] = {"minLine": data.minLine, "maxLine": data.maxLine};
      }
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

app.get('/main',function(req,res){
  res.render('main',{
    title: 'Home'
  });
});
app.get('/refresh',function(req,res){
  console.log("here");
  var filename = map[req.cookies.sessionid].slice(0,-3);
  console.log(filename);
  var idx = map.indexOf(filename);
  if(idx > -1)
  {
    map.splice(idx,1);
  }
  res.clearCookie(req.cookies.sessionid);
  exec('rm /uploads/' + filename+'.bc',  function(error, stdout, stderr){
      if(error) console.log(error);
  });
  exec('rm /uploads/' + filename+'.cpp',function(error, stdout, stderr){
      if(error) console.log(error);
  });
  res.redirect('/');
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
app.get('/sample',function(req,res)
{
  var path = __dirname + "/samples/" + req.query["fileID"];
  var filename = req.cookies.sessionid;
  var newPath = __dirname + "/uploads/" + filename;
  makeBitCodeFile(path, newPath, req.cookies.sessionid);
  res.sendfile(path);
});
app.post('/upload',function(req,res){
  var extension = '.cpp'; 
  var filename = req.cookies.sessionid;
  var newPath = __dirname + "/uploads/" + filename; //__dirname is a global, containing the current dir 
  makeBitCodeFile(req.files.SelectedFile.path, newPath, req.cookies.sessionid);
  res.redirect('back'); // return to the previous page
});

app.get('/next',function(req,res){
  fileId = map[req.cookies.sessionid];
  query = req.query;
  if(!isPingQuery(query))
  {
    toSendToExecutor = {};
    var fileId = map[req.cookies.sessionid]; 
    toSendToExecutor["val"] = {}
    toSendToExecutor["val"]["isBFS"] = (typeof query["isBFS"] === 'undefined') ? 0:query["isBFS"];
    toSendToExecutor["val"]["branch"] = (typeof query["branch"] === 'undefined') ? 0:query["branch"];
    toSendToExecutor["val"]["steps"] = (typeof query["steps"] === 'undefined') ? 1:query["steps"];
    toSendToExecutor["val"]["prevId"] = (typeof query["prevId"] === 'undefined') ? -1:query["prevId"];
    toSendToExecutor["id"] = fileId;
    console.log("Sending to Executor : " );
    console.log(toSendToExecutor);
    client.write(JSON.stringify(toSendToExecutor));
  }
  //console.log("sending to user : " + JSON.stringify(toSend[fileId]));
  if(toSend[fileId])
  {
    //sendCopy = lodash.cloneDeep(toSend[fileId]);
    console.log("sending to client " + JSON.stringify(toSend[fileId]));
    res.send(toSend[fileId]);
    toSend[fileId] = null;
  }
  else
  {
    res.send({"updated":false,"completed":false});
  }
  
});

app.get('/exclude', function(req,res){
  query = req.query;
  fileId = map[req.cookies.sessionid];

  if(!isPingQuery(query))
  {
    if(query["lineno"]) 
      toSendToExecutor = {"id":fileId, "val": {"exclude":query["lineno"], "isNode":"0"}};
    else
    {
      toSendToExecutor = {"id":fileId, "val": {"exclude":query["nodeid"], "isNode":"1"}};
      //temp - because ubaid not replying in this case
    }
    res.send({});
    client.write(JSON.stringify(toSendToExecutor));

  }
  if(toSend[fileId])
  {
      res.send(toSend[fileId]);
      console.log("sending" + JSON.stringify(toSend[fileId]))
      toSend[fileId] = null;
  }
});
server = http.createServer(app).listen(app.get('port'), function(){
  console.log("Express server listening on " + server.address().address + ":" + app.get('port'));
});
server.on('connection', function(socket){
  console.log('Connection :  ' + socket.remoteAddress);
  });


