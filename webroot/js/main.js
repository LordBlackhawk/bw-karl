

$(document).ready(function()
{
    function statusUpdate()
    {
        $.ajax({
            type: "POST",
            url: 'status',
            dataType: "json",
            timeout: 800, // in milliseconds
            success: function(data) 
            {
                if(!data)
                    return;
                
                switch(data.status)
                {
                    case "pregame":
                        $("#info").html("waiting for game...");
                        $("#status").css("background-color","#000099");
                        break;
                    case "running":
                        $("#info").html("Time: "+data.time+" Minerals: "+data.minerals+" Gas: "+data.gas);
                        $("#status").css("background-color","#007700");
                        break;
                    default:
                        $("#info").html("unknown game state: "+data.status);
                        $("#status").css("background-color","#777777");
                }
                
            },
            error: function(request, status, err)
            {
                $("#status").css("background-color","#770000");
            }
        });
    }
    
    setInterval(statusUpdate,1000);
    
    $(".uiimport").each(function(index)
    {
        var $uiimport=$(this);
        $uiimport.load("ui/"+$uiimport.data('uiimport')+".html");
    });
    

    $('#gamespeed').noUiSlider({
        start: [ 0 ],
        step: 1,
        range: {
                'min': [  0 ],
                'max': [ 100 ]
        }
    }).change(function(){
        $.post('gamespeed', {value:$('#gamespeed').val()},function(data)
        {
            if(data!=="ok")
                alert(data);
        },'text');
    });

    $('#engine-interrupt').on('click',function(){
        $.post('interrupt', {},function(data)
        {
            if(data!=="ok")
                alert(data);
        },'text');
    });

    $('#engine-continue').on('click',function(){
        $.post('continue', {},function(data)
        {
            if(data!=="ok")
                alert(data);
        },'text');
    });

    $('#add-planitem-morph').on('submit',function(e){
        e.preventDefault();
        $.post('add', {type:'MorphUnitPlanItem',unitType:$('#add-planitem-morph-type').val()},function(data)
        {
            if(data!=="ok")
                alert(data);
        },'text');
    });
    $('#add-planitem-build').on('submit',function(e){
        e.preventDefault();
        $.post('add', {type:'BuildPlanItem',unitType:$('#add-planitem-build-type').val()},function(data)
        {
            if(data!=="ok")
                alert(data);
        },'text');
    });
    $.get('unittypes',function(data)
    {
        var buildings=[];
        var morphable=[];
        
        for(var unitType in data)
        {
            var ut=data[unitType];
            
            if(ut.isBuilding)
            {
                buildings.push(unitType);
            }
            if(ut.race==="Zerg" && !ut.isBuilding)
            {
                morphable.push(unitType);
            }
        }
        
        $('#add-planitem-morph-type').autocomplete({
            autoSelectFirst: true,
            lookup:morphable,
            onSelect:function(value){
                //alert(value);
            }
        });
        $('#add-planitem-build-type').autocomplete({
            autoSelectFirst: true,
            lookup:buildings,
            onSelect:function(value){
                //alert(value);
            }
        });
    });
});