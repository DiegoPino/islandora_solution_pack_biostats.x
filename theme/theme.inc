<?php
/**
* @file
* Theme hooks.
*/

/**
* Implements hook_preprocess().
*/
function islandora_biostats_preprocess_islandora_biostats_perobject(array &$variables)
{

  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  module_load_include('inc', 'islandora_biostats', 'includes/gbifapi');
  module_load_include('inc', 'islandora_biostats', 'includes/redbioapi');
  module_load_include('inc', 'islandora_biostats', 'includes/taxa');
  
  $islandora_object = $variables['islandora_object'];
  $pid = $islandora_object->id;
  drupal_set_title($islandora_object->label+ 'Stats');
  $dwc_solr = islandora_biostats_redbioapi_perobject_solr($pid); //returns solr
  $classification_base=array('kingdom','phylum','class','order','family','genus','species');
  $highertaxo_bread_list=array();
  $existing_classification_base=array();
  if (isset($dwc_solr['response']['docs'][0]['dwc.scientificName_s']) && strlen($dwc_solr['response']['docs'][0]['dwc.scientificName_s'])>0) {
      //We are assuming we don't trust our data. So we get the info from GBIF by making a species match using our solr stored scientificName
      $gbiftaxo=islandora_biostats_gbifapi_taxonkey($dwc_solr['response']['docs'][0]['dwc.scientificName_s']);
      
      
      if (isset($gbiftaxo['usageKey'])) { //Means we got a positive result from GBFI
        $taxoforMap=$gbiftaxo['usageKey'];
        //Get ocurrences count for Chile
        $totalGbifOcurrences=islandora_biostats_gbifapi_count($taxoforMap);
        //populate our higher taxa clasification
        foreach($classification_base as $taxa) {
          if ($gbiftaxo['rank']==strtoupper($taxa)) break;
          array_unshift($existing_classification_base,$taxa);//we keep count of our tree
          $highertaxo_bread_list[]=$gbiftaxo[$taxa];
          
        }
        foreach($existing_classification_base as $taxa) {
          if ($totalGbifOcurrences>0) break;//If we already had this, break inmidiatly
            //If we got no ocurrences for species we search the taxa key for parent ocurrences
         
         
          $totalGbifOcurrences=islandora_biostats_gbifapi_count($gbiftaxo[$taxa.'Key']);          
          $taxoforMap=!empty($totalGbifOcurrences)? $gbiftaxo[$taxa.'Key']:$taxoforMap;
          } 
      
      }

  $highertaxo[] = array(
     '#theme' => 'item_list',
     '#items' => $highertaxo_bread_list,
     '#type' => 'ol',
     '#attributes' => array('class' => 'breadcrumb'),
   );
   $highertaxo['#attached']['js'][] = array(
       'data' => array(
        'biostats_leaflet'=>array(array('gbifcount'=>$totalGbifOcurrences,'gbiftaxo'=>$taxoforMap)),
       ),
       'type' => 'setting',
     );
     $args = array(
      'taxonKey' => $taxoforMap,  
      'country'=> 'CL',
      'offset'=> 0, 
      'hasCoordinate' => 'true',
      'hasGeospatialIssue' => 'false',
      'limit' =>300,
     );
  
  
     $data = islandora_biostats_gbifapi_ocurrences($args);
     
     $GbifObject = new biostatsGBIFOccurrencesStats($taxoforMap,$totalGbifOcurrences);
     $GbifObject->addJSon($data['results'],0);
     $facets=$GbifObject->getFacets();



  if (!empty($dwc_solr)) {
    //$styles = module_invoke_all('leaflet_map_info'); 
    $variables['map']= theme('islandora_red_biodiversidad_map',
        array('islandora_object' => $islandora_object,'results'=>NULL,'limit_to_cmodel'=>array('info:fedora/biodiversity:OccurrenceCModel')));
   
    
    
    
    $variables['objectsolr']=$dwc_solr;
    //$variables['results'][] = drupal_render(islandora_biostats_build_chart(array(),'Table','biotstats_chart1' , $variables['objectsolr']['response']['docs'][0], $height = '400px'));
    $chartData=array('Publishing Country'=>'Occurrences contribution')+$facets['publishingCountry'];
    $variables['results'] = drupal_render(islandora_biostats_build_chart(array('title'=>'Occurrences Contribution by Country','is3D' => true),'PieChart','biotstats_chart2' , (object) $chartData, $height = '400px'));
     $variables['results'].= drupal_render(islandora_biostats_build_chart(array(),'Table','biotstats_chart1' , $variables['objectsolr']['response']['docs'][0], $height = '400px'));
    $variables['highertaxo']=  drupal_render($highertaxo);
    
    //dpm(islandora_biostats_redbioapi_relatedtoobject_solr($pid, array('RELS_EXT_taxonOfId_uri_ms','RELS_EXT_identifies_uri_ms')));
    //dpm(islandora_biostats_redbioapi_relatedtoobject_solr($pid, array('RELS_EXT_taxonOfId_uri_ms','RELS_EXT_identifies_uri_ms','RELS_EXT_atEvent_uri_ms')));
    //Creando Tablas
    
    
    
    
    //$variables['tablas']=theme('table', array('header' => $header, 'rows' => $rows));
    }  
  }
}
