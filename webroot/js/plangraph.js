

function plangraphInit()
{
    // Custom transition function
    function transition(selection)
    {
        return selection.transition().duration(500);
    }

    function plangraphRedraw(data,animate_transition)
    {
        try
        {
            var g = new dagreD3.Digraph();

            var config=
            {
                    // "2nodes"/"1node"/"edge"
                displayPortsAs: $("#plangraph-displayPortsAs").val(),

                    // true/false
                skipResources: $("#plangraph-skipResources").prop('checked'),

                    // true/false
                skipUnconnected: $("#plangraph-skipUnconnected").prop('checked')
            };

            var dotOutput="digraph G {\n";

            var portOwner={};

            function ensureNodeExists(id,data)
            {
                if(!g.hasNode(id))
                {
                    g.addNode(id, typeof(data)!== "undefined"?data:{label: "..." /*"stub for "+id*/});
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
                g.addEdge(id?id:"edge"+to+from,to,from,data);
                if(to!=="" && from!=="")
                    dotOutput=dotOutput+to+' -> '+from+';\n';
                //g.addEdge(id,from,to,data);
            }

            function getJoinedPortName(p1,p2)
            {
                return "port"+(p1<p2?p1+"-"+p2:p2+"-"+p1);
            }

            function getShortUnitType(type)
            {
                return type.replace(/(zerg |terran |protoss |resource )/i, '');
            }

            function getEstimatedTimeString(time)
            {
                if(time==-1)
                    return "A";
                return time;
            }
            
            function getStatusEstimatedString(status,time)
            {
                switch(status)
                {
                    case "planned":
                        return getEstimatedTimeString(time);
                    case "active":
                        return "A";
                    case "executing":
                        return "E";
                    case "failed":
                        return "F";
                    default:
                        return "?";
                }
            }

            function getPositionString(pos)
            {
                if(pos.info)
                    return pos.info;
                return "("+pos.x+","+pos.y+")";
            }

            function getPortLabel(port)
            {
                var label;
                    switch(port.name)
                    {
                        case "RequireUnit":
                        case "ProvideUnit":
                            label=getShortUnitType(port.data.unitType);
                            if(port.data.unitID==0)
                                label=label+" <span style=\"color: red; font-size: 18px;\">("+port.data.unitID+")</span>";
                            else
                                label=label+" ("+port.data.unitID+")";
                            break;
                        case "RequireUnitExistance":
                        case "ProvideUnitExistance":
                            label="Existance: "+getShortUnitType(port.data.unitType);
                            break;
                        case "RequireEnemyUnit":
                        case "ProvideEnemyUnit":
                            label="Enemy: "+getShortUnitType(port.data.unitType);
                            if(port.data.unitID==0)
                                label=label+" <span style=\"color: red; font-size: 18px;\">("+port.data.unitID+")</span>";
                            else
                                label=label+" ("+port.data.unitID+")";
                            break;

                        case "Resource":
                            label=port.data.minerals?"minerals: "+port.data.minerals+" ":""+port.data.gas?"gas: "+port.data.gas:"";
                            break;

                        case "Supply":
                            label=(-port.data.required)+" "+port.data.race+" supply";
                            break;

                        case "ProvideMineralField":
                        case "RequireMineralField":
                            label="MineralField";
                            break;

                        case "RequireSpace":
                            label=port.name+": "+getShortUnitType(port.data.unitType);
                            break;
                        default:
                            label=port.name+"<br /><i>"+JSON.stringify(port.data)+"</i>";
                    }
                return label;
            }

            function addPlanItem(item,type)
            {
                var label;

                switch(item.name)
                {
                    case "EnemyUnit":
                        label="Enemy: "+getShortUnitType(item.data.unitType);
                        label+=" ("+item.data.unitID+")";
                        break;
                    case "OwnUnit":
                        label="Own: "+getShortUnitType(item.data.unitType);
                        label+=" ("+item.data.unitID+")";
                        break;

                    case "Resource":
                        if(config.skipResources)
                            return;
                        label=getShortUnitType(item.data.unitType)+" ("+item.data.unitID+")"+"<br />"+"worker: "+item.data.numberOfWorkers+", minerals: "+item.data.mineralsLeft;
                        break;

                    case "MorphUnit":
                    case "Build":
                        label=item.name+": "+getShortUnitType(item.data.unitType);
                        break;

                    case "MoveToPosition":
                        label="MoveTo: "+getPositionString(item.data.pos);
                        break;

                    case "GatherMinerals":
                    case "AttackUnit":
                        label=item.name;
                        break;

                    default:
                        label=item.name+"<br /><i>"+JSON.stringify(item.data)+"</i>";
                }
                if(type==="planitem")
                {
                    label="<span class=\"active_indicator\">"+getStatusEstimatedString(item.status,item.estimatedStartTime)+"</span>"+label;
                }

                if(config.skipUnconnected)
                {
                    var haveConnection=false;
                    for(var p in item.ports)
                    {
                        port=item.ports[p];
                        if(port.data.connectedPort && 0!=port.data.connectedPort)
                            haveConnection=true;
                    }
                    if(!haveConnection)
                        return;
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
                        
                        var label=getPortLabel(port);

                        var activeconnection=port.activeConnection?" activeconnection":"";
                        var active="<span class=\"active_indicator\">"+getEstimatedTimeString(port.estimatedTime)+"</span>";
                        if(port.type==="require")
                        {
                            label="<span class=\"require_indicator"+activeconnection+"\"></span>"+label+active;
                        }
                        else
                        {
                            label=active+label+"<span class=\"provide_indicator"+activeconnection+"\"></span>";
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
                                addEdge(null,"port"+port.id,"port"+port.data.connectedPort,{label: "connected" });
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
                        ensureNodeExists("emptyRequirePortForItem"+item.id,{label:" "});
                        addEdge(null,"item"+item.id,"emptyRequirePortForItem"+item.id, {label: 'nothing' /*, style: 'visibility: hidden;  display: none;' */});
                    }
                }
                else if(config.displayPortsAs==="1node")
                {
                    for(var p in item.ports)
                    {
                        var port=item.ports[p];

                        if(port.data.connectedPort && 0!=port.data.connectedPort || port.type==="provide")
                        {
                            var label="<span class=\"active_indicator\">"+getEstimatedTimeString(port.estimatedTime)+"</span>"+getPortLabel(port);
                            var portName=getJoinedPortName(port.id,port.data.connectedPort);
                            
                            var graphinfo;  //provide ports override require ports
                            if(!g.hasNode(portName)||port.type==="provide")
                                graphinfo={
                                label: "<div class=\"label\">"+label+"</div>",
                                type: "port",
                                port: port };

                            ensureNodeExists(portName, graphinfo);
                            
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
                                var label="<div class=\"label\">"+getPortLabel(port)+"</div>";
                                if(port.type==="require")
                                {
                                    addEdge(null,"item"+item.id, "item"+portOwner[port.data.connectedPort], { label: label });
                                }
                                else
                                {
                                    addEdge(null,"item"+portOwner[port.data.connectedPort], "item"+item.id, { label: label });
                                }
                            }
                        }
                        else if(port.type==="provide")  //display estimates for unconnected provide ports
                        {
                            var label="<span class=\"active_indicator\">"+getEstimatedTimeString(port.estimatedTime)+"</span> ? ";
                            ensureNodeExists("emptyItemForProvidePort"+port.id,{
                                label:"<div class=\"label\">"+label+"</div>",
                                type: "port",
                                port: port });
                            addEdge(null,"emptyItemForProvidePort"+port.id, "item"+item.id, { label: "<div class=\"label\">"+getPortLabel(port)+"</div>" });
                        }
                    }
                }

                    //gather all ports in data.ports
                for(p in item.ports)
                    data.ports[item.ports[p].id]=item.ports[p];
            }

            data.ports={};

            var items=data.items;
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
                            return "node "+g.node(u).type+" "+(g.node(u).port.impossible?"impossible":(g.node(u).port.active?"active":"inactive"));
                        case "planitem":
                            return "node "+g.node(u).type+" "+(g.node(u).item.impossible?"impossible":(g.node(u).item.active?"active":"inactive"));;
                        case "boundaryitem":
                            return "node "+g.node(u).type;
                        default:
                            return "node";
                    }
                }).attr("original-title", function(u)
                {
                    switch(g.node(u).type)
                    {
                        case "port":
                            return g.node(u).port.id;
                        case "planitem":
                            return g.node(u).item.id;
                        case "boundaryitem":
                            return g.node(u).item.id;
                        /*default:
                            return "node";*/
                    }
                });
                return svgNodes;
            });
            //renderer.zoom(false);
            var layout = dagreD3.layout().nodeSep(20).rankSep(20).rankDir("LR").rankSimplex(true);

            if(animate_transition)
                renderer.transition(transition);

            renderer.layout(layout).run(g, d3.select("#plangraph-graph g"));

            $("#plangraph-output-svg").text($("#plangraph-graph").html().replace(/<g/g, "\n<g"));
            $("#plangraph-output-dot").text(dotOutput+"}");
        }
        catch (e)
        {
            alert(""+e+":"+data);
        }
    }

    var currentPlanData;

    var updateAlreadyRunning=false;

    function plangraphUpdate()
    {
        if(updateAlreadyRunning)
        {
            alert("already fetching new plan...");
            return;
        }
        updateAlreadyRunning=true;

        $.post('plan',{},function(data)
        {
            updateAlreadyRunning=false;
            if(!data)
                return;
            
            if(data.status==="running")
            {
                plangraphRedraw(data,typeof(currentPlanData)!=="undefined");

                $("#plangraph-headline").text("Plangraph for frame #"+data.time);

                currentPlanData=data;
            }
            
            if($('#plangraph-autoupdate').prop('checked'))
                setTimeout(plangraphUpdate,1000);

        }).fail(function() {
            updateAlreadyRunning=false;
            $("#plangraph-autoupdate").prop('checked',false);
        });
    }
    
    $("#plangraph-update").on('click',function(){plangraphUpdate();});

    function configChanged()
    {
        $('#plangraph-autoupdate').prop('checked',false);
        if(!updateAlreadyRunning)
            plangraphRedraw(currentPlanData,false);
    }
    $("#plangraph-displayPortsAs").on('change',configChanged);
    $("#plangraph-skipResources").on('change',configChanged);
    $("#plangraph-skipUnconnected").on('change',configChanged);
    $("#plangraph-autoupdate").on('change',function()
    {
        if($("#plangraph-autoupdate").prop('checked') && !updateAlreadyRunning)
            plangraphUpdate();
    });
    plangraphUpdate();

    function getTooltipForItem(typename,item)
    {
        if(!item)
            return "item undefined";

        var tooltip="<h3>"+item.name+typename+"</h3>";

        tooltip+="<table>";
        for(var field in item)
        {
            if(field==="name" || field==="ports" || field==="data")
                continue
            tooltip+="<tr><td>"+field+":</td><td>"+JSON.stringify(item[field])+"</td></tr>";
        }
        for(var field in item.data)
        {
            tooltip+="<tr><td>"+field+":</td><td>"+JSON.stringify(item.data[field])+"</td></tr>";
        }

        tooltip+="</table>";

        return tooltip;
    }

    $('#plangraph-graph .planitem').tipsy({
        gravity: 'n',
        html: true,
        live: true,
        hoverlock: true,
        title: function() {
            if(!currentPlanData)
                return "no data";
            return getTooltipForItem("PlanItem",currentPlanData.items[this.getAttribute('original-title')]);
        }
    });
    $('#plangraph-graph .boundaryitem').tipsy({
        gravity: 'n',
        html: true,
        live: true,
        hoverlock: true,
        title: function() {
            if(!currentPlanData)
                return "no data";
            return getTooltipForItem("BoundaryItem",currentPlanData.boundaries[this.getAttribute('original-title')]);
        }
    });
    $('#plangraph-graph .port').tipsy({ 
        gravity: 'n',
        html: true,
        live: true,
        hoverlock: true,
        title: function() {
            if(!currentPlanData)
                return "no data";
            return getTooltipForItem("Port",currentPlanData.ports[this.getAttribute('original-title')]);
        }
    });
    
}
