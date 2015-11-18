/* --------------------- Symbolic Tree --------------------- */


var margin = {top: 20, right: 120, bottom: 20, left: 120},
width = 960 - margin.right - margin.left,
height = 500 - margin.top - margin.bottom;
var i = 0,
duration = 750,
root;
var tree = d3.layout.tree()
.size([height, width]);
var diagonal = d3.svg.diagonal()
.projection(function(d) { return [d.x, d.y]; });
var svg = d3.select("#graph").append("svg")
.attr("width", width + margin.right + margin.left)
.attr("height", height + margin.top + margin.bottom)
.append("g")
.attr("transform", "translate(" + margin.left + "," + margin.top + ")")
var treeData = [];
var numSteps, branchSelected, explore;
var contextMenuShowing = false;
numSteps = 1;
/* ---------------------- Tree update and node handling ------------------------- */

function update(source) {
    // Compute the new tree layout.
    var nodes = tree.nodes(root).reverse(),
    links = tree.links(nodes);

    // Normalize for fixed-depth.
    nodes.forEach(function(d) { d.y = d.depth * 100; });

    // Update the nodes…
    var node = svg.selectAll("g.node")
    .data(nodes, function(d) { return d.id || (d.id = ++i); });

    // Enter any new nodes at the parent's previous position.
    var nodeEnter = node.enter().append("g")
    .attr("class", "node")
    .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; })
    .on("click", click);

    nodeEnter.append("circle")
    .attr("r", 1e-6)
    .style("fill", function(d) { return d.children ? "lightsteelblue" : "#fff"; })
    .call(d3.helper.tooltip(
    function(d, i){
        return d.text + " \n<b>Constraint: </b> \n" + d.constraints;
    }
    ))
    .on('contextmenu', function(d, i){
        menu = d3.select("body")
        .append("div")
        .attr("class", "menu")
        .style("left", "300px")
        .style("top", "500px");
        menu.html(
            '<form id='menuoptions' onsubmit="return handleMenuOptions(\''+d.id+'\')">' + 
                "Number of steps to explore:<br>" +
                "<input type='text' name='steps'>" +
                "<br>" +
                "<p>Explore by: </p>" + 
                "<input type='radio' name='explore' value='BFS'> BFS" +
                "<br>" +
                "<input type='radio' name='explore' value='DFS'> DFS" +
                "<br>" + 
                "<p>Select Branch</p>" +
                "<input type='radio' name='branch' value='left'> Left" +
                "<br>" +
                "<input type='radio' name='branch' value='right'> Right" +
                "<br><br>" +
                "<input type='submit' value='Submit'>" +
            "</form>");
        contextMenuShowing = true;
    });
    nodeEnter.append("text")
    .attr("y", function(d) { return d.children ? -18 : 18; })
    .attr("dy", ".35em")
    .attr("text-anchor", "middle")
    .text(function(d) { return d.node; })
    .style("fill-opacity", 1e-6);



  // Transition nodes to their new position.
    var nodeUpdate = node.transition()
    .duration(duration)
    .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });

    nodeUpdate.select("circle")
    .attr("r", 10)
    .style("fill", function(d) { return d.children ? "lightsteelblue" : "#fff"; });

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
    root.x0 = height / 2;
    root.y0 = 0;
    update(root);

    d3.select(self.frameElement).style("height", "500px");
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
    getNext(d.id);
}


/* ---------------- Step To Get Next Node --------------------------------------- */

function getNext(nodeID)
{

    for (var i = 0; i < numSteps; i++)
    {
        $.get('next', {isBFS: explore, branch: branch, steps: numSteps, prevID: nodeID}, function(data)
        {
            nodeObj = JSON.parse(data);  
            if(!nodeObj.updated && stepsPerformed <= steps)
            {
                setTimeout(getNextContext(nodeID), 1000);
                stepsPerformed = stepsPerformed + 1;
            }
            else
            {
                console.log("fin = " + nodeObj.fin);
                if(nodeObj.fin !== "0") 
                {  //document.getElementById("Step").disabled = true;
                }
                else
                {
                    var node = {"node": nodeObj.node, "text": nodeObj["text"], "parent": nodeObj["parent"], "children": [], "constraints": nodeObj["constraints"], 
                    "startLine": nodeObj["startLine"], "endLine": nodeObj["endLine"]};
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
                    for (var line = startLine; line < endLine; line++)
                    {
                        document.getElementById(line).style.color = 'darkslateblue';
                    }
                }
            }

        });
    }

    numSteps = 1;
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
        d3.select('.menu').remove();
    }
    catch(e){
        console.log(e);
        console.log('inside loser catch');
    }
    return false;
}




/* ---------------- Upload File and Code --------------------- */

var _submit = document.getElementById('_submit'), 
_file = document.getElementById('_file'), 
_progress = document.getElementById('_progress'); 
var data = new FormData();
function loaded(evt) {
    var fileString = evt.target.result.replace(/\r/g, "\n");
    var splitted = fileString.split("\n");
    for (var i = 1; i <= splitted.length; i++)
    {
        $("#codedata").append('<p id = "'+i+'">'+ i + ". " + splitted[i] + '</p>');  
    }
        
}
var upload = function(){
   if(_file.files.length === 0){
      return;
    }

    data.append('SelectedFile', _file.files[0]);
    var request = new XMLHttpRequest();
    request.onreadystatechange = function(){
        if(request.readyState == 4){
            try {
                var resp = JSON.parse(request.response);
            } catch (e){
                var resp = {
                    status: 'error',
                    data: 'Unknown error occurred: [' + request.responseText + ']'
                };
            }
            console.log(resp.status + ': ' + resp.data);
        }
    };

    request.upload.addEventListener('progress', function(e){
      _progress.style.width = Math.ceil(e.loaded/e.total) * 100 + '%';
    }, false);

    request.open('POST', 'upload');
    request.send(data);
    var reader = new FileReader();
    reader.readAsBinaryString(_file.files[0]);
    reader.onload = loaded;
}
_submit.addEventListener('click', upload);