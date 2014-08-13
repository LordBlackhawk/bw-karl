

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
                $("#info").html("Time: "+data.time+" Minerals: "+data.minerals+" Gas: "+data.gas);
                $("#status").css("background-color","#007700");
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
        });
    });
});