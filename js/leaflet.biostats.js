/*
Dieog Pino N 2015
Incremental GBIF lat/long from ocurrences to leaflet features load
 */

(function ($) {

  var LEAFLET_MARKERCLUSTER_EXCLUDE_FROM_CLUSTER = 0x01;

  Drupal.behaviors.biostats_leaflet = { // overrides same behavior in leaflet/leaflet.drupal.js and Leaflet_markercluster.drupal.js
    attach: function(context, settings) {
      
      $(settings.biostats_leaflet).each(function () {
      
        if ((typeof this.gbiftaxo !== 'undefined') && (this.gbiftaxo>0)) {
        var ocurrenceCount=this.gbifcount;  
        var taxoKey=parseInt(this.gbiftaxo);
        var progress =  $('#biostats-progress')[0];
        var progressBar = $('#biostats-progress-bar')[0];
        var map;  
        var markerList = [];
        var markersGBIF;
        var controlMapa;
        var lGroupGbif = new L.LayerGroup();
        var streamedrecordscount=0;
        function updateProgressBar(processed, total) {
        
           
        				// if it takes more than a second to load, display the progress bar:
        			progress.style.display = 'block';
        			progressBar.style.width = Math.round(processed/total*100) + '%';
        		

        		if (processed === total) {
        				// all markers processed - hide the progress bar:
        			progress.style.display = 'none';
        			
              }
        		}
          function loadGBIFfeaturesStream() {
             
             
           oboe(Drupal.settings.basePath + 'islandora/biostats/ajax/getgbifocurrences_stream/CL/' + taxoKey+'/0')
              .node('!.results.*', function( record ){
                streamedrecordscount++;
                http://www.gbif.org/occurrence/
                var title = 'GBIF ID <a href="http://www.gbif.org/occurrence/'+record.gbifID+'" target="_blank">'+record.gbifID+'</a><br> Event Date:'+record.eventDate;
                title=title + "<br>for " + record.scientificName + '('+record.taxonRank+')';
                title=title + "<br>published by Country:"+record.publishingCountry;
                var feature={
                    type : 'point',
                    lat: parseFloat(record.decimalLatitude),
                    lon: parseFloat(record.decimalLongitude),
                    leaflet_id: 'gbif-'+record.gbifID,
                    popup : title,
                    clusterGroup : 'gbif',
                  };
                updateProgressBar( streamedrecordscount, ocurrenceCount); 
                var lFeature = Drupal.leaflet.create_point(feature, map);
                lFeature.bindPopup(feature.popup)
                //markerList.push(lFeature);
                markersGBIF.addLayer(lFeature);
                if (streamedrecordscount==1) {
                  map.addLayer(markersGBIF);
                  }
                  
                
                  
               
              })
              .done(function(things) {
                console.log(
                'there are', things.results.length, 'records');
               
                lGroupGbif.addLayer(markersGBIF);
                map.addLayer(lGroupGbif);
                controlMapa.addOverlay(lGroupGbif,'GBIF Occurrences');

              })
              .fail(function() {
                console.log('failed');
                 // we don't got it
              });
              
              } 
        
        
        
        
        function loadGBIFfeatures(result) {
             
             
             
              for (var i = 0; i < result.length; i++) {
                var a = result[i];
               
                var title = a.gbifID+' '+a.collectionCode;
                var feature={
                    type : 'point',
                    lat: parseFloat(a.decimalLatitude),
                    lon: parseFloat(a.decimalLongitude),
                    leaflet_id: 'gbif-'+a.gbifID,
                    popup : title,
                    clusterGroup : 'gbif',
                  };
              
                var lFeature = Drupal.leaflet.create_point(feature, map);
                lFeature.bindPopup(feature.popup)
                markerList.push(lFeature);
              }
              if (result.length>0)
                {
               
                markersGBIF.addLayers(markerList);
                lGroupGbif.addLayer(markersGBIF);
                map.addLayer(lGroupGbif);
                controlMapa.addOverlay(lGroupGbif,'GBIF Occurrences');
                }
               
              }  
        function onEachFeature(feature, layer) {
              			//var popupContent = "<p>I started out as a GeoJSON " +
              			//		feature.geometry.type + ", but now I'm a Leaflet vector!</p>";

              			//if (feature.properties && feature.properties.popupContent) {
              				//popupContent += feature.properties.popupContent;
                      //}

              			//layer.bindPopup(popupContent);
              		}  
        //$(selector).bind(event,data,function,map)
        $(document).bind('leaflet.map', function(e, settingsLeaflet, lMap)
          {
         
          
         
          map=lMap;
          var regionesChile;
          /*regionesChile=$.getJSON('/json/chileregion.json', function(data){regionesChile = L.geoJson(data, {

          			onEachFeature: onEachFeature
          		}).addTo(map); 
            })
          */
          
          
          var clustergroupsettings=settingsLeaflet.settings
          clustergroupsettings.chunkedLoading=  true;
          clustergroupsettings.iconCreateFunction =function (cluster) {
          		var childCount = cluster.getChildCount();

          		var c = ' biostatsmarker-cluster-';
          		if (childCount < 10) {
          			c += 'small';
          		} else if (childCount < 100) {
          			c += 'medium';
          		} else {
          			c += 'large';
          		}

          		return new L.DivIcon({ html: '<div><span>' + childCount + '</span></div>', className: 'biostatsmarker-cluster' + c, iconSize: new L.Point(40, 40) });
          	},
          markersGBIF= new L.MarkerClusterGroup(clustergroupsettings);
          loadGBIFfeaturesStream();
   
          });
          $(document).bind('leaflet.controlCluster', function(e,controlCluster,otro) {
          
            controlMapa=controlCluster;
          });  
        }  
    });    
  }  
  }
})(jQuery);
