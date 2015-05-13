<?php

/**
 * @file
 * Holds the ajax callbacks for the islandora_red_biodiversidad.
 */

/**
 * Returns array of items to autocomplete forms.
 *
 * @param string $type
 *   Selects either scientificName or Genus, could be extended to others
 *
 * @return array
 *   array of all candidates
 * requires that solr contains and index dw.scientificName_mt(tokenized field) or dw.genus_mt (tokenized field) from darwincore
 * @see folder sorlconfig/ for our implementation for solr
 */
function islandora_biostats_autocomplete($type) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $mappings = array(
    'scientificName' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.scientificName_mt',
    ),
    'genus' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.genus_mt',
    ),
  );
 
  $results = array();

  $content_model_field = variable_get(
      'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
  );
   
  $biodiversidad_query_field = $mappings[$type]['title'];
  $stub="";
  if (isset($_GET['q'])) {
    $stub = end(explode('/', $_GET['q']));
}
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
      '!biodiversidad_query_field:(!biodiversidad_query)', array(
        '!biodiversidad_query_field' => $biodiversidad_query_field,
        '!biodiversidad_query' => $stub.'*',
      )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = array(
    'fl' => "$biodiversidad_query_field,PID",
    'fq' => format_string(
        '!content_model_field:"info:fedora/!content_model"', array(
          '!content_model_field' => $content_model_field,
          '!content_model' => $mappings[$type]['cmodel'],
        )
    ),
  );
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while searching ScientificName in Solr for callback.', array(), WATCHDOG_ERROR);
  }
  $list = array();
  
  foreach ($json['response']['docs'] as $choice) {
	
    if (isset($choice[$mappings[$type]['title']])) {
      $list[$choice[$mappings[$type]['title']][0]] = $choice[$mappings[$type]['title']][0];//Solr devuelve un arreglo. Verificar schema.xml
    }
  }
 
  drupal_json_output($list);
}

function islandora_biostats_ajax_getgbifocurrences($country, $taxonkey=1, $offset=0) {
  module_load_include('inc', 'islandora_biostats', 'includes/gbifapi');
  module_load_include('inc', 'islandora_biostats', 'includes/taxa');
  $args = array(
   'taxonKey' => $taxonkey,  
   'country'=> $country,
   'offset'=> $offset, 
   'hasCoordinate' => 'true',
   'hasGeospatialIssue' => 'false',
  );
  
  
  $data = islandora_biostats_gbifapi_ocurrences($args);
  $output=array();
  if (count($data)>0 && isset($data['count']) && ($data['count']>0)) {
    $setting ['biostats_leaflet']['count'] = $data['count'];
    $output=$data['results'];
    /*return $outpout; = array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_settings($setting),
      )
    )
   */     
  }
  return drupal_json_output($output);
}
/**
 * Incremental Json Streaming Function, allows a continuos stream of Ocurrences, called recursive
*/
function islandora_biostats_ajax_getgbifocurrences_stream($country, $taxonkey=1, $offset=0) {
  module_load_include('inc', 'islandora_biostats', 'includes/gbifapi');
   module_load_include('inc', 'islandora_biostats', 'includes/taxa');
  $args = array(
   'taxonKey' => $taxonkey,  
   'country'=> $country,
   'offset'=> $offset, 
   'hasCoordinate' => 'true',
   'hasGeospatialIssue' => 'false',
   'limit' =>300,
  );
  
  
  $data = islandora_biostats_gbifapi_ocurrences($args);
  sleep(1);
    
  if (($offset==0) && (count($data)>0 && isset($data['count']) && ($data['count']>0)))
    {
      drupal_add_http_header('Content-Type', 'text/event-stream');
      drupal_add_http_header('Cache-Control','no-cache');
     
      echo "{\"results\":[\n";
    }
    $i=0; //well not very clean, but neccesary
    //we can't output a whole json string everytime because our js library, OB
    //thinks(correctly) every group of records is a new json and we wan't a continuous stream
  if ((count($data)>0 && isset($data['count']) && ($data['count']>0)) || ($data['endOfRecords']==false)) {
    foreach($data['results'] as $record)
      {
      if (($i>0) || ($offset>= $args['limit'])){ echo ",";} 
      echo drupal_json_encode($record,JSON_HEX_TAG | JSON_HEX_APOS | JSON_HEX_AMP | JSON_HEX_QUOT)."\n";
      $i++;
      }
    ob_flush();
    flush();
    if ($data['count']>=$offset+$args['limit'])
      {
      islandora_biostats_ajax_getgbifocurrences_stream($country, $taxonkey, $offset+$args['limit']);  
      }
    else
      {
       echo "]\n}\n"; 
      }  
  }
  
  
  ob_flush();
  flush();
  //drupal_exit();
  return NULL;
}
