
/**
 * Module dependencies.
 */

var express = require('express'),
  routes = require('./routes'),
  user = require('./routes/user'),
  http = require('http'),
  path = require('path'),
  async = require('async'),
  Q = require("q");

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
var Chance = require('chance');
var chance = new Chance();
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


var toSend = [];
var MSG_TYPE_EXCLUDENODE = 100;
var MSG_TYPE_EXPANDNODE = 200;
var MSG_TYPE_FUNCNAMES = 300;
/////////////////////////////////////
////////////UTILS///////////////////
///////////////////////////////////

function isPingQuery(query)
{
  return (query['isPing'].valueOf() != 'false');
}
function makeBitCodeFile(oldpath,newpath,id)
{
  var newFileName = Q.defer();
  async.waterfall([
    function(callback){
      fs.readFile(oldpath, function (err, data)
      {
        var prefix = '#include "../../errors.h"';
        prefix = "";
        data =  prefix + data;
        callback(err,newpath+".cpp",data)
      });},
    function(path,data,callback){
      fs.writeFile(path, data,function(err)
      {      
        console.log(newpath+".cpp" + " file written")
        bcFile = newpath+".bc";
        toExec = "clang-3.5 -emit-llvm " + newpath  + ".cpp -g -c -o " + bcFile;
        callback(err,toExec) 
      });
    },
    function(toExec,callback)
    {
      exec(toExec,function (error, stdout, stderr) {
        if(error)
        {
          console.log(error);
          newFileName.reject(error);
        }
        else
        {
          console.log(newpath+".bc" + " file emitted")
          newFileName.resolve(bcFile); // store mapping between sessionid and filename
          // things from this map will need to be deleted later .. when client leaves .. or when execution is completed
        }
        callback(error);
      });
      
    }
  ]);
  
  return newFileName.promise;
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
    if(data.fin  === "1") client.end("FIN");
    else
    {
      if(data["type"] == MSG_TYPE_EXPANDNODE)
      {
        var fileId = data.fileId;
        toSend[fileId] = {'nodes':Array(),'updated':true, 'completed':true};
        var toSendToUser = {}
        data = data.nodes;
        for(var i = 0; i < data.length; i++)
        {
          toSendToUser["node"] = data[i]["node"];
          toSendToUser["parent"] = data[i]["parent"];
          toSendToUser["text"] = data[i]["text"];
          toSendToUser["constraints"] = data[i]["constraints"];
          toSendToUser["startLine"] = data[i]["startLine"];// - 1;
          toSendToUser["endLine"] = data[i]["endLine"];//  - 1;
          toSendToUser["addModel"] = data[i]["addModel"];
          toSendToUser["extra"] = data[i]["extra"];
          toSend[fileId]['nodes'].push(lodash.cloneDeep(toSendToUser));
        }
      }
      else if (data["type"] == MSG_TYPE_EXCLUDENODE)
      {
        toSend[data.fileId] = {"minLine": data.minLine , "maxLine": data.maxLine};
      }
      else if (data["type"] == MSG_TYPE_FUNCNAMES)
      {
        toSend[data.fileId] = {};
        toSend[data.fileId]["functions"] = data["functions"];
        console.log("toSend = " + JSON.stringify(toSend[data.fileId]));
      }
    }  
   
});

app.configure(function(){
  app.set('port', process.env.PORT || 80);
  app.set('views', __dirname + '/views');
  app.set('view engine', 'ejs');
  app.engine('.html', require('ejs').renderFile);
  app.use(express.favicon());
  app.use(express.logger('dev'));
  app.use(express.cookieParser());
  app.use(express.session({secret: 'i12kroejfpajwke-012joepegvn2je0j2pwkps'}));
  app.use(express.bodyParser());
  app.use(express.methodOverride());
  app.use(app.router);
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
  var filename = chance.string({length:10, pool:"abcdefghijklmnopqrstuvwxyz"});
  var newPath = __dirname + "/uploads/" + filename;
  makeBitCodeFile(path, newPath, filename).then(function(fileId)
  {
    req.session.file = fileId;
    res.sendfile(path);
  });
});
app.post('/upload',function(req,res){
  var extension = '.cpp'; 
  var filename = req.cookies.sessionid;
  var newPath = __dirname + "/uploads/" + filename; //__dirname is a global, containing the current dir 
  if (req.session.file == undefined)
    req.session.file = makeBitCodeFile(req.files.SelectedFile.path, newPath, req.cookies.sessionid);
  res.redirect('back'); // return to the previous page
});

app.get('/next',function(req,res){
  var fileId = req.session.file;
  query = req.query;
  if(!isPingQuery(query))
  {
    toSendToExecutor = {};
    toSendToExecutor["val"] = {}
    toSendToExecutor["val"]["isBFS"] = (typeof query["isBFS"] === 'undefined' || query["isBFS"] === '') ? 0:query["isBFS"];
    toSendToExecutor["val"]["branch"] = (typeof query["branch"] === 'undefined' || query["branch"] === '' ) ? 0:query["branch"];
    toSendToExecutor["val"]["steps"] = (typeof query["steps"] === 'undefined' || query["steps"] === '') ? 1:query["steps"];
    toSendToExecutor["val"]["prevId"] = (typeof query["prevId"] === 'undefined' || query["prevId"] === '') ? -1:query["prevId"];
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
app.get('/constraints',function(req,res){
  var fileId = req.session.file;
  query = req.query;
  console.log(query);
  if(!isPingQuery(query))
  {
    var zipped = lodash.zip(query["inputConstraints"].split(","), query["expectedOutput"].split(",")); 
    var pairs = lodash.map(zipped, function(x){
      return {"constraints":x[0],"returnValue":x[1]};
    });
    client.write(JSON.stringify({"id":fileId, "val":{"nodeid": query["nodeid"], "pairs": pairs}}));
  }
  else if (toSend[fileId])
  {
      console.log("sending" + JSON.stringify(toSend[fileId]))
      res.send(toSend[fileId]);
      toSend[fileId] = null;
      return;
  }
  res.send({}); 

});
app.get('/metadata', function(req,res)
{
  var fileId = req.session.file;
  console.log(toSend[fileId]);
  query = req.query;
  if (!isPingQuery(query))
  {
    toSendToExecutor = {"id" : fileId, "type": MSG_TYPE_FUNCNAMES};
    console.log("sending " + JSON.stringify(toSendToExecutor));
    client.write(JSON.stringify(toSendToExecutor));
  }
  if (toSend[fileId])
  {
    res.send(toSend[fileId]);
    toSend[fileId] = null;
  }
  else
  {
    res.send({});
  }
  
});
app.get('/exclude', function(req,res){
  var fileId = req.session.file;
  query = req.query;
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
  else res.send({});  
  
});
server = http.createServer(app).listen(app.get('port'), function(){
  console.log("Express server listening on " + server.address().address + ":" + app.get('port'));
});
server.on('connection', function(socket){
  console.log('Connection :  ' + socket.remoteAddress);
  });


