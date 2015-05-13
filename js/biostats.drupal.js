(function ($) {
  Drupal.behaviors.biostats_viz = {
    attach:function (context, settings) {
      //console.log(settings.biostats_viz);
      $(settings.biostats_viz).each(function () {
        // skip to the next iteration if the map already exists
        var container = $('#'+this.chartId)[0];
        //console.log(container);
        if (!container || container._biostats_viz) {
          return;
        }
  
        // load a options object with all of our google chart settings
       
       
        var options = {
        
        };
        for (var setting in this.chartoptions) {
          options[setting] = this.chartoptions[setting];
        }
       //var chartdataArray = $.map($.makeArray(this.chartdata), function(el,i) { return [i,el] });
      
        var chartdataArray  = [];
        for(var key in this.chartdata) {
            if(this.chartdata.hasOwnProperty(key)) {
              chartdataArray.push([key,this.chartdata[key]]);
            }
          }  
        //console.log(chartdataArray);
        
        //console.log(chartdataArray);
        if (this.chartdata) {
          var data = google.visualization.arrayToDataTable(chartdataArray);
          var chart = new google.visualization[this.charttype](container,true);
          chart.draw(data, options);	
       }
      
      });
    }
  }
})(jQuery);
