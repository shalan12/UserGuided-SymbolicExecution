/* --------------------- Symbolic Tree --------------------- */
var margin = { top: 40, right: 120, bottom: 20, left: 80};
var width = 960 - margin.right - margin.left;
var height = 1000 - margin.top - margin.bottom;
/*var margin = {top: 200.5, right: 120, bottom: 20, left: 275},
width = 1040,//960 - margin.right - margin.left,
height = 1040;//margin.top - margin.bottom;*/
var zoomStep = 0.2;
var actualZoomLevel = 1.0;
var i = 0,
duration = 750,
root;
var tree,diagonal,svg;
var zoom = d3.behavior.zoom()
            .on("zoom", function(){
                if (contextMenuShowing == false)
                    svg.attr("transform", "translate(" + d3.event.translate + ")scale(" + d3.event.scale + ")");
            });
var container;
function setup()
{
    tree = d3.layout.tree()
    .size([height, width]);
    diagonal = d3.svg.diagonal()
    .projection(function(d) { return [d.x, d.y]; });
    var temp = d3.select("#graph")
        .append("svg")
        .attr("viewbox", "0,0,960,500");
    svg = temp
        // .attr("width", width+ margin.right + margin.left)
        // .attr("height", height+ margin.top + margin.bottom)
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
    container = svg;
    temp.on("mousedown", function(){
            if(d3.event.button === 2){
                d3.event.stopImmediatePropagation();
            };
        })
        .call(zoom).on("dblclick.zoom", function(){svg.attr("transform", "translate(" + margin.left + "," + margin.top + ")");
        zoom.scale(1);});
}
function zoomIn(){
    //Calculate and set the new zoom level 
    actualZoomLevel = roundFloat(parseFloat(actualZoomLevel) + parseFloat(zoomStep));
    zoom.scale(actualZoomLevel);
    //Get the actual position of the container
    var xPosition = d3.transform(container.attr("transform")).translate[0];
    var yPosition = d3.transform(container.attr("transform")).translate[1];
    //Esecute the transformation setting the actual position and the new zoom level
    container.attr("transform", "translate(" + xPosition + ", " + yPosition + ")scale(" + zoom.scale() + ")");
}

function zoomOut(){
    actualZoomLevel = roundFloat(parseFloat(actualZoomLevel) - parseFloat(zoomStep));
    zoom.scale(actualZoomLevel);
    var xPosition = d3.transform(container.attr("transform")).translate[0];
    var yPosition = d3.transform(container.attr("transform")).translate[1];
    container.attr("transform", "translate(" + xPosition + ", " + yPosition + ")scale(" + zoom.scale() + ")");
}
function roundFloat(value){
    return value.toFixed(2);
}
/*var rect = svg.append("svg:rect")
    .attr("width", width)
    .attr("height", height);
rect.call(d3.behavior.zoom().y(y).on("zoom", function(){svg.select("g.y.axis").call(yAxis);}))*/

var treeData = [];
var numSteps, branchSelected, explore;
var beginSymbolicExecution = false;
function setDefaults()
{
    numSteps = 1;
    branchSelected = 0;
    explore = 1;
}
var numOfCodeLines = 0;
var contextMenuShowing = false;
var contextmenu = [
            {
                title: 'Exclude',
                action: function(elm,d,i) {
                    //console.log(elm + " " + d.node + " " + i);
                    if (contextMenuShowing == true)
                    {
                        d3.select("#formmenu").remove();
                        contextMenuShowing = false;
                    }
                    else
                    {
                        $.ajax({
                            url: "/exclude",
                            data: {"nodeid":d.node, "isPing":false}  // temp -- isPing should be false at first, then should keep pinging until EXPECTED reply received
                        }).done(function(){
                                console.log(d3.select("#name"+d.node).style("fill", "grey"));
                                var todo = [d];
                                while (todo)
                                {
                                    var curr = todo.pop();
                                    excludeNode(curr.node);
                                    //d3.select("#name"+curr.node).style("fill", "grey");
                                    for (var line = curr.startLine; line <= curr.endLine; line++)
                                    {
                                        document.getElementById(line).style.backgroundColor = 'red';
                                    }   
                                    d3.select("#name"+curr.node).on('contextmenu', function(d, i){
                                        console.log("Don't do anything");
                                    });
                                    d3.select("#name"+curr.node).on('dblclick', function(d,i){
                                        console.log("Dont get next element");
                                    });
                                    if(curr.children)
                                        todo = todo.concat(curr.children);
                                }

                        });
                    }
                }
            },
            {
                title: 'Options',
                action: function(elm, d, i) {/*
                    console.log('You have clicked the second item!');
                    console.log('The data for this circle is: ' + d);*/
                    if(contextMenuShowing) {
                        //d3.event.preventDefault();
                        d3.select("#formmenu").remove();
                        contextMenuShowing = false;
                    }
                    else 
                    {
                        d3_target = d3.select(d3.event.target);
                        d3.event.preventDefault();
                        contextMenuShowing = true;
                        datum = d3_target.datum();
                        menu = d3.select("#graph")
                        .append("div")
                        .attr("class", "well bs-component col-lg-3")
                        .attr("id", "formmenu");
                        // .style("margin-left", d.x-50 +"px")
                        // .style("margin-top", -500 + d.y + 50 +"px");
                        menu.html(
                                '<legend>Options:</legend>'+
                                '<form class=\'form-group\' id=\'menuoptions\' onsubmit="return handleMenuOptions(\''+d.node+'\')">' + 
                                "Number of steps to explore:<br>" +
                                    "<input type='text' name='steps'>" +
                                    "<br>" +
                                    "<p>Explore by: </p>" + 
                                    "<input type='radio' name='explore' value='1' checked=''> BFS" +
                                    "<br>" +
                                    "<input type='radio' name='explore' value='0'> DFS" +
                                    "<br>" + 
                                    "<p>Select Branch</p>" +
                                    "<input type='radio' name='branch' value='0' checked=''> Left" +
                                    "<br>" +
                                    "<input type='radio' name='branch' value='1'> Right" +
                                    "<br><br>" +
                                    "<input type='submit' value='Submit'>" +

                                "</form>");
                    }
                }
            }
        ]



/* ---------------------- Tree update and node handling ------------------------- */

function update(source) {
    // Compute the new tree layout.
    var nodes = tree.nodes(root).reverse(),
    links = tree.links(nodes);

    // Normalize for fixed-depth.
    nodes.forEach(function(d) { d.y = (d.depth * 100)+30; });

    // Update the nodes…
    var node = svg.selectAll("g.node")
    .data(nodes, function(d) { return d.id || (d.id = ++i); });

    // Enter any new nodes at the parent's previous position.
    var nodeEnter = node.enter().append("g")
    .attr("class", "node")
    .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; })
    .on("click", function(d){highlightPathToNode(d);})
    .on("dblclick", click);
    
    nodeEnter.append("circle")
    .attr("r", 1e-6)
    .attr("id", function(d){ return 'name' + d.node; })
    .style("fill", function(d) {         
        if(d.excluded === true)
        {
            return "grey";
        }
        else if(d.children)
        {   
            return "lightsteelblue";
        } 
        else
        { 
            return "#fff";
        }
    })   // return d.children ? "lightsteelblue" : "#fff"; 
    .call(d3.helper.tooltip(
        function(d, i){
            return d.text + " \n<b>Constraint: </b> \n" + d.constraints;
        }
    ))
    .each(function(d){checkForModel(d)})
    .on('contextmenu', d3.contextMenu(contextmenu));

    nodeEnter.append("text")
    //.attr("y", function(d) { return d.children ? -18 : 18; })
    .attr("dy", ".35em")
    .attr("text-anchor", "middle")
    .text(function(d) { return d.node; })
    .style("fill-opacity", 1e-6)
    .style("color", "#ebebeb");



  // Transition nodes to their new position.
    var nodeUpdate = node.transition()
    .duration(duration)
    .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });

    nodeUpdate.select("circle")
    .attr("r", 10)
    .style("fill", function(d) { 
        if(d.excluded === true)
        {
            return "grey";
        }
        else if(d.children)
        {   
            return "lightsteelblue";
        } 
        else
        { 
            return "#fff";
        }
    });

    nodeUpdate.select("text")
    .style("fill-opacity", 1);

  // Transition exiting nodes to the parent's new position.
    var nodeExit = node.exit().transition()
    .duration(duration)
    .attr("transform", function(d) { return "translate(" + source.y + "," + source.x + ")"; })
    .remove();

    nodeExit.select("circle")
    .attr("r", 1e-6);

    nodeExit.select("text")
    .style("fill-opacity", 1e-6);

  // Update the links…
    var link = svg.selectAll("path.link")
    .data(links, function(d) { return d.target.id; });

  // Enter any new links at the parent's previous position.
    link.enter().insert("path", "g")
    .attr("class", "link")
    .attr("d", function(d) {
        var o = {x: source.x0, y: source.y0};
        return diagonal({source: o, target: o});
    });

  // Transition links to their new position.
    link.transition()
    .duration(duration)
    .attr("d", diagonal);

  // Transition exiting nodes to the parent's new position.
    link.exit().transition()
    .duration(duration)
    .attr("d", function(d) {
    var o = {x: source.x, y: source.y};
        return diagonal({source: o, target: o});
    })
    .remove();

  // Stash the old positions for transition.
    nodes.forEach(function(d) {
        d.x0 = d.x;
        d.y0 = d.y;
    });
}



function updateGraph()
{
    root = treeData[0]
    root.x0 = height/2;
    root.y0 = 0;
    update(root);

    d3.select(self.frameElement).style("height", "800px");
}

// Toggle children on click.
function click(d) {
/*      if (d.children) {
        d._children = d.children;
        d.children = null;
      } else {
        d.children = d._children;
        d._children = null;
      }*/
      //----------------//
      //update(d);
      //----------------//
    setDefaults();
    getNext(d.node);
}

function highlightPathToNode(node)
{
    d3.selectAll("circle").style("stroke", "steelblue");
    hightlightCode(1, numOfCodeLines, '#ebebeb');
    document.getElementById("name"+node.node).style.stroke = '#FF6347';
    hightlightCode(node.startLine, node.endLine, '#FF6347');
    var curr = node;
    while (curr.parent)
    {
        document.getElementById("name"+curr.parent.node).style.stroke = '#FFB2A4';
        hightlightCode(curr.parent.startLine, curr.parent.endLine, '#FFB2A4');
        curr = curr.parent;
    }
}

function hightlightCode(startLine, endLine, color)
{
    for (var line = startLine; line <= endLine; line++)
    {   
        document.getElementById(line).style.backgroundColor = color;
    }    
}

function excludeNode(nodeID)
{
    var toExclude = treeData.filter(function ( obj ) {
        return obj.node === nodeID;
    })[0];
    toExclude.excluded = true; 
}

function excludeStatement(startLine,isPing)
{
    isPing = isPing || false;
    $.ajax({
        url: "/exclude",
        data: {"lineno": startLine, "isPing":isPing, "isNode": false} 
    }).done(function(resp){
        if (resp.minLine !== undefined)
        {
            var start = resp.minLine,
            end = resp.maxLine;
            hightlightCode(start,end,'red');
        } 
        else setTimeout(function(){excludeStatement(startLine,true);}, 1000);
    });
}
/*var exclusionMenu = {
                title: 'Exclude',
                action: excludeStatement(line);
}*/
function addNode(nodeObj)
{
    var node = {"node": nodeObj.node, "text": nodeObj["text"], "parent": nodeObj["parent"], "children": [], "constraints": nodeObj["constraints"], 
            "startLine": nodeObj["startLine"], "endLine": nodeObj["endLine"], "excluded":false, "addModel": nodeObj["addModel"]};
    treeData.push(node);
    for (var j = 0; j < treeData.length; j++)
    {
        console.log("before");
        console.log(treeData[j]);
        if (treeData[j].node == node["parent"])
        {
            if(treeData[j].children)
                treeData[j].children.push(node);
            else treeData[j].children = [node];
        }
        console.log("after");
        console.log(treeData[j]);
    }
    updateGraph();
    console.log(numOfCodeLines);
    d3.selectAll("circle").style("stroke", "steelblue");
    document.getElementById("name"+node.node).style.stroke = '#FF6347';
    /*hightlightCode(1, numOfCodeLines, '#ebebeb');*/
    hightlightCode(node.startLine, node.endLine, 'gold');
}
/* ---------------- Step To Get Next Node --------------------------------------- */
function addNodes(data)
{
    if (data.nodes === undefined || data.nodes.length == 0)
    {
        var noNodeAlert = d3.select("#graph")
            .append("div")
            .attr("class", "col-lg-4 bs-component alert alert-dismissible alert-warning")
            .attr("id", "noNode-alert");
        noNodeAlert.html(
        '<button type="button" class="close" data-dismiss="alert">&times;</button>' +
        'This node has no more branches to explore.\n' +
        '<input type="button" class="btn btn-primary" id="noNodeBtn" value="OK">');
        $("#noNodeBtn").on("click", function(){d3.select("#noNode-alert").remove();});   
    }
    else
    {
        for (var i =0; i < data.nodes.length; i++)
            addNode(data.nodes[i]);
    }
}
function getNext(nodeID,isPing)
{
    if(!beginSymbolicExecution)
    {
        document.getElementById("Step").style.display = "none";
        beginSymbolicExecution = true;
    }
    isPing = isPing || false;
    $.get('next', {'isBFS': explore, 'branch': branchSelected, 
                 'steps': numSteps, 'prevId': nodeID, 'isPing': isPing}, function(data){
            if(data.updated)
            {
                addNodes(data);
            }
            if(!data.completed)
            {
                setTimeout(function(){getNext(nodeID,true);}, 1000);
            }
    });
}

function addModel(id)
{
    var _modelOptions = document.getElementById('modelData');
    var inputs = document.getElementsByName('focusedInput[]');
/*    inputConstraints  = [].map.call(inputs, function( input ) {
        return input.value;
    }).join( ',' );*/
    var inputConstraints = "";
    for (var i =0; i < inputs.length-1; i++)
    {
        inputConstraints = inputConstraints + inputs[i].value + ",";

    }
    inputConstraints += inputs[inputs.length-1].value;
    var outputs = document.getElementsByName('focusedOutput[]');
    /*expectedOutputs  = [].map.call(outputs, function( output) {
        return output.value;
    }).join( ',' );*/
    var expectedOutputs = "";
    for (var i =0; i < outputs.length-1; i++)
    {
        expectedOutputs = expectedOutputs + outputs[i].value + ",";

    }
    expectedOutputs += outputs[outputs.length-1].value;
    
    d3.select("#model-input").remove();

    console.log(inputConstraints);
    console.log(expectedOutputs);
    console.log(id);
    console.log(JSON.stringify({
        url: "/constraints",
        data: {"nodeid": id, "inputConstraints": inputConstraints, "expectedOutput": expectedOutputs}
    }));
    url = "/constraints";
    query = {"id": id, "inputConstraints": inputConstraints, 
    "expectedOutput": expectedOutputs};
    var f = function sendReq(isPing)
    {
        query["isPing"] = isPing;
        $.ajax({
                url: "/constraints",
                data: query}).done(function(resp){
                if(!resp.completed)
                {
                    setTimeout(function(){sendReq(true);}, 1000);
                }
                else
                {
                    addNodes(resp);
                }
            });
    };
    f(false);
    return false;
}        

function addModelInput()
{
    var newInputsDiv = document.createElement('div');
    newInputsDiv.innerHTML = '<label class="control-label" for="input"> Provide the input constraints: </label>' +
            '<input class="form-control" id="input" class="focusedInput" name="focusedInput[]" type="text">' +
            '<label class="control-label" for="output"> Provide the output for the input constraint: </label>' +
            '<input class="form-control" id="output" class="focusedOutput" name="focusedOutput[]" type="text">'+
            '<br><br>';
    document.getElementById("modelFormInputs").appendChild(newInputsDiv);
}

function getModelData(isNode, node, func)
{
    var id;
    if(isNode.valueOf() == 'true')
    {
        id = node.node;
    }
    else id = func;
    d3.select("#model-alert").remove();
    var modelForm = d3.select("#graph")
    .append("div")
    .attr("id", "model-input")
    .attr("class", "well bs-component col-lg-3");
/*    .style("margin-left", node.x+120+"px")
    .style("margin-top",node.y-350+"px");*/
    modelForm.html(
        '<legend>Add Model for function:</legend>' +
        'Available variables to choose: x, y, z' +
        '<form class=\'form-group\' id=\'modelData\' onsubmit="return addModel(\''+id+'\')">' +
        '<br>' +
        '<div id="modelFormInputs">' +
            '<label class="control-label" for="input"> Provide the input constraints: </label>' +
            '<input class="form-control" id="input" class="focusedInput" name="focusedInput[]" type="text">' +
            '<label class="control-label" for="output"> Provide the output for the input constraint: </label>' +
            '<input class="form-control" id="output" class="focusedOutput" name="focusedOutput[]" type="text">'+
            '<br><br>' +
        '</div>' +
        '<input type="button" value="Add another constraint" onClick="addModelInput()">' +
        '<input type="submit" value="Submit">' +
        '</form>');
}

function checkForModel(selection)
{
        if (selection.addModel.valueOf() == 'true')
        {   
            var modelAlert = d3.select("#graph")
            .append("div")
            .attr("class", "col-lg-4 bs-component alert alert-dismissible alert-warning")
            .attr("id", "model-alert");
/*            .style("margin-left", selection.x+120 + "px")
            .style("margin-top", selection.y-350 + "px");*/
            var a = selection;
            modelAlert.html(
                '<button type="button" class="close" data-dismiss="alert">&times;</button>' +
                'An external function call was executed at node \''+selection.node+'\'. Please provide a model for the function.\n' +
                '<br></br>' +
                '<input type="button" class="btn btn-primary" id="addModelBtn" value="Add Model">');
            $("#addModelBtn").on("click", function(){getModelData('true',selection)});
        }    
}

function addModelForFunction(func)
{
    getModelData('false',func.name);
    hightlightCode(func.startLine, func.endLine, 'gold');
}

/* ---------------- Nodes Right Click Menu Options ------------------------------ */


function handleMenuOptions(nodeID)
{
    try
    {
        var _explore = document.getElementById('menuoptions');
        numSteps = _explore.elements.namedItem('steps').value;
        branchSelected = _explore.elements.namedItem('branch').value;
        explore = _explore.elements.namedItem('explore').value;
        console.log(numSteps);
        console.log(branchSelected);
        console.log(explore);
        getNext(nodeID);
        d3.select('#formmenu').remove();
        contextMenuShowing = false;
    }
    catch(e){
        console.log(e);
        console.log('inside loser catch');
    }
    return false;
}




/* ---------------- Upload File and Code --------------------- */

// var _submit = document.getElementById('_submit'), 
// _file = document.getElementById('_file');
// //_progress = document.getElementById('_progress'); 
// var data = new FormData();
// function loaded(file) {
//     var reader = new FileReader();
//     reader.readAsBinaryString(file);
//     reader.onload = function(evt) {
//         var fileString = evt.target.result.replace(/\r/g, "\n");
//         var splitted = fileString.split("\n");
//         document.getElementById("filecode").style.display = "block";
//         numOfCodeLines = splitted.length;    
//         for (var i = 1; i <= splitted.length; i++)
//         {
//             $("#codedata").append('<pre contextmenu="exclusionMenu" id = "'+i+'">'+ i + "." + splitted[i-1] + '<menu type="context" id="exclusionMenu"><menuitem label="Exclude" onclick="excludeStatement(\''+i+'\')"></menuitem</menu></pre>');  
//         }
//     }
        
// }
// var upload = function(){
//     document.getElementById('instructions').style.display = "none";
//     document.getElementById('beginSymbolicExecutiom').style.display = "block";
//     if(_file.files.length === 0){
//       return;
//     }

//     data.append('SelectedFile', _file.files[0]);
//     var request = new XMLHttpRequest();
//     request.onreadystatechange = function(){
//         if(request.readyState == 4){
//             try {
//                 var resp = JSON.parse(request.response);
//             } catch (e){
//                 var resp = {
//                     status: 'error',
//                     data: 'Unknown error occurred: [' + request.responseText + ']'
//                 };
//             }
//             console.log(resp.status + ': ' + resp.data);
//             $("#codeinstructions").append('<p class="panel-body">Upload Successful!</p>');
//         }
//     };
//     request.open('POST', 'upload');
//     request.send(data);
//     loaded(_file.files[0]);
// }
// _submit.addEventListener('click', upload);


/*----------------------- Upload Sample File, Code and Functions ----------------- */


function displayFunctionNames(functions)
{
    var functionNamesDiv = document.getElementById('functionNames');
    var buttonID = ""
    for(var i = 1; i <= functions.length; i++)
    {
        $('#functionNames').append('<p id="'+functions[i-1].name+'">'+ i + ". " + functions[i-1].name + '</p>');
        buttonID = "addModelFor" + functions[i-1].name;
        console.log(buttonID);
        $('#functionNames').append('<input type="button" onClick="addModelForFunction('+functions[i-1]+')" id="'+buttonID+'" value="Add Model">');
    }

}

function uploadSample(isPing)
{    
    var sampleSubmit = document.getElementById('_submitSample'),
    sampleName = document.getElementById('selectSampleCode').value;
    var sampleFilePath = "file:///home/habiba/Documents/University/Sproj/Sproj/src/SUT/" + sampleName;
    var functions = [{"name": "notMain"},{"name":"notMain2"}];
    isPing = isPing || false;
    $.ajax({
        url: "/sample",
        data: {"fileID": sampleName, "isPing":isPing} 
    }).done(function(resp){
        $.get("/main",function (data){
            $("#mainContent").html(data);
            document.getElementById('Back').style.display = "block";
            setup();
            var fileString = resp.file.replace(/\r/g, "\n");
            var splitted = fileString.split("\n");
            numOfCodeLines = splitted.length;
            for (var i = 1; i <= splitted.length; i++)
            {
                $("#codedata").append('<pre id = "'+i+'" ondblclick="excludeStatement(\''+i+'\')">'+ i + "." + splitted[i-1]+'</pre>');
            }
            displayFunctionNames(resp.functions);

            // document.getElementById('instructions').style.display = "none";
            // document.getElementById('beginSymbolicExecutiom').style.display = "block";
        });
       /* if(resp.file !== undefined)
        {
            document.getElementById('instructions').style.display = "none";
            document.getElementById('beginSymbolicExecutiom').style.display = "block";        
            loaded(resp.file);
        }
        else
        {
           // setTimeout(uploadSample(true),1000);
        
        }*/
    });
}

function goBack()
{
    $.get({
        url: "/refresh",
    });
}