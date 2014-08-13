
function plangraphUpdate()
{
    $.post('plan',{},function(data)
    {
        if(!data)
            return;
        try 
        {
            var g = new dagreD3.Digraph();

            var config = { 
                    // "2nodes"/"1node"/"edge"
                displayPortsAs: $("#plangraph-displayPortsAs").val(),
                    // true/false
                skipResources: $("#plangraph-skipResources").prop('checked')
            }

            var dotOutput="digraph G {\n";

            var portOwner={};

            function ensureNodeExists(id,data)
            {
                if(!g.hasNode(id))
                {
                    g.addNode(id, typeof(data)!== "undefined"?data:{label: "stub for "+id});
                }
                else if(typeof(data)!== "undefined")
                    g.node(id,data);    //update data

                if(typeof(data)!== "undefined")
                {
                    if(typeof(data.item)!=="undefined")
                        dotOutput=dotOutput+id+' [shape=box,label="'+data.item.name+'"];\n';
                    else if(typeof(data.port)!=="undefined")
                        dotOutput=dotOutput+id+' [label="'+data.port.name+'"];\n';
                    else if(typeof(data.label!=="undefined"))
                        dotOutput=dotOutput+id+' [label="'+data.label+'"];\n';
                }
            }
            function addEdge(id,from,to,data)
            {
                g.addEdge(id,to,from,data);
                if(to!=="" && from!=="")
                    dotOutput=dotOutput+to+' -> '+from+';\n';
                //g.addEdge(id,from,to,data);
            }

            function getJoinedPortName(p1,p2)
            {
                return "port"+(p1<p2?p1+"-"+p2:p2+"-"+p1);
            }

            function addPlanItem(item,type)
            {
                var label;

                switch(item.name)
                {
                    case "EnemyUnit":
                    case "OwnUnit":
                        label=item.name+": "+item.data.unitType+" ("+item.data.unitID+")";
                        break;

                    case "Resource":
                        if(config.skipResources)
                            return;
                        label=item.name+": "+item.data.unitType+" ("+item.data.unitID+")"+"<br />"+"worker: "+item.data.numberOfWorkers+", minerals: "+item.data.mineralsLeft;
                        break;

                    case "MorphUnit":
                    case "Build":
                        label=item.name+": "+item.data.unitType;
                        break;

                    case "GatherMinerals":
                    case "MoveToPosition":
                    case "AttackUnit":
                        label=item.name;
                        break;

                    default:
                        label=item.name+"<br /><i>"+JSON.stringify(item.data)+"</i>";
                }

                ensureNodeExists("item"+item.id, {
                    label: "<div class=\"label\">"+label+"</div>",
                    type: type, 
                    item: item });

                if(config.displayPortsAs === "2nodes")
                {
                    var haveRequirePort=false;

                    for(var p in item.ports)
                    {
                        var port=item.ports[p];

                        var label;

                        switch(port.name)
                        {
                            case "RequireUnit":
                            case "ProvideUnit":
                            case "RequireEnemyUnit":
                            case "ProvideEnemyUnit":
                                label=port.name+": "+port.data.unitType+" ("+port.data.unitID+")";
                                break;

                            case "Resource":
                                label=port.name+": minerals: "+port.data.minerals+", gas: "+port.data.gas;
                                break;

                            case "Supply":
                                label=port.name+": race: "+port.data.race+", "+(port.data.required>0?"required: "+port.data.required:"provided: "+(-port.data.required));
                                break;

                            case "ProvideMineralField":
                            case "RequireMineralField":
                                label=port.name;
                                break;

                            case "RequireSpace":
                                label=port.name+": "+port.data.unitType;
                                break;
                            default:
                                label=port.name+"<br /><i>"+JSON.stringify(port.data)+"</i>";
                        }

                        ensureNodeExists("port"+port.id, {
                            label: "<div class=\"label\">"+label+"</div>",
                            type: "port",
                            port: port });

                        if(port.type==="require")
                        {
                            haveRequirePort=true;
                            addEdge(null,"item"+item.id, "port"+port.id, { label: "port" });
                        }
                        else
                        {
                            addEdge(null, "port"+port.id, "item"+item.id, { label: "port" });
                        }

                        if(port.data.connectedPort && 0!=port.data.connectedPort)
                        {
                            ensureNodeExists("port"+port.data.connectedPort);
                            if(port.type==="require")
                                addEdge(null,"port"+port.id,"port"+port.data.connectedPort,{label: "connected" })
                            /*else  //don't also re-connect the provide to the require port again
                                g.addEdge(null,"port"+port.data.connectedPort,"port"+port.id,{label: "connected" })*/
                        }
                        else 
                        {
                            if(port.type==="provide")
                            {
                                    //insert a invisible item to ensure nice alignment
                                ensureNodeExists("emptyItemForPort"+port.id,{label:"?"});
                                addEdge(null,"emptyItemForPort"+port.id,"port"+port.id);
                            }
                        }
                    }

                    if(!haveRequirePort)    //insert a invisible port to ensure nice alignment
                    {
                        ensureNodeExists("emptyRequirePortForItem"+item.id,{label:""});
                        addEdge(null,"item"+item.id,"emptyRequirePortForItem"+item.id, {style: 'visibility: hidden;  display: none;' });
                    }
                }
                else if(config.displayPortsAs==="1node")
                {
                    for(var p in item.ports)
                    {
                        var port=item.ports[p];
                        var label;

                        if(port.data.connectedPort && 0!=port.data.connectedPort)
                        {
                            switch(port.name)
                            {
                                case "RequireUnit":
                                case "ProvideUnit":
                                    label="Unit: "+port.data.unitType+" ("+port.data.unitID+")";
                                    break;
                                case "RequireEnemyUnit":
                                case "ProvideEnemyUnit":
                                    label="Enemy: "+port.data.unitType+" ("+port.data.unitID+")";
                                    break;

                                case "Resource":
                                    label=port.name+": minerals: "+port.data.minerals+", gas: "+port.data.gas;
                                    break;

                                case "Supply":
                                    label=port.name+": race: "+port.data.race+", "+(port.data.required>0?"required: "+port.data.required:"provided: "+(-port.data.required));
                                    break;

                                case "ProvideMineralField":
                                case "RequireMineralField":
                                    label="MineralField";
                                    break;

                                case "RequireSpace":
                                    label="Space: "+port.data.unitType;
                                    break;
                                default:
                                    label=port.name+": "+JSON.stringify(port.data);
                            }

                            var portName=getJoinedPortName(port.id,port.data.connectedPort);
                            ensureNodeExists(portName,{
                                label: "<div class=\"label\">"+label+"</div>",
                                type: "port",
                                port: port });
                            if(port.type==="require")
                            {
                                addEdge(null,"item"+item.id, portName, { label: "port" });
                            }
                            else
                            {
                                addEdge(null, portName, "item"+item.id, { label: "port" });
                            }
                        }
                    }
                }
                else if(config.displayPortsAs==="edge")
                {
                    for(var p in item.ports)
                    {
                        var port=item.ports[p];

                        portOwner[port.id]=item.id;

                        if(port.data.connectedPort && 0!=port.data.connectedPort)
                        {
                            if(typeof(portOwner[port.data.connectedPort])!=="undefined")
                            {
                                var label;
                                switch(port.name)
                                {
                                    case "RequireUnit":
                                    case "ProvideUnit":
                                        label="Unit: "+port.data.unitType+" ("+port.data.unitID+")";
                                        break;
                                    case "RequireEnemyUnit":
                                    case "ProvideEnemyUnit":
                                        label="Enemy: "+port.data.unitType+" ("+port.data.unitID+")";
                                        break;

                                    case "Resource":
                                        label=port.name+": minerals: "+port.data.minerals+", gas: "+port.data.gas;
                                        break;

                                    case "Supply":
                                        label=port.name+": race: "+port.data.race+", "+(port.data.required>0?"required: "+port.data.required:"provided: "+(-port.data.required));
                                        break;

                                    case "ProvideMineralField":
                                    case "RequireMineralField":
                                        label="MineralField";
                                        break;

                                    case "RequireSpace":
                                        label="Space: "+port.data.unitType;
                                        break;
                                    default:
                                        label=port.name+": "+JSON.stringify(port.data);
                                }
                                if(port.type==="require")
                                {
                                    addEdge("edge"+getJoinedPortName(item.id,portOwner[port.data.connectedPort]),"item"+item.id, "item"+portOwner[port.data.connectedPort], { label: label });
                                }
                                else
                                {
                                    addEdge("edge"+getJoinedPortName(item.id,portOwner[port.data.connectedPort]),"item"+portOwner[port.data.connectedPort], "item"+item.id, { label: label });
                                }
                            }
                        }
                    }
                }
            }

            var items=data.items;//$.parseJSON(data);
            for(var i in items)
            {
                var item=items[i];
                addPlanItem(item,"planitem");
            }

            var boundaryitems=data.boundaries;
            for(var b in boundaryitems)
            {
                var item=boundaryitems[b];
                addPlanItem(item,"boundaryitem");
            }

            var renderer = new dagreD3.Renderer();
            var oldDrawNodes = renderer.drawNodes();
            renderer.drawNodes(function(g, svg) 
            {
                var svgNodes = oldDrawNodes(g, svg);
                svgNodes.attr("class", function(u) 
                { 
                    switch(g.node(u).type)
                    {
                        case "port":
                            return "node "+g.node(u).type+" "+(g.node(u).port.active?"active":"inactive");
                        case "planitem":
                            return "node "+g.node(u).type;
                        case "boundaryitem":
                            return "node "+g.node(u).type;
                    }
                });
                return svgNodes;
            });
            //renderer.zoom(false);
            var layout = dagreD3.layout().nodeSep(20).rankSep(20).rankDir("LR").rankSimplex(true);

            // Custom transition function
            function transition(selection) 
            {
                return selection.transition().duration(500);
            }
            renderer.transition(transition);

            renderer.layout(layout).run(g, d3.select("#plangraph-graph g"));
            $("#plangraph-output-svg").text($("#plangraph-graph").html().replace(/<g/g, "\n<g"));
            $("#plangraph-output-dot").text(dotOutput+"}");

            if($('#plangraph-autoupdate').prop('checked'))
                setTimeout(plangraphUpdate,1000);
        }
        catch (e)
        {
            alert(""+e+":"+data);
        }
    }).fail(function() {

    });
}

function plangraphInit()
{
    $("#plangraph-update").on('click',function(){plangraphUpdate();});
    plangraphUpdate();
}
